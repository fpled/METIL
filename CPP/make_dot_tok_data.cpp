#include "nstring.h"
#include "lexem.h"
#include "lexer.h"
#include "errorlist.h"
#include "hashstring.h"
#include <usual_strings.h>
#include "config.h"
#include "autorestore.h"
#include "splittedvec.h"
#include "hashset.h"
#include "typechar.h"
#include <string.h>

#define MAKE_DOT_TOK_DATA_CPP
#include "make_dot_tok_data.h"

TokData make_tok_data( const char *txt, const char *provenance, ErrorList *error_list, bool one_var ) {
    unsigned oe = error_list->data.size();
    DotSarToDotTok dstdt( txt, provenance, error_list, one_var );
    if ( oe != error_list->data.size() ) {
        TokData res;
        res.data = NULL;
        res.size = 0;
        return res;
    }
    return dstdt.contiguous_bin_data();
}

inline unsigned ceil_modulo(unsigned v,unsigned m) { m -= 1; return ( v + m ) & ~m; } /// @see GetRoomForStringInfo and WriteStringInfo

struct GetRoomForStringInfo {
    void operator()(const DotSarToDotTok::RelData &red) {
        unsigned si = ceil_modulo( red.si, sizeof(unsigned) );
        res += sizeof(unsigned) * 3 + si;
    }
    unsigned res;
};

struct WriteStringInfo {
    void operator()(const DotSarToDotTok::RelData &red) {
        unsigned si = ceil_modulo( red.si, sizeof(unsigned) );
        ((unsigned *)res)[0] = red.n.v;
        ((unsigned *)res)[1] = red.si;
        ((unsigned *)res)[2] = hashstring( red.s, red.si );
        res += sizeof(unsigned) * 3;
        memcpy( res, red.s, red.si );
        for(unsigned i=red.si;i<si;++i) res[i]=0;
        res += si;
    }
    char *res;
};

TokData DotSarToDotTok::contiguous_bin_data() {
    TokData res;
    
    //
    TokHeader th;
    strncpy( th.md5_ctrl_sum, USUAL_STRINGS_MD5, 32 );
    th.nb_pos_types_to_rellocate = pos_types_to_rellocate.size();
    th.nb_pos_strings_to_rellocate = pos_strings_to_rellocate.size();
    th.nb_strings = strings_to_rellocate.size();
    th.nb_property_names = property_names.size();
    
    // get room needed to store string info
    GetRoomForStringInfo ri;
    ri.res = th.nb_pos_strings_to_rellocate * sizeof(unsigned);
    strings_to_rellocate.apply( ri );
    
    // allocation
    res.size = sizeof(TokHeader) - 4 * sizeof(unsigned) + 
            pos_types_to_rellocate.size() * sizeof(unsigned) +
            ri.res + /// pos and string data
            property_names.size() * sizeof(unsigned) +
            bin.size();
    res.data = (char *)malloc( res.size );
    WriteStringInfo wsi; wsi.res = res.data + sizeof(TokHeader) - 4 * sizeof(unsigned);
    
    // copy rellocation data
    pos_types_to_rellocate.copy_binary_data_to( wsi.res );   wsi.res += pos_types_to_rellocate.size() * sizeof(unsigned);
    pos_strings_to_rellocate.copy_binary_data_to( wsi.res ); wsi.res += pos_strings_to_rellocate.size() * sizeof(unsigned);
    strings_to_rellocate.apply( wsi ); // updates also strings_to_rellocate
    
    // copy property_names
    th.offset_of_property_names = wsi.res - res.data;
    property_names.copy_binary_data_to( wsi.res );   wsi.res += property_names.size() * sizeof(unsigned);
    
    // copy of binary tok data
    if ( bin.size() ) {
        th.offset_of_first_tok_in_block = wsi.res - res.data;
        bin.copy_binary_data_to( wsi.res ); wsi.res += bin.size();
    }
    else
        th.offset_of_first_tok_in_block = 0;
    
    // copy of tok header
    memcpy( res.data, &th, sizeof(TokHeader) - 4 * sizeof(unsigned) );
    
    return res;
}

DotSarToDotTok::DotSarToDotTok( const char *sar_txt_, const char *provenance_, ErrorList *error_list_, bool one_var ) : 
        sar_txt(sar_txt_), provenance(provenance_), error_list(error_list_) {
    if ( not sar_txt )
        return;

    // get a lexem graph
    Lexer lexer( sar_txt, provenance, error_list );
    
    // convert lexems to Tok
    static_ = true;
    const_ = false;
    virtual_ = false;
    stack_prev = new StackPrev;
    stack_prev->main_one = true;
    
    if ( lexer.root() ) {
        push_type_and_offsets( lexer.root(), Tok::CHECK_ROOM_IN_VARIABLE_STACK, 0 );
        unsigned *room_for_var = bin.binary_write<unsigned>( 0 ); //
        
        for(const Lexem *l = lexer.root(); l; l=l->next )
            push_tok( l, /*expect_result only at end*/ one_var * (not l->next) );
        
        *room_for_var = stack_prev->needed_size;
    }
    
    bin.binary_write<unsigned>( Tok::END_TOK_FILE ); //
    bin.binary_write<unsigned>( 0 ); //
}

DotSarToDotTok::~DotSarToDotTok() {
    delete stack_prev;
}

void DotSarToDotTok::push_type_and_offsets( const Lexem *l, unsigned type, int nb_var_created_or_destroyed_in_stack ) {
    //pos_types_to_rellocate.push_back( bin.size() );
    
    bin.binary_write<unsigned>( type ); // type
    bin.binary_write<unsigned>( l->s - sar_txt ); // offset_in_sar_file
    
    for(int i=0;i<nb_var_created_or_destroyed_in_stack;++i)
        stack_prev->push_unnamed_var();
    for(int i=0;i>nb_var_created_or_destroyed_in_stack;--i)
        stack_prev->pop();
}

int DotSarToDotTok::push_string( const char *s, unsigned si ) {
    RelData red( s, si );
    if ( red.n.v >= NB_PREALLOCATED_STRINGS ) {
        pos_strings_to_rellocate.push_back( bin.size() );
        strings_to_rellocate.insert( red, red.n.v );
    }
    bin.binary_write( red.n ); // write to bin
    return red.n.v;
}

void DotSarToDotTok::push_tok_variable_( const Lexem *l, const RelData &red ) {
    int scope_pos = stack_prev->find_var_in_scope( red.n.v );
    if ( scope_pos < 0 ) { // found in scope
        push_type_and_offsets( l, Tok::VARIABLE_IN_STACK, 1 );
        bin.binary_write<int>( scope_pos );
    }
    else {
        int num_args = stack_prev->find_var_in_args( red.n.v );
        num_args = -1; // warning
        if ( num_args >= 0 and not stack_prev->of_class ) { // found in args
            push_type_and_offsets( l, Tok::VARIABLE_IN_ARGS, 1 );
            bin.binary_write<int>( num_args );
        }
        else { // not found
            push_type_and_offsets( l, Tok::VARIABLE_NAMED, 1 );
            if ( red.n.v >= NB_PREALLOCATED_STRINGS ) {
                pos_strings_to_rellocate.push_back( bin.size() );
                strings_to_rellocate.insert( red, red.n.v );
            }
            bin.binary_write<int>( red.n.v );
        }
    }
    
    // now, we know the name of the last variable in stack
    stack_prev->items.back().var = red.n.v;

}

void DotSarToDotTok::push_tok_variable( const Lexem *l, int n ) {
    push_tok_variable_( l, RelData( NULL, 0, n ) );
}

void DotSarToDotTok::push_tok_variable( const Lexem *l, const char *s, unsigned si ) {
    push_tok_variable_( l, RelData( s, si ) );
}


void DotSarToDotTok::push_block( const Lexem *l ) {
    for(;l;l=l->next)
        push_tok( l, false );
}

void DotSarToDotTok::push_tok_if_calc( const Lexem *n, int &nb_calculated_names, int nstring_conv ) {
    if ( n and n->type != Lexem::VARIABLE ) {
        push_type_and_offsets( n, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
        push_tok_variable( n, nstring_conv ); // String
        //
        if ( n->type == STRING_calc_name_NUM ) push_tok( n->children[0], true );
        else                                   push_tok( n, true );
        //
        push_tok_apply( n, 1, true );
        
        //
        ++nb_calculated_names;
    }
}

void DotSarToDotTok::push_tok_generic_apply( const Lexem *l, bool expect_result ) {
    // preparation ( get nb... and ch list )
    SplittedVec<const Lexem *,32> ch;
    get_children_of_type( next_if_CR( l->children[1] ), STRING___comma___NUM, ch );
    unsigned nb_unnamed_children = 0, nb_named_children = 0;
    for(; nb_unnamed_children<ch.size() and ch[nb_unnamed_children]->type!=STRING_reassign_NUM; ++nb_unnamed_children );
    for(unsigned i=nb_unnamed_children;i<ch.size();++i,++nb_named_children) {
        if ( ch[i]->type!=STRING_reassign_NUM ) {
            error_list->add( "after a named argument, all arguments must be named", ch[i]->s, provenance );
            return;
        }
    }
    
    // particular case : assertion
    Lexem *f = l->children[0];
    if ( nb_unnamed_children==2 and nb_named_children==0 and expect_result==false and f->type == Lexem::VARIABLE and f->si == 6 and strncmp( f->s, "assert", 6 ) == 0 ) {
        // cond
        push_type_and_offsets( ch[0], Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1);
        push_tok_variable( ch[0], STRING_not_boolean_NUM );
        push_tok( ch[0], true ); // cond
        push_tok_apply( ch[0], 1, /*expect_result*/true );
        
        // if
        unsigned os = bin.size();
        push_type_and_offsets( ch[0], Tok::get_IF_OR_WHILE_type(/*want_while*/false), 0 );
        bin.binary_write<unsigned>( 0 ); // unsigned *offset_if_fail
        unsigned *offset_if_not_executed = bin.binary_write<unsigned>( 0 );
        
        //
        push_tok_variable( ch[1], STRING_throw_failed_assertion_NUM );
        push_tok( ch[1], true );
        push_tok_apply( ch[1], 1, false );
        
        // end if
        push_type_and_offsets( ch[0], Tok::END_OF_AN_IF_BLOCK, 0 );
        *offset_if_not_executed = bin.size() - os;
        return;
    }
    
    // PUSH_ROOM_FOR_NEW_VARIABLE
    if ( expect_result )
        push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
    
    // function (Definition or...)
    push_tok( f, true );
    
    // children as tokens
    for(unsigned i=0;i<nb_unnamed_children;++i)
        push_tok( ch[i], true );
    for(unsigned i=0;i<nb_named_children;++i)
        push_tok( ch[nb_unnamed_children+i]->children[1], true );
    
    // calculated names ( f( @toto = 10 ) )
    int nb_calculated_names = 0;
    for(unsigned i=0;i<nb_named_children;++i)
        push_tok_if_calc( ch[nb_unnamed_children+i]->children[0], nb_calculated_names, STRING_String_NUM );
    
    
    // tok
    int t = ( l->type == Lexem::APPLY ? Tok::APPLY : ( l->type == Lexem::SELECT ? Tok::SELECT : Tok::CHANGE_BEHAVIOR ) );
    
    push_type_and_offsets( l, t, - 1 - nb_unnamed_children - nb_named_children - nb_calculated_names );
    bin.binary_write<unsigned>( Tok::Apply::EXPECT_RES * expect_result );
    bin.binary_write<unsigned>( nb_unnamed_children );
    bin.binary_write<unsigned>( nb_named_children );
    for(unsigned i=0;i<nb_named_children;++i) // names
        push_string_or_get_calc( ch[nb_unnamed_children+i]->children[0] );
}

int DotSarToDotTok::push_string_or_get_calc( const Lexem *l ) {
    if ( l and l->type == Lexem::VARIABLE )
        return push_string( l->s, l->si );
    bin.binary_write<int>( -1 );
    return -1;
}

void DotSarToDotTok::push_to_and_or_or( const Lexem *l, bool expect_result ) {
    SplittedVec<const Lexem *,8> ch;
    get_children_of_type( l, l->type, ch );
    
    SplittedVec<int *,8> ends;
    SplittedVec<unsigned,8> ends_os;
    
    // first condition
    stack_prev->push_something_unknown();
    
    push_tok( ch[0], true );
    push_type_and_offsets( l, Tok::CONVERSION_TO_BOOL, 1 );
    //
    for(unsigned i=1;i<ch.size();++i) {
        // OR / AND tok
        ends_os.push_back( bin.size() );
        push_type_and_offsets( l, Tok::get_AND_OR_OR_type( l->type == STRING_or_boolean_NUM ), - 2 );
        ends.push_back( bin.binary_write<int>(0) ); // offset to fail
        // condition
        if ( i<ch.size()-1 ) {
            push_tok( ch[i], true );
            push_type_and_offsets( l, Tok::CONVERSION_TO_BOOL, 1 );
        }
        else
            push_tok( ch[i], expect_result );
    }
    for(unsigned i=0;i<ends.size();++i)
        *ends[i] = bin.size() - ends_os[i];
}

void DotSarToDotTok::push_tok_assign( const Lexem *l ) {
    bool want_ref = l->children[1]->type == STRING___ref___NUM;
    bool only_type = ( l->type == STRING_assign_type_NUM );
    
    // rhs
    push_tok( want_ref ? l->children[1]->children[0] : l->children[1], 1 );
    
    // lhs
    int nb_calculated_names = 0;
    push_tok_if_calc( l->children[0], nb_calculated_names, STRING_String_NUM );
    
    // tok
    push_type_and_offsets( l, Tok::get_ASSIGN_type(nb_calculated_names!=0,only_type,stack_prev->has_unknown_var() or stack_prev->main_one), - nb_calculated_names ); // type
    bin.binary_write<unsigned>( want_ref * Tok::Assign::WANT_REF + static_ * Tok::Assign::STATIC + const_ * Tok::Assign::CONST + virtual_ * Tok::Assign::VIRTUAL ); // attributes
    bin.binary_write<unsigned>( offset_to_doc_of( l, sar_txt ) ); // doc
    int str = push_string_or_get_calc( l->children[0] ); // name
    
    if ( not stack_prev->assign_name_to_last_var( str ) )
        error_list->add("There's already a variable named '"+std::string(Nstring(str))+"' in corresponding scope.",l->children[0]->s,provenance);
}

void DotSarToDotTok::push_tok_apply( const Lexem *l, unsigned nb_unnamed, bool expect_result ) {
    push_type_and_offsets( l, Tok::APPLY, - 1 - nb_unnamed );
    bin.binary_write<unsigned>( Tok::Apply::EXPECT_RES * expect_result ); // attributes
    bin.binary_write<unsigned>( nb_unnamed ); // nb_unnamed
    bin.binary_write<unsigned>( 0 ); // nb_named
}

void DotSarToDotTok::push_tok_get_attr( const Lexem *l, bool ptr, bool partial_instanciation ) {
    
    // lhs
    if ( partial_instanciation )
        push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
    
    if ( ptr ) {
        push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
        push_type_and_offsets( l, Tok::VARIABLE_NAMED, 1 );
        bin.binary_write<int>( STRING_pointed_value_NUM );
    }
    

    //
    push_tok( l->children[0], 1 );
    
    //
    if ( ptr )
        push_tok_apply( l->children[0], 1, 1 );
    
    if ( partial_instanciation )
        push_type_and_offsets( l, Tok::PARTIAL_INSTANCIATION, -1 ); // type
    
    // calculated name ?
    int nb_calculated_names = 0;
    push_tok_if_calc( l->children[1], nb_calculated_names, STRING_String_NUM );
    
    
    // tok
    push_type_and_offsets( l, Tok::GET_ATTR, -nb_calculated_names );
    push_string_or_get_calc( l->children[1] );
    stack_prev->items.back().var = 0;
}
    
void DotSarToDotTok::push_default_arguments_values( const Lexem *l ) {
    for( unsigned i=l->def->min_nb_arg+l->def->self_as_arg; i<l->def->arguments.size(); ++i ) {
        if ( l->def->arguments[i].default_value ) {
            stack_prev->max_num_arg_that_can_be_used = i;

            int os = bin.size();
            push_type_and_offsets( l, Tok::PROPOSITION_OF_DEFAULT_VALUE, 0 );
            bin.binary_write<unsigned>( i - l->def->self_as_arg ); // num_arg
            int *offset_to_next_inst = bin.binary_write<int>( 0 ); //
        
            //
            push_tok( l->def->arguments[i].default_value, true );
        
            push_type_and_offsets( l, Tok::ASSIGN_TO_ARG_NB, 0 );
            bin.binary_write<unsigned>( i - l->def->self_as_arg ); // num_arg
        
            //
            *offset_to_next_inst = bin.size() - os;
        }
    }
    stack_prev->max_num_arg_that_can_be_used = l->def->arguments.size();
    stack_prev->push_something_unknown(); // ...
}


void DotSarToDotTok::push_tok_definition_try_block( const Lexem *l ) {
    // 
    push_default_arguments_values( l );
    
    // calculated pertinence
    if ( l->def->pertinence ) {
        push_tok( l->def->pertinence, true );
        push_type_and_offsets( l, Tok::PERTINENCE_IN_TRY_BLOCK, 0 ); // type
    }
    
    // condition
    if ( l->def->condition ) {
        push_tok( l->def->condition, true );
        push_type_and_offsets( l->def->condition, Tok::CONDITION_IN_TRY_BLOCK, 0 ); // type
    }
    
    // OK
    push_type_and_offsets( l, Tok::END_TRY_BLOCK, false );
}

void DotSarToDotTok::push_tok_definition( const Lexem *l ) {
    unsigned os = bin.size();
    
    // names which may be pre-calculated
    int nb_calculated_names = 0;
    push_tok_if_calc( l->def->tname, nb_calculated_names, STRING_String_NUM );
    for( unsigned i = l->def->self_as_arg; i < l->def->arguments.size() - l->def->varargs; ++i ) // argument names
        push_tok_if_calc( l->def->arguments[i].tname, nb_calculated_names, STRING_String_NUM );
    
    //
    push_type_and_offsets( l, Tok::DEFINITION, 1 /* - nb_calculated_names */ + ( l->def->offset_to_property_name > 0 ) );
    // attributes
    bin.binary_write<unsigned>(
            Tok::Definition::CLASS * l->def->is_class +
            Tok::Definition::STATIC * ( static_ or l->def->is_class ) +
            Tok::Definition::VIRTUAL * virtual_ +
            Tok::Definition::VARARGS * l->def->varargs +
            Tok::Definition::SELF_AS_ARG * l->def->self_as_arg + 
            Tok::Definition::ABSTRACT * l->def->abstract + 
            Tok::Definition::METHOD * ( stack_prev->of_class != 0 )
                              );
    // name
    int str_name = push_string_or_get_calc( l->def->tname );
    stack_prev->assign_name_to_last_var( str_name );
    
    // offset_to_doc
    bin.binary_write<unsigned>( offset_to_doc_of( l, sar_txt ) );
    // nb_variable_needed_in_stack
    unsigned *size_needed_in_stack = bin.binary_write<unsigned>( 0 );
    // pertinence
    if ( l->def->abstract )
        bin.binary_write<Float64>( FLOAT64_MIN_VAL );
    else
        bin.binary_write<Float64>( l->def->default_pertinence );
    // min_nb_args
    bin.binary_write<unsigned>( l->def->min_nb_arg );
    // property_name
    if ( l->def->offset_to_property_name > 0 )
        push_string( l->def->tname->s + l->def->offset_to_property_name, l->def->tname->si - l->def->offset_to_property_name );
    else
        bin.binary_write<int>( 0 );
    // type_property
    bin.binary_write<int>( l->def->type_property );
    // offset_to_try_block
    unsigned *beg_try_block = bin.binary_write<unsigned>( 0 );
    // offset_to_block
    unsigned *beg_block = bin.binary_write<unsigned>( 0 );
    // offset_to_next_inst
    unsigned *end_def = bin.binary_write<unsigned>( 0 );
    // args
    SplittedVec<int,16> args;
    bin.binary_write<unsigned>( l->def->arguments.size() - l->def->varargs - l->def->self_as_arg );
    for( unsigned i = l->def->self_as_arg; i < l->def->arguments.size() - l->def->varargs; ++i )
        args.push_back( push_string_or_get_calc( l->def->arguments[i].tname ) );
    
    //
    
    // ---------------------------------- try block ----------------------------------
    if ( l->def->needs_try_block() ) {
        StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev; for(unsigned i=0;i<args.size();++i) stack_prev->push_arg( args[i] );
        if ( l->def->varargs ) stack_prev->push_arg( STRING_varargs_NUM );

        *beg_try_block = bin.size() - os;
        push_tok_definition_try_block( l );
    
        *size_needed_in_stack = std::max( *size_needed_in_stack, stack_prev->needed_size );
        delete stack_prev; stack_prev = old_stack_prev;
    }
            
    // ------------------------------------ block ------------------------------------
    AutoRestore<bool> as( static_ ); static_ = false;
    StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev;
    if ( not l->def->is_class ) {
        for(unsigned i=0;i<args.size();++i) stack_prev->push_arg( args[i] );
        if ( l->def->varargs ) stack_prev->push_arg( STRING_varargs_NUM );
    }
    if ( l->def->is_class )
        stack_prev->of_class = str_name;
    
    *beg_block = bin.size() - os;
    // copy of template arguments
    if ( l->def->is_class ) {
        for(unsigned num_args=0;num_args<l->def->arguments.size();++num_args) {
            push_type_and_offsets( l, Tok::VARIABLE_IN_ARGS, 1 );
            bin.binary_write<int>( num_args );
            //
            push_type_and_offsets( l, Tok::get_ASSIGN_type(false,false,true), 0 ); // type
            bin.binary_write<unsigned>( Tok::Assign::STATIC + Tok::Assign::CONST ); // attributes
            bin.binary_write<unsigned>( 0 ); // doc
            int str = push_string_or_get_calc( l->def->arguments[num_args].tname ); // name
            stack_prev->assign_name_to_last_var( str );
        }
    }
    // inheritance
    for(unsigned i=0;i<l->def->inheritance.size();++i) {
        push_type_and_offsets( l->def->inheritance[i], Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
        push_tok( l->def->inheritance[i], true );
        push_type_and_offsets( l->def->inheritance[i], Tok::PARTIAL_INSTANCIATION, -1 );
        push_type_and_offsets( l->def->inheritance[i], Tok::APPEND_INHERITANCE_DATA, -1 );
        stack_prev->push_something_unknown();
    }
    // block
    if ( not l->def->abstract )
        push_block( l->def->block );
    push_type_and_offsets( l, l->def->is_class ? Tok::END_CLASS_BLOCK : Tok::END_DEF_BLOCK, false );
    
    *size_needed_in_stack = std::max( *size_needed_in_stack, stack_prev->needed_size );
    delete stack_prev; stack_prev = old_stack_prev;
    
    //
    *end_def = bin.size() - os;
}

/// 4 possible operators: , ; space return -> at most a 4D array
void DotSarToDotTok::get_array_data( const Lexem *t, SplittedVec<const Lexem *,32> &ch, SplittedVec<unsigned,4> &dim ) {
    while ( t and t->type==Lexem::CR )
        t = t->next;
    if ( not t )
        return;

    // we begin by guessing cardinality and dimension
    unsigned d[4] = {0,1,1,1};

    // skip returns at the beginning
    while ( t and t->type==Lexem::CR )
        t = t->next;
    if ( not t )
        return;

    // look for a ;
    const Lexem *tc = t;
    if ( t->type == STRING___comma_dot___NUM ) {
        SplittedVec<const Lexem *,8> children_comma_dot;
        get_children_of_type( t, STRING___comma_dot___NUM, children_comma_dot );
        if ( children_comma_dot.size() ) {
            tc = children_comma_dot[0];
            d[2] = children_comma_dot.size();
        }
    }
    
    // look for a ,
    if ( tc->type == STRING___comma___NUM )
        d[3] = nb_children_of_type( tc, STRING___comma___NUM );


    // find d[1] (number of carriage returns)
    for(const Lexem *o=t;o;o=o->next)
        d[1] += ( o->type == Lexem::CR and o->next and o->next->type != Lexem::CR );
     
    // find dim[0] number of elements splitted by spaces
    for(const Lexem *o=t;o and o->type != Lexem::CR;o=o->next)
        d[0]++;
    
    // ---------------------------------------------------------
    // get children
    // ch.reserve( d[0]*d[1]*d[2]*d[3] );
    for(const Lexem *o=t;o;o=o->next) {
        if ( o->type == STRING___comma_dot___NUM ) {
            SplittedVec<const Lexem *,8> children_comma_dot;
            get_children_of_type( o, STRING___comma_dot___NUM, children_comma_dot );
            if ( children_comma_dot.size() != d[2] ) {
                error_list->add( "Non-concordant rank of data specified around ';'.", o->s, provenance );
                return;
            }
            if ( children_comma_dot.size() and children_comma_dot[0]->type == STRING___comma___NUM ) {
                for(unsigned i=0;i<children_comma_dot.size();++i) {
                    SplittedVec<const Lexem *,8> children_comma;
                    get_children_of_type( children_comma_dot[i], STRING___comma___NUM, children_comma );
                    if ( children_comma.size() != d[3] ) {
                        error_list->add( "Non-concordant rank of data specified around ','.", children_comma_dot[i]->s, provenance );
                        return;
                    }
                    for(unsigned i=0;i<children_comma.size();++i)
                        ch.push_back( children_comma[i] );
                }
            } else {
                for(unsigned i=0;i<children_comma_dot.size();++i)
                    ch.push_back( children_comma_dot[i] );
            }
        } else if ( o->type == STRING___comma___NUM ) {
            SplittedVec<const Lexem *,8> children_comma;
            get_children_of_type( o, STRING___comma___NUM, children_comma );
            if ( children_comma.size() != d[3] ) {
                error_list->add( "Non-concordant rank of data specified around ','.", o->s, provenance );
                return;
            }
            for(unsigned i=0;i<children_comma.size();++i)
                ch.push_back( children_comma[i] );
        } else if ( o->type != Lexem::CR ) {
            ch.push_back( o );
        }
    }

    for(int i=3;i>=0;--i)
        if ( d[i] > 1 )
            dim.push_back( d[i] );
    // if all dim == 1
    if ( dim.size() == 0 )
        dim.push_back( 1 );
}

void DotSarToDotTok::push_tok_make_vec( const Lexem *l ) {
    SplittedVec<const Lexem *,32> ch;
    SplittedVec<unsigned,4> dim;
    get_array_data( l->children[0], ch, dim );
    
    //
    push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
    //
    push_tok_variable( l, STRING_make_little_heterogeneous_array_NUM );
    // dim
    push_type_and_offsets( l, Tok::NUMBER_INT32, 1 ); // type
    bin.binary_write<int>( dim.size() ); // attributes
    // sizes
    for(unsigned i=0;i<dim.size();++i) {
        push_type_and_offsets( l, Tok::NUMBER_INT32, 1 ); // type
        bin.binary_write<int>( dim[i] ); // attributes
    }
    // data
    for(unsigned i=0;i<ch.size();++i)
        push_tok( ch[i], true );
    //
    push_tok_apply( l, 1 + dim.size() + ch.size(), true );
}

void DotSarToDotTok::push_tok_for( const Lexem *l ) {
    if ( l->children[0]->type != STRING___in___NUM ) {
        error_list->add("syntax of 'for' is 'for ... in ... ...'.",l->children[0]->s,provenance);
        return;
    }
    
    // names
    SplittedVec<const Lexem *,8> names;
    get_children_of_type( l->children[0]->children[0], STRING___comma___NUM, names );
    
    // list.__for__
    if ( l->children[0]->children[1]->type == STRING___comma___NUM ) {
        SplittedVec<const Lexem *,32> ch;
        get_children_of_type( l->children[0]->children[1], STRING___comma___NUM, ch );
            
        push_type_and_offsets( l->children[0]->children[1], Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 ); //
        push_tok_variable( l->children[0]->children[1], STRING_zip_NUM );
        for(unsigned i=0;i<ch.size();++i)
            push_tok( ch[i], true );
        push_tok_apply( l->children[0]->children[1], ch.size(), true );
    } else
        push_tok( l->children[0]->children[1], true );
    push_type_and_offsets( l, Tok::GET_ATTR, 0 );
    bin.binary_write<int>( STRING___for___NUM );
    stack_prev->items.back().var = 0;
    
    // calculated names
    int nb_calculated_names = 0; // 1 for the list tok
    for(unsigned i=0;i<names.size();++i)
        push_tok_if_calc( names[i], nb_calculated_names, STRING_String_NUM );

    // for tok
    unsigned os = bin.size();
    push_type_and_offsets( l, Tok::get_FOR_type(/*want_tuple*/l->children[0]->children[0]->type == STRING___comma___NUM), - 1 - nb_calculated_names );
    unsigned *end = bin.binary_write<unsigned>( 0 ); // offset_to_next_inst
    bin.binary_write<unsigned>( names.size() ); // names
    SplittedVec<int,8> names_str;
    for(unsigned i=0;i<names.size();++i)
        names_str.push_back( push_string_or_get_calc( names[i] ) );
    
    // new stack prev
    StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev; // looking in parent scopes is forbidden
    // CHECK_ROOM_IN_VARIABLE_STACK
    push_type_and_offsets( l->children[1], Tok::CHECK_ROOM_IN_VARIABLE_STACK, 0 );
    unsigned *room_for_var = bin.binary_write<unsigned>( 0 ); //
        
    // block
    for(unsigned i=0;i<names.size();++i)
        stack_prev->push_named_var( names_str[i] );
    push_block( child_if_block( l->children[1] ) );
    push_type_and_offsets( l, Tok::END_OF_A_FOR_BLOCK, false );
    *end = bin.size() - os;

    *room_for_var = stack_prev->needed_size;
    old_stack_prev->needed_size += stack_prev->needed_size;
    delete stack_prev; stack_prev = old_stack_prev;
    
}

void DotSarToDotTok::push_tok_lambda( const Lexem *l ) {
    unsigned os = bin.size();
    push_type_and_offsets( l, Tok::LAMBDA, 0 ); // type
    unsigned *end = bin.binary_write<unsigned>( 0 ); // offset_to_next_inst
    // names
    SplittedVec<const Lexem *,8> names;
    get_children_of_type( child_if_paren( l->children[0] ), STRING___comma___NUM, names );
    bin.binary_write<unsigned>( names.size() ); // names
    for(unsigned i=0;i<names.size();++i)
        push_string( names[i]->s, names[i]->si );
    
    // tokens
    push_tok( l->children[1], true );
    push_type_and_offsets( l, Tok::DEF_RETURN, 0 ); // type
    push_type_and_offsets( l, Tok::END_DEF_BLOCK, 0 ); // type
    
    *end = bin.size() - os;
}

void DotSarToDotTok::push_tok_label( const Lexem *l ) {
    // calculated name
    int nb_calculated_names;
    push_tok_if_calc( l->children[0], nb_calculated_names, STRING_String_NUM );
    
    //
    unsigned os = bin.size();
    push_type_and_offsets( l, Tok::LABEL, 1 ); // type
    unsigned *end = bin.binary_write<unsigned>( 0 ); // offset_to_next_inst
    push_string_or_get_calc( l->children[0] ); // name label
    
    // block
    StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev( old_stack_prev );
    
    push_block( child_if_block( l->children[1] ) );
    push_type_and_offsets( l, Tok::END_OF_A_LABEL_BLOCK, 0 );
    
    delete stack_prev; stack_prev = old_stack_prev;
    
    *end = bin.size() - os;
}

bool acts_like_an_if(const Lexem *l) { return ( l->type == STRING___if___NUM or l->type == STRING___while___NUM ); }

template<class TV> void get_if_data( const Lexem *t, TV &ch, TV &conditions, const char *provenance, ErrorList *error_list ) {
    // -> STRING___if___NUM
    if ( acts_like_an_if(t) ) { // end of recursion
        conditions.push_back( t->children[0] );
        ch.push_back( child_if_block(t->children[1]) );
        return;
    }
    // -> STRING___else___NUM
    if ( t->type != STRING___else___NUM ) {
        error_list->add( "Operator should be an 'else'", t->s, provenance );
        return;
    }
    if ( acts_like_an_if(t->children[0]) or t->children[0]->type == STRING___else___NUM )
        get_if_data( t->children[0], ch, conditions, provenance, error_list );

    if ( acts_like_an_if(t->children[1]) )
        get_if_data( t->children[1], ch, conditions, provenance, error_list );
    else // else
        ch.push_back( child_if_block(t->children[1]) );
}

void DotSarToDotTok::push_tok_if_rec( const Lexem *l, const SplittedVec<const Lexem *,8> &ch, const SplittedVec<const Lexem *,8> &conditions, unsigned n, unsigned &max_needed_size_in_stack ) {
    bool want_while = false;
    // condition
    int offset_before_condition = bin.size();
    int *offset_if_fail = NULL, *offset_if_not_executed = NULL, offset_before_IF_OR_WHILE_tok = 0;
    if ( n < conditions.size() ) {
        const Lexem *cp = conditions[n]->parent;
        want_while = ( cp->type == STRING___while___NUM );
        //        
        push_tok( conditions[n], true );
        offset_before_IF_OR_WHILE_tok = bin.size();
        push_type_and_offsets( l, Tok::get_IF_OR_WHILE_type( want_while ), 0 );
        offset_if_fail = bin.binary_write<int>( 0 ); // offset_if_fail
        offset_if_not_executed = bin.binary_write<int>( 0 ); // offset_if_not_executed
    }
    // ok block
    StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev( old_stack_prev );
    
    push_block( ch[n] );
    
    max_needed_size_in_stack = std::max( max_needed_size_in_stack, stack_prev->needed_size );
    delete stack_prev; stack_prev = old_stack_prev;
    
    // fail block
    int os_before_END_OF_OK_PART_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE = 0, *offset_to_end_else_block = NULL;
    if ( n+1 < ch.size() ) {
        os_before_END_OF_OK_PART_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE = bin.size();
        if ( want_while ) {
            push_type_and_offsets( l, Tok::END_OF_A_WHILE_BLOCK_FOLLOWED_BY_AN_ELSE, 0 );
            bin.binary_write<int>( offset_before_condition - os_before_END_OF_OK_PART_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE ); // offset_to_condition
        }
        else
            push_type_and_offsets( l, Tok::END_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE, 0 );
        offset_to_end_else_block = bin.binary_write<int>( 0 ); // offset_to_end_else_block
        
        if ( offset_if_fail )
            *offset_if_fail = bin.size() - offset_before_IF_OR_WHILE_tok;
        push_tok_if_rec( l, ch, conditions, n+1, max_needed_size_in_stack );
    }
    
    if ( n < conditions.size() ) {
        if ( want_while ) {
            int sbewnb = bin.size();
            push_type_and_offsets( l, Tok::get_END_OF_A_WHILE_BLOCK_type(/*in else block*/n+1 < ch.size()), 0 );
            bin.binary_write<int>( offset_before_condition - sbewnb ); // offset_to_condition
        }
        else
            push_type_and_offsets( l, Tok::END_OF_AN_IF_BLOCK, 0 );
    }
    
    if ( n+1 < ch.size() )
        *offset_to_end_else_block = bin.size() - os_before_END_OF_OK_PART_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE;
    
    if ( offset_if_not_executed )
        *offset_if_not_executed = bin.size() - offset_before_IF_OR_WHILE_tok;
    
    
    if ( n < conditions.size() )
        stack_prev->pop(); // we eat the condition. miam

    if ( want_while ) stack_prev->push_something_unknown();
}

void DotSarToDotTok::push_tok_if( const Lexem *l ) {
    //
    SplittedVec<const Lexem *,8> ch, conditions;
    get_if_data( l, ch, conditions, provenance, error_list );
    //
    unsigned max_var_stack_size = 0;
    
    push_tok_if_rec( l, ch, conditions, 0, max_var_stack_size );

    // size needed for tmp variables
    stack_prev->needed_size += max_var_stack_size;
}

void DotSarToDotTok::push_tok_string( const Lexem *l ) {
    push_type_and_offsets( l, Tok::STRING, true ); // type
    
    std::string parsed_string; parsed_string.reserve( l->si );
    int nb_spaces = l->num;
    for(unsigned i=0;i<l->si;++i) {
        // if from <<< -> remove spaces at the beginning of the line
        if ( nb_spaces and ( l->s[i] == '\n' or i==0 ) ) {
            if ( l->s[i] == '\n' )
                parsed_string.push_back( l->s[i++] );
            for(int j=0;j<nb_spaces and l->s[i]==' ';++j,++i);
            i -= bool(i);
            continue;
        }
        //
        if ( l->s[i] == '\\' and i<l->si-1 ) {
            if ( l->num == 0 ) {
                if ( l->s[i+1]=='n' ) { parsed_string.push_back( '\n' ); ++i; continue; }
                if ( l->s[i+1]=='r' ) { parsed_string.push_back( '\r' ); ++i; continue; }
                if ( l->s[i+1]=='t' ) { parsed_string.push_back( '\t' ); ++i; continue; }
                if ( l->s[i+1]=='"' ) { parsed_string.push_back( '"'  ); ++i; continue; }
            }
            if ( l->s[i+1]=='$' ) { parsed_string.push_back( '$'  ); ++i; continue; }
            if ( l->s[i+1]=='0' ) {
                unsigned o = 0;
                while ( type_char( l->s[++i] ) == TYPE_CHAR_number )
                    o = o*10 + ( l->s[i] - '0' );
                parsed_string.push_back( (char)o ); --i; continue;
            }
            if ( l->s[i+1]=='\\' ) { parsed_string.push_back( '\\' ); ++i; continue; }
        }
        parsed_string.push_back( l->s[i] );
    }
    
    bin.binary_write<unsigned>( parsed_string.size() ); // length
    bin.append( &parsed_string[0], parsed_string.size() ); // data
    for(unsigned i=parsed_string.size(); i%sizeof(unsigned); ++i ) // alignement
        bin.binary_write<char>( 0 );
}

void DotSarToDotTok::push_tok_operator( const Lexem *l, bool expect_result ) {
    // PUSH_ROOM_FOR_NEW_VARIABLE
    if ( expect_result )
        push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
    
    // corresponding definition
    push_tok_variable( l, cpp_name_of_operator(l->type), cpp_name_size_of_operator(l->type) );
    
    // children
    if ( l->children[0] ) push_tok( l->children[0], true );
    if ( l->children[1] ) push_tok( l->children[1], true );

    // apply
    push_tok_apply( l, (l->children[0]!=NULL)+(l->children[1]!=NULL), expect_result );
}

void DotSarToDotTok::push_tok_new( const Lexem *l, bool expect_result ) {
    const Lexem *class_to_instantiate = l->children[0];
    //
    SplittedVec<const Lexem *,32> ch;
    unsigned nb_unnamed_children = 0, nb_named_children = 0;
    if ( l->children[0]->type == Lexem::APPLY ) {
        class_to_instantiate = l->children[0]->children[0];
        get_children_of_type( next_if_CR( l->children[0]->children[1] ), STRING___comma___NUM, ch );
        for(; nb_unnamed_children<ch.size() and ch[nb_unnamed_children]->type!=STRING_reassign_NUM; ++nb_unnamed_children );
        for(unsigned i=nb_unnamed_children;i<ch.size();++i,++nb_named_children) {
            if ( ch[i]->type!=STRING_reassign_NUM ) {
                error_list->add( "after a named argument, all arguments must be named", ch[i]->s, provenance );
                return;
            }
        }
    }
    
    // new
    if ( expect_result )
        push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
    push_tok_variable( l, STRING___new___NUM );
    push_tok( class_to_instantiate, true );
    for(unsigned i=0;i<nb_unnamed_children;++i)
        push_tok( ch[i], true );
    for(unsigned i=0;i<nb_named_children;++i)
        push_tok( ch[nb_unnamed_children+i]->children[1], true );
    
    // calculated names ( f( @toto = 10 ) )
    int nb_calculated_names = 0;
    for(unsigned i=0;i<nb_named_children;++i)
        push_tok_if_calc( ch[nb_unnamed_children+i]->children[0], nb_calculated_names, STRING_String_NUM );
    
    //
    push_type_and_offsets( l, Tok::APPLY, - 2 - nb_unnamed_children - nb_named_children - nb_calculated_names );
    bin.binary_write<unsigned>( Tok::Apply::EXPECT_RES * expect_result );
    bin.binary_write<unsigned>( 1 + nb_unnamed_children );
    bin.binary_write<unsigned>( nb_named_children );
    for(unsigned i=0;i<nb_named_children;++i) // names
        push_string_or_get_calc( ch[nb_unnamed_children+i]->children[0] );
}

void DotSarToDotTok::push_tok_get_value_prop( const Lexem *l ) {
    push_tok( l->children[0], true );
    push_type_and_offsets( l, Tok::GET_VALUE_PROP, 0 );
}

void DotSarToDotTok::push_tok_try( const Lexem *l ) {
    unsigned os = bin.size();
    push_type_and_offsets( l, Tok::TRY, 0 );
    unsigned *offset_to_next_inst = bin.binary_write<unsigned>( 0 );
    
    // block
    const Lexem *b = child_if_block( l->children[0] );
    StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev; // looking in parent scopes is forbidden
    // 
    push_block( b );
    push_type_and_offsets( b, Tok::END_TRY_EXCEPTION_BLOCK, 0 );
    // free stack
    old_stack_prev->needed_size += stack_prev->needed_size;
    delete stack_prev; stack_prev = old_stack_prev;
    
    // offset_to_next_inst
    *offset_to_next_inst = bin.size() - os;
}

void DotSarToDotTok::push_tok_catch( const Lexem *l ) {
    // name
    int nb_calculated_names = 0; // 1 for the list tok
    push_tok_if_calc( l->children[0], nb_calculated_names, STRING_String_NUM );
    
    //
    unsigned os = bin.size();
    push_type_and_offsets( l, Tok::CATCH, 0 );
    int name = push_string_or_get_calc( l->children[0] );
    unsigned *offset_to_next_inst = bin.binary_write<unsigned>( 0 );
    
    // block
    const Lexem *b = child_if_block( l->children[1] );
    StackPrev *old_stack_prev = stack_prev; stack_prev = new StackPrev; // looking in parent scopes is forbidden
    // variable name
    stack_prev->push_named_var( name );
    push_block( b );
    push_type_and_offsets( b, Tok::END_CATCH_BLOCK, 0 );
    // free stack
    old_stack_prev->needed_size += stack_prev->needed_size;
    delete stack_prev; stack_prev = old_stack_prev;
    
    // offset_to_next_inst
    *offset_to_next_inst = bin.size() - os;
}

void DotSarToDotTok::push_tok_doubledoubledot( const Lexem *l ) {
    push_tok_get_attr( l, false, true );
}

void DotSarToDotTok::push_tok( const Lexem *l, bool expect_result ) {
    switch( l->type ) {
        case Lexem::VARIABLE:
            if ( not expect_result ) error_list->add("lonely variable. Statement has no effect.",l->s,provenance);
            push_tok_variable( l, l->s, l->si );
            break;
        case Lexem::NUMBER: {
            if ( not expect_result ) error_list->add("lonely number. Statement has no effect.",l->s,provenance);
            
            bool it = l->number->has_e==false and l->number->expo==0 and (l->number->attributes & Tok::Number::IMAGINARY)==0;
            if ( it and ( l->number->attributes & Tok::Number::UNSIGNED ) and l->number->v <= Rationnal::BI(std::numeric_limits<unsigned>::max()) ) { // unsigned
                push_type_and_offsets( l, Tok::NUMBER_UNSIGNED32, 1 ); // type
                bin.binary_write<unsigned>( l->number->v ); // attributes
            }
            else if ( it and l->number->v <= Rationnal::BI(std::numeric_limits<int>::max()) ) { // int
                push_type_and_offsets( l, Tok::NUMBER_INT32, 1 ); // type
                bin.binary_write<int>( l->number->v ); // attributes
            }
            else { // generic number
                push_type_and_offsets( l, Tok::NUMBER, 1 ); // type
                bin.binary_write<unsigned>( l->number->attributes ); // attributes
                bin.binary_write<int>( l->number->expo ); // expo
                bin.binary_write<unsigned>( l->number->v.n + 1 ); // nb values
                bin.binary_write<unsigned>( l->number->v.val ); // val[0]
                for(unsigned i=0;i<l->number->v.n;++i)
                    bin.binary_write<unsigned>( l->number->v.ext[i] ); // val[i+1]
            }
            break; }   
        case Lexem::APPLY:
        case Lexem::SELECT:
        case Lexem::CHANGE_BEHAVIOR:
            push_tok_generic_apply( l, expect_result );
            break;
        case STRING_and_boolean_NUM:
        case STRING_or_boolean_NUM:
           push_to_and_or_or( l, expect_result );
           break;
        case STRING_assign_NUM:
        case STRING_assign_type_NUM:
            if ( expect_result ) error_list->add("an assignement does not return anything.",l->s,provenance);
            push_tok_assign( l );
            break;
        case STRING_get_attr_NUM:
        case STRING_get_attr_ptr_NUM:
            if ( not expect_result ) error_list->add("lonely attribute extraction. Statement has no effect.",l->s,provenance);
            push_tok_get_attr( l, l->type == STRING_get_attr_ptr_NUM, false );
            break;
        case STRING___def___NUM:
        case STRING___class___NUM:
            if ( expect_result ) error_list->add("a definition does not return anything.",l->s,provenance);
            push_tok_definition( l );
            break;
        case STRING_reassign_NUM: // a = 10
            if ( expect_result ) error_list->add("a reassignement does not return anything.",l->s,provenance);
            push_tok( l->children[0], true ); // a.reassign
            push_type_and_offsets( l, Tok::GET_ATTR, 0 );
            bin.binary_write<int>( STRING_reassign_NUM );
            stack_prev->items.back().var = 0;
            
            push_tok( l->children[1], true ); // call (with arg[0]=10)
            push_type_and_offsets( l, Tok::APPLY, - 2 );
            bin.binary_write<unsigned>( Tok::Apply::EXPECT_RES * 0 );
            bin.binary_write<unsigned>( /*nb_unnamed_children*/ 1 );
            bin.binary_write<unsigned>( /*nb_named_children*/ 0 );
            break;
        case STRING___property___NUM: {
            SplittedVec<const Lexem *,32> ch;
            get_children_of_type( l->children[0], STRING___comma___NUM, ch );
            for(unsigned i=0;i<ch.size();++i) {
                // -> assign true with static const attributes
                if ( expect_result ) error_list->add("'property something' does not return anything.",l->s,provenance);
                // true
                push_tok_variable( l, STRING_true_NUM );
                
                // calculated name ?
                int nb_calculated_names = 0;
                push_tok_if_calc( ch[i], nb_calculated_names, STRING_String_NUM );
                
                // tok
                push_type_and_offsets( l, Tok::get_ASSIGN_type(nb_calculated_names!=0,/*only_type*/false,stack_prev->has_unknown_var() or stack_prev->main_one), -nb_calculated_names ); // type
                bin.binary_write<unsigned>( Tok::Assign::STATIC + Tok::Assign::CONST + Tok::Assign::PROPERTY ); // attributes
                bin.binary_write<unsigned>( 0 ); // doc
                int str = push_string_or_get_calc( ch[i] ); // name
                
                if ( str >= 0 )
                    property_names.push_back( str );
            }
            break; }
        case STRING___static___NUM: {
            if ( expect_result ) error_list->add("'static something' does not return anything.",l->s,provenance);
            AutoRestore<bool> ob( static_ ); static_ = true;
            push_tok( l->children[0], false );
            break; }
        case STRING___virtual___NUM: {
            AutoRestore<bool> ob( virtual_ ); virtual_ = true;
            push_tok( l->children[0], expect_result );
            break; }
        case STRING___const___NUM:
            if ( expect_result ) {
                push_tok( l->children[0], true );
                push_type_and_offsets( l, Tok::CONSTIFY, 0 ); // type
            }
            else {
                AutoRestore<bool> ob( const_ ); const_ = true;
                push_tok( l->children[0], false );
            }
            break;
        case STRING___comma_dot___NUM: // a;b
            if ( expect_result ) error_list->add("an instruction block does not return anything.",l->s,provenance);
            push_tok( l->children[0], false );
            push_tok( l->children[1], false );
            break;
        case STRING___comma___NUM: { // a,b
            SplittedVec<const Lexem *,32> ch;
            get_children_of_type( l, STRING___comma___NUM, ch );
            
            if ( expect_result )
                push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 ); // preparation of apple
            for(unsigned i=0;i<ch.size();++i)
                push_tok( ch[i], true );
            push_type_and_offsets( l, Tok::get_MAKE_VARARGS_type( expect_result ), - ch.size() );
            bin.binary_write<unsigned>( /*nb_unnamed_children*/ ch.size() );
            bin.binary_write<unsigned>( /*nb_names*/ 0 );
            break; }
        case Lexem::PAREN: // (a), [a] or {a}
            if ( l->s[0] == '(' ) { // (a)
                if ( l->children[0] ) {
                    if ( l->children[0]->next )
                        error_list->add("between '(' and ')', several tokens are forbidden.",l->s,provenance);
                    push_tok( l->children[0], expect_result );
                }
            }
            else if ( l->s[0] == '[' ) // [a]
                push_tok_make_vec( l );
            else { // {a}
                std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
            }
            break;
        case Lexem::BLOCK: // 
            error_list->add("lonely block !!",l->s,provenance);
            break;
        case STRING___for___NUM:
            push_tok_for( l );
            break;
        case STRING_doubledoubledot_NUM:
            push_tok_doubledoubledot( l );
            break;
        case STRING___lambda___NUM:
            push_tok_lambda( l );
            break;
        case STRING___label___NUM:
            push_tok_label( l );
            break;
        case STRING___else___NUM:
        case STRING___while___NUM:
        case STRING___if___NUM:
            if (expect_result) error_list->add("branches do not return anything.",l->s,provenance);
            push_tok_if( l );
            break;
        case Lexem::STRING:
            if ( not expect_result )
                error_list->add("lonely string declaration. Statement has no effect.",l->s,provenance);
            push_tok_string( l );
            break;
        case STRING___pass___NUM:
            push_type_and_offsets( l, Tok::PASS, 0 ); // type
            break;
        case STRING___return___NUM:
            push_tok( l->children[0], true );
            push_type_and_offsets( l, Tok::DEF_RETURN, 0 ); // type
            break;
        case STRING___break___NUM:
        case STRING___continue___NUM:
            push_type_and_offsets( l, Tok::get_BREAK_OR_CONTINUE_type( l->type == STRING___continue___NUM ), 0 ); // type
            break;
        case STRING___self___NUM:
            push_type_and_offsets( l, Tok::SELF, 1 ); // type
            break;
        case STRING___this___NUM:
            push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
            push_tok_variable( l, "pointer_on", 10 );
            push_type_and_offsets( l, Tok::SELF, 1 ); // 
            push_tok_apply( l, 1, expect_result );
            break;
        case STRING___import___NUM:
        case STRING___include___NUM:
            push_tok( l->children[0], true );
            push_type_and_offsets( l, Tok::IMPORT, -1 ); // type
            break;
        case STRING___exec___NUM:
            push_tok( l->children[0], true );
            push_type_and_offsets( l, Tok::EXEC, -1 ); // type
            break;
        case STRING___throw___NUM:
            if ( expect_result ) error_list->add("a throw does not return anything.",l->s,provenance);
            push_tok( l->children[0], true );
            push_type_and_offsets( l, Tok::THROW, -1 ); // type
            break;
        case STRING___try___NUM:
            if ( expect_result ) error_list->add("a try does not return anything.",l->s,provenance);
            push_tok_try( l );
            break;
        case STRING___catch___NUM:
            if ( expect_result ) error_list->add("a catch does not return anything.",l->s,provenance);
            push_tok_catch( l );
            break;
        case STRING_partial_instanciation_NUM:
            push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 ); // type
            push_tok( l->children[0], true );
            push_type_and_offsets( l, Tok::PARTIAL_INSTANCIATION, -1 ); // type
            break;
        case STRING_pointed_value_NUM:
            if ( expect_result )
                push_type_and_offsets( l, Tok::PUSH_ROOM_FOR_NEW_VARIABLE, 1 );
            push_type_and_offsets( l, Tok::VARIABLE_NAMED, 1 );
            bin.binary_write<int>( STRING_pointed_value_NUM );
            push_tok( l->children[0], true ); // val
            push_tok_apply( l->children[0], 1, expect_result );
            break;
        case STRING___interact___NUM:
            push_type_and_offsets( l, Tok::INTERACT, 0 );
            break;
        case STRING___new___NUM:
            push_tok_new( l, expect_result );
            break;
        case STRING___get_value_prop___NUM:
            if ( not expect_result )
                error_list->add("lonely get_value_prop. Statement has no effect.",l->s,provenance);
            push_tok_get_value_prop( l );
            break;
        case STRING_exec_in_prev_scope_NUM:
            push_type_and_offsets( l, Tok::EXEC_IN_PREV_SCOPE, 0 );
            push_tok( l->children[0], expect_result );
            push_type_and_offsets( l, Tok::END_EXEC_IN_PREV_SCOPE, 0 );
            break;
        default: // type >= 0
            push_tok_operator( l, expect_result );
    }
}

