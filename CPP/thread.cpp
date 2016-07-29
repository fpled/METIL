#include "tok.h"
#include "thread.h"
#include "sourcefile.h"
#include "config.h"
#include "scope.h"
#include "errorlist.h"
#include "tokheader.h"
#include "globaldata.h"
#include "type.h"
#include "transientdata.h"
#include "definitiondata.h"
#include "definition.h"
#include "size_alignement.h"
#include "bool.h"
#include <usual_strings.h>
#include "property.h"
#include "varargs.h"
#include "block.h"
#include "system.h"
#include "cfile.h"
#include "mutex.h"
#include "DisplayWindowCreator.h"
#include "functionnal.h"
#include "md5.h"
#include "metil_interactive.h"
#include "symbolic/codewriter.h"
#include "symbolic/codewriteralt.h"
#include "symbolic/AsmWriter.h"
#include "symbolic/ex.h"
#include "lambda.h"
#include "label.h"
#include "compiledfunctionset.h"
#include "metil_sys_dir.h"
#include <assert.h>
#ifdef __APPLE__
  void *memalign( int align, int size ) { return malloc( size ); }
#else
  #include <malloc.h>
#endif
#include <sstream>
extern "C" {
    #include "mpi_wrap.h"
}

unsigned tok_end_def_block[] = { Tok::END_DEF_BLOCK, 0 };
unsigned tok_test_next_type[] = { Tok::TEST_NEXT_TYPE, 0 };

Thread::Thread( Scope *main_scope_, ErrorList *error_list_, Thread *parent_, unsigned prereserved_room_in_variable_stack ) {
    main_scope = main_scope_;
    error_list = error_list_;
    parent = parent_;
    if ( parent ) {
        types = parent->types;
        interpreter_behavior = parent->interpreter_behavior;
    }
    else {
        types = new SplittedVec<Type,1024,2048>;
    }
    //             parent->child_threads.push_back( this );
    
    cpt_use = 0;
    return_value = 0;
    
    beg_variable_stack = (Variable *)malloc( prereserved_room_in_variable_stack * sizeof(Variable) );
    end_variable_stack = beg_variable_stack + prereserved_room_in_variable_stack;
    current_variable_sp = beg_variable_stack;
    
    last_named_variable_in_current_scope = NULL;
    old_scope = NULL;
    
    current_tok = NULL;
    current_sourcefile = NULL;
    current_def_trial = NULL;
    current_self = NULL;
    compile_mode = false;
    profile_mode = false;
    max_recursive_depth = 500;
    
//     display_stack_file.open("ticks");
}

Thread::~Thread() {
    //         parent->child_threads.erase_elem_in_unordered_list( this );
    if ( global_data.Def and not parent ) {
        for(unsigned i=0;i<global_data.Def->nb_variables;++i) {
            //std::cout << global_data.Def->variables[ i ].v.type->name << std::endl;
            if ( global_data.Def->variables[ i ].v.type == global_data.Def )
                free( reinterpret_cast<Definition *>(global_data.Def->variables[ i ].v.data)->def_data );
            free( global_data.Def->variables[ i ].v.data );
        }
        //         for(unsigned i=0;i<global_data.Bool->nb_variables;++i) {
        //             //std::cout << global_data.Def->variables[ i ].v.type->name << std::endl;
        //             free( global_data.Bool->variables[ i ].v.data );
        //         }
        // free( Def );
        //         global_data.Error->destroy();
        global_data.Def->destroy();
        //         global_data.Def = NULL;
    }
    
    dec_ref( main_scope );
    dec_ref( error_list );
    
    free( beg_variable_stack );
    if ( not parent )
        delete types;
}

unsigned Thread::nb_def_trial() const {
    unsigned res = 0;
    for( DefTrial *d = current_def_trial; d; d=d->prev_def_trial ) ++res;
    return res;
}

void assign_error_var( Variable *v ) {
    v->type = global_data.Error;
    v->cpt_use = reinterpret_cast<int *>( &v->data );
    *v->cpt_use = 654;
}

void assign_void_var( Variable *v ) {
    v->type = global_data.Void;
    v->cpt_use = reinterpret_cast<int *>( &v->data );
    *v->cpt_use = 654;
}
   
void offset_if_necessary( Variable *beg_variable_stack, Variable *end_variable_stack, SignedSize diff, Variable * &v ) {
    if ( v and v >= beg_variable_stack and v < end_variable_stack )
        (char *&)v += diff;
}

void offset_var_stack( Variable *beg_variable_stack, Variable *end_variable_stack, SignedSize diff, Callable *c ) {
    if ( not c ) return;
    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, c->self );
    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, c->first_variable );
    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, c->condition );
    for(unsigned i=0;i<c->nb_args_including_varargs;++i)
        offset_if_necessary( beg_variable_stack, end_variable_stack, diff, c->arguments[i] );
}
   
SignedSize Thread::check_for_room_in_variable_stack( Variable * &sp, int nb_variables ) {
    if ( sp + nb_variables >= end_variable_stack ) {
        std::cerr << "!!!!!!!!!! changing stack size" << std::endl;
        
        unsigned new_size = ( end_variable_stack - beg_variable_stack ) * 8;
        Variable *new_beg_stack = (Variable *)realloc( beg_variable_stack, sizeof(Variable) * new_size );
        
        //
        SignedSize diff = (char *)new_beg_stack - (char *)beg_variable_stack;
        
        // offset Variable *
        if ( diff ) {
            (char * &)sp += diff;
            
            // for each variable...
            for( Variable *v=new_beg_stack; v<sp; ++v ) {
                if ( v->type == global_data.BlockOfTok ) {
                    Block *b = reinterpret_cast<Block *>( v->data );
                    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, b->last_named );
                    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, b->self       );
                }
            }
            // def_trial
            for( DefTrial *def_trial=current_def_trial; def_trial; def_trial = def_trial->prev_def_trial ) {
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, def_trial->def_v );
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, def_trial->first_default_variable );
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, def_trial->return_var );
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, def_trial->old_current_self );
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, def_trial->old_last_named_variable_in_current_scope );
                
                for(Variable *v = def_trial->old_last_named_variable_in_current_scope; v; v=v->prev_named )
                    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, v->prev_named );
                
                for(Callable *c=def_trial->first_callable_to_try;c;c=c->next)
                    offset_var_stack( beg_variable_stack, end_variable_stack, diff, c );
                    
                for(Callable *c=def_trial->first_ok_callable;c;c=c->next)
                    offset_var_stack( beg_variable_stack, end_variable_stack, diff, c );
                    
                offset_var_stack( beg_variable_stack, end_variable_stack, diff, def_trial->current_callable );
            }
            //
            for( OldScope *os=old_scope; os; os=os->prev ) {
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, os->variable_sp );
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, os->last_named_variable );
                for(Variable *v = os->last_named_variable; v; v=v->prev_named )
                    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, v->prev_named );
            }
            //
            offset_if_necessary( beg_variable_stack, end_variable_stack, diff, current_self );
            offset_if_necessary( beg_variable_stack, end_variable_stack, diff, last_named_variable_in_current_scope );
            for(Variable *v = last_named_variable_in_current_scope; v; v=v->prev_named )
                offset_if_necessary( beg_variable_stack, end_variable_stack, diff, v->prev_named );
            //
            for(unsigned i=0;i<main_scope->hash_size;++i) {
                if ( main_scope->hash_table[i] ) {
                    offset_if_necessary( beg_variable_stack, end_variable_stack, diff, main_scope->hash_table[i] );
                    for(Variable *v = main_scope->hash_table[i]; v; v=v->prev_named )
                        offset_if_necessary( beg_variable_stack, end_variable_stack, diff, v->prev_named );
                }
            }
        }
        beg_variable_stack = new_beg_stack;
        end_variable_stack = beg_variable_stack + new_size;
        
        return diff;
    }
    return 0;
}
SignedSize Thread::check_for_room_in_variable_stack( Variable * &sp, int nb_variables, Variable *&v0 ) {
    if ( v0 >= beg_variable_stack and v0 < end_variable_stack ) {
        SignedSize displacement = check_for_room_in_variable_stack( sp, nb_variables );
        v0 = reinterpret_cast<Variable *>( reinterpret_cast<char *>(v0) + displacement );
        return displacement;
    }
    return check_for_room_in_variable_stack( sp, nb_variables );
}
SignedSize Thread::check_for_room_in_variable_stack( Variable * &sp, int nb_variables, Variable *&v0, Variable *&v1 ) {
    bool cv0 =( v0 >= beg_variable_stack and v0 < end_variable_stack );
    bool cv1 =( v1 >= beg_variable_stack and v1 < end_variable_stack );

    SignedSize displacement = check_for_room_in_variable_stack( sp, nb_variables );
    if ( cv0 ) v0 = reinterpret_cast<Variable *>( reinterpret_cast<char *>(v0) + displacement );
    if ( cv1 ) v1 = reinterpret_cast<Variable *>( reinterpret_cast<char *>(v1) + displacement );
    return displacement;
}
    
void Thread::push_prev_file_to_read( SourceFile *sourcefile ) {
    if ( sourcefile and sourcefile->tok_header() and sourcefile->tok_header()->tok_data() ) {
        sourcefile->add_property_names( main_scope );
        
        OldPC *pc = old_pc_list.new_elem();
        pc->sourcefile = sourcefile;
        pc->tok = NULL;
        pc->next_tok = sourcefile->tok_header()->tok_data();
    }
}

const void *Thread::go_to_next_trial() {
    return NULL;
}
    
const void *Thread::pop_pc() {
    const void *tok = NULL;
    if ( old_pc_list.size() ) {
        OldPC &pc = old_pc_list.back();
        tok = pc.next_tok;
        set_current_sourcefile( pc.sourcefile );
        old_pc_list.pop_back();
    }
    return tok;
};
const void *Thread::pop_def_trial() {
    DefTrial *def_trial = current_def_trial;
    
    set_current_sourcefile( def_trial->old_sourcefile );
    last_named_variable_in_current_scope = def_trial->old_last_named_variable_in_current_scope;
    current_self = def_trial->old_current_self;
    info_stack.pop_until( (char *)def_trial );
    current_def_trial = def_trial->prev_def_trial;
    
    return def_trial->old_next_tok;
}
    
Variable *Thread::find_var_in_current_scope( Nstring name ) {
    if ( currently_in_main_scope() )
        return main_scope->find_var( name );
    for(Variable *v=last_named_variable_in_current_scope; v; v=v->prev_named)
        if ( v->name == name )
            return v;
    return NULL;
}

Variable *Thread::find_var( Nstring name ) {
    for(Variable *v=last_named_variable_in_current_scope; v; v=v->prev_named)
        if ( v->name == name )
            return v;
    return main_scope->find_var( name );
}
Variable *Thread::find_var_but_not_a_property_name_in_current_scope( Nstring name ) {
    for(Variable *v=last_named_variable_in_current_scope; v; v=v->prev_named)
        if ( v->name == name )
            return v;
    if ( last_named_variable_in_current_scope )
        return NULL;
    return main_scope->find_var_but_not_a_property_name( name );
}

    
int Thread::nb_accessible_variables() const {
    int res = main_scope->nb_accessible_variables();
    for(Variable *v=last_named_variable_in_current_scope; v; v=v->prev_named) ++res;
    return res;
}

Nstring Thread::accessible_variables_nb(unsigned n) const {
    for(Variable *v=last_named_variable_in_current_scope; v; v=v->prev_named)
        if ( n-- == 0 )
            return v->name;
    return main_scope->accessible_variables_nb(n);
}

void Thread::register_var_in_current_scope( const void *tok, Variable *v, Nstring name, bool need_verification ) {
    if ( currently_in_main_scope() ) {
        Variable *t = main_scope->find_var_but_not_a_property_name( name );
        if ( t )
            add_error( "There's already a variable named '"+std::string(name)+"' in current scope.", tok );
        main_scope->register_var( v, name );
    }
    else {
        if ( need_verification ) {
            if ( last_named_variable_in_current_scope and last_named_variable_in_current_scope->name == name and last_named_variable_in_current_scope->can_be_redefined() ) 
                last_named_variable_in_current_scope = last_named_variable_in_current_scope->prev_named;
            else {
                for( Variable *nv = last_named_variable_in_current_scope, *ov = NULL; nv; nv=nv->prev_named ) {
                    if ( nv->name == name ) {
                        if ( nv->can_be_redefined() )
                            ov->prev_named = nv->prev_named;
                        else
                            add_error( "There's already a variable named '"+std::string(name)+"' in current scope.", tok );
                        break;
                    }
                    ov = nv;
                }
            }
        }
        v->name = name;
        v->prev_named = last_named_variable_in_current_scope;
        last_named_variable_in_current_scope = v;
    }
}

void Thread::add_error( const std::string &msg, const void *tok, const std::vector<ErrorList::Provenance> &possibilities, unsigned nb_def_trial_to_skip ) {
    if ( not return_value )
        return_value = 201;
    
    ErrorList::Error e;
    e.warn = false;
    e.display_escape_sequences = error_list->display_escape_sequences;
    e.due_to_not_ended_expr = false;
    e.display_col = true;
    e.msg = msg;
    // current tok
    if ( nb_def_trial_to_skip == 0 ) {
        if ( tok and current_sourcefile ) {
            int offset_in_sar_file = reinterpret_cast<const int *>(tok)[1];
            const char *t = current_sourcefile->sar_text();
            if ( offset_in_sar_file < (int)strlen( t + 1 ) )
                e.caller_stack.push_back( ErrorList::Provenance( t ? t + offset_in_sar_file : t, current_sourcefile->provenance ) );
        }
    }
    
    // DefTrial data
    DefTrial *d = current_def_trial;
    while ( nb_def_trial_to_skip-- > 1 and d ) d = d->prev_def_trial;
    for( ; d; d=d->prev_def_trial ) {
        if ( d->old_tok and d->old_sourcefile ) {
            const char *t = d->old_sourcefile->sar_text();
            e.caller_stack.push_back( ErrorList::Provenance( t ? t + /*offset_in_sar_file*/ reinterpret_cast<const unsigned *>(d->old_tok)[1] : t, d->old_sourcefile->provenance ) );
        }
    }
    e.possibilities = possibilities;
    
    error_list->add( e );
    
    if ( not interpreter_behavior.interpretation )
        exit(1);
    //         metil_interactive( this, tok, tok, sp, "=> ", "-> " );
}

void Thread::rm_var_in_current_scope( Variable *v ) {
    if ( currently_in_main_scope() )
        main_scope->rm_var( v );
    else {
        if ( v == last_named_variable_in_current_scope )
            last_named_variable_in_current_scope = v->prev_named;
        else  {
            for(Variable *sv=last_named_variable_in_current_scope;sv->prev_named;sv=sv->prev_named) {
                if ( sv->prev_named == v ) {
                    sv->prev_named = v->prev_named;
                    return;
                }
            }
            //
            std::cout << "You're trying, using rm_var_in_current_scope to rm a var that is not in the current scope" << std::endl;
        }
    }
}

void Thread::push_var_if_to_be_destroyed( Variable *v, Variable * &sp ) {
    if ( *v->cpt_use == 0 ) {
        check_for_room_in_variable_stack( sp, 1, v );
        make_copy( sp++, v );
    }
    else
        v->dec_ref();
}

void Thread::push_var_in_variables_to_del_at_next_end_block_if_to_be_destroyed( Variable *v ) {
    if ( *v->cpt_use == 0 )
        make_copy( variables_to_del_at_next_end_block.new_elem(), v );
    else
        v->dec_ref();
}

bool Thread::currently_in_main_scope() const {
    return current_def_trial==NULL and old_scope==NULL;
}

bool Thread::replace_var_by_ext_method_named( Variable *v, Nstring name ) {
    // look up for an external method in current scope
    for(Variable *d=last_named_variable_in_current_scope; d; d=d->prev_named) {
        if ( d->name == name and d->type == global_data.Def ) {
            Definition *method = reinterpret_cast<Definition *>( d->data );
            for(DefinitionData *dd = method->def_data; dd; dd=dd->next_with_same_name ) {
                if ( dd->has_self_as_arg() ) {
                    v->replace_by_method( this, method );
                    return true;
                }
            }
        }
    }
    // and in main scope
    Variable *d = main_scope->find_var( name );
    if ( d and d->type == global_data.Def ) {
        Definition *method = reinterpret_cast<Definition *>( d->data );
        for(DefinitionData *dd = method->def_data; dd; dd=dd->next_with_same_name ) {
            if ( dd->has_self_as_arg() ) {
                v->replace_by_method( this, method );
                return true;
            }
        }
    }
    return false;
}

bool Thread::replace_var_by_ext_property_named( Variable *v, Nstring name ) {
    // look up for an external method in current scope
    for(Variable *d=last_named_variable_in_current_scope; d; d=d->prev_named) {
        if ( d->name == name and d->type == global_data.Property ) {
            Property *prop = reinterpret_cast<Property *>( d->data );
            v->replace_by_property( prop );
            return true;
        }
    }
    // and in main scope
    Variable *d = main_scope->find_var( name );
    if ( d and d->type == global_data.Property ) {
        Property *prop = reinterpret_cast<Property *>( d->data );
        v->replace_by_property( prop );
        return true;
    }
    return false;
}

Int32 __nb_stack_level__( const Thread *th, const void *tok ) {
    Int32 res = 0;
    for( DefTrial *d = th->current_def_trial; d; d=d->prev_def_trial )
        res += ( d->old_tok and d->old_sourcefile and d->old_sourcefile->sar_text() );
    return res;
}

ErrorList::Provenance __get_info_on_stack_level__( const Thread *th, const void *tok, Int32 level ) {
    for( DefTrial *d = th->current_def_trial; d; d=d->prev_def_trial )
        if ( d->old_tok and d->old_sourcefile and d->old_sourcefile->sar_text() and level-- == 0 )
            return ErrorList::Provenance( d->old_sourcefile->sar_text() + /*offset_in_sar_file*/ reinterpret_cast<const int *>(d->old_tok)[1], d->old_sourcefile->provenance );
    return ErrorList::Provenance( NULL, NULL );
}

void __file_at_stack_level__( Thread *th, const void *tok, Int32 level, Variable *return_var ) {
    ErrorList::Provenance e = __get_info_on_stack_level__( th, tok, level );
    if ( return_var ) assign_string( th, tok, return_var, e.provenance, strlen(e.provenance) );
}

Int32 __line_at_stack_level__( const Thread *th, const void *tok, Int32 level ) {
    ErrorList::Provenance e = __get_info_on_stack_level__( th, tok, level );
    return e.line;
}

Int32 __col_at_stack_level__( const Thread *th, const void *tok, Int32 level ) {
    ErrorList::Provenance e = __get_info_on_stack_level__( th, tok, level );
    return e.col;
}

void __line_str_at_stack_level__( Thread *th, const void *tok, Int32 level, Int32 nb_col_max, Variable *return_var ) {
    ErrorList::Provenance e = __get_info_on_stack_level__( th, tok, level );
    if ( return_var ) {
        assign_string( th, tok, return_var, e.complete_line, e.length_complete_line );
    }
}

int Thread::display_stack(const void *tok) {
    for(Int32 i=0;i<__nb_stack_level__(this, tok);++i) {
        ErrorList::Provenance e = __get_info_on_stack_level__( this, tok, i );
        std::cerr << i << " " << e.provenance << " " << e.line << " " << e.col << std::endl;
    }
    std::cerr << std::endl;
    return 0;
}


template<int compile_mode> const void *conv_last_var_to( const N<compile_mode> &n__compile_mode, Thread *th, const void *tok, Variable * &sp, Variable *target_class );

#include "generated_tok_methods.h"
#include "thread_functions.h"

template<int compile_mode> const void *conv_last_var_to( const N<compile_mode> &n__compile_mode, Thread *th, const void *tok, const void *next_tok, Variable * &sp, Variable *target_class ) {
    th->check_for_room_in_variable_stack( sp, 2 );
    make_copy( sp+1, sp-1 );
    assign_error_var( sp-1 );
    make_copy( sp, target_class );
    target_class->inc_ref();
    sp += 2;
    //
    return exec_tok_apply(n__compile_mode,th,tok,/*nb_unnamed*/1,/*nb_named*/0,/*names*/NULL,/*expect_res*/true,/*next_tok*/next_tok,sp);
}

template<int compile_mode,int profile_mode> bool thread_loop_( Thread *th, const N<compile_mode> &n__compile_mode, const N<profile_mode> &n__profile_mode ) {
    // restore state
    Variable *sp = th->current_variable_sp;
    const void *tok = th->current_tok;
    if ( not tok ) { tok = th->pop_pc(); }
    if ( not tok ) { static unsigned tok_by_default[] = { Tok::END_TOK_FILE, 0 }; tok = &tok_by_default; } // if there's no file to read
    
    // loop
    while ( true ) {
        if ( profile_mode )
            th->interpreter_behavior.prof_entries.add_tick( th );
//         if ( th->current_sourcefile )
//         std::cerr << th->current_sourcefile->provenance << ":"
//                  << ErrorList::Provenance( th->current_sourcefile->sar_text() + reinterpret_cast<const int *>(tok)[1],
//                                            th->current_sourcefile->provenance ).line << ":error" << std::endl;
        // std::cerr << "  "; Tok::disp( std::cerr, tok ); std::cerr << std::endl;
        switch ( *reinterpret_cast<const int *>(tok) ) {
            #include "cases_for_thread_loop.h"
        }
    }
    return true;
}

bool thread_loop( Thread *th ) {
    if ( th->compile_mode ) {
        if ( th->profile_mode )
            return thread_loop_( th, N<1>(), N<1>() );
        else
            return thread_loop_( th, N<1>(), N<0>() );
    }
    else {
        if ( th->profile_mode )
            return thread_loop_( th, N<0>(), N<1>() );
        else
            return thread_loop_( th, N<0>(), N<0>() );
    }
}
