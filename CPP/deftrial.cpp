#include "deftrial.h"
#include "type.h"
#include "variable.h"
#include "definition.h"
#include "definitiondata.h"
#include "thread.h"
#include "globaldata.h"
#include "scope.h"
#include <usual_strings.h>
#include "sourcefile.h"
#include "varargs.h"
#include <sstream>
#include <fstream>

void DefTrial::rm_callables_without_enough_pertinence() {
    // first element of the list
    while ( first_callable_to_try and first_callable_to_try->pertinence < min_pertinence_unconditionnal )
        first_callable_to_try = first_callable_to_try->next;
    //
    if ( first_callable_to_try ) {
        Callable *c = first_callable_to_try;
        while ( c->next ) {
            if ( c->next->pertinence < min_pertinence_unconditionnal )
                c->next = c->next->next;
            else
                c = c->next;
        }
    }
}

void DefTrial::push_in_ok_callable( Callable *c, bool fully_asserted ) {
    if ( fully_asserted )
        min_pertinence_unconditionnal = std::max( min_pertinence_unconditionnal, c->pertinence );
    // sorted list -> push after last p such as p->pertinence > c->pertinence
    if ( first_ok_callable ) {
        if ( c->pertinence >= first_ok_callable->pertinence ) {
            c->next = first_ok_callable;
            first_ok_callable = c;
        }
        else {
            for( Callable *p = first_ok_callable; ; p = p->next ) {
                if ( not p->next ) {
                    p->next = c;
                    c->next = NULL;
                    break;
                }
                if ( c->pertinence >= p->next->pertinence ) {
                    c->next = p->next;
                    p->next = c;
                    break;
                }
            }
        }
    }
    else {
        first_ok_callable = c;
        c->next = NULL;
    }
}

#define DL 
// #define DL std::cout << __LINE__ << std::endl

template<int is_class,int has_varargs> void append_def_data_if_basic_conditions_are_checked_( 
        const N<is_class> &n_is_class, const N<has_varargs> &n_has_varargs, Thread *th, Variable *&sp, DefinitionData *dd, unsigned &has_func_or_class ) {
    const bool is_function = not is_class;
    has_func_or_class |= 1 << is_class;
            
    DefTrial *def_trial = th->current_def_trial;
    DL;
    // ccode try function
    if ( dd->cpp_thread_try_func ) {
        dd->cpp_thread_try_func( th, def_trial, dd, sp );
        return;
    }
    DL;
    // static pertinence
    if ( dd->pertinence < def_trial->min_pertinence_unconditionnal ) return;
    DL;
    //
    unsigned nb_uargs = def_trial->def->nb_uargs + is_function*def_trial->nb_uargs;
    unsigned nb_nargs = def_trial->def->nb_nargs + is_function*def_trial->nb_nargs;
    DL;
    // not enough arguments ?
    if ( nb_uargs + nb_nargs < dd->min_nb_args ) return;
    DL;
    // too much arguments ? (we don't take named arguments into account because they can "surdefine" an already declared one)
    if ( has_varargs==false and nb_uargs > dd->nb_args ) return;
    DL;
    
    // need a self ?
    Variable *self = NULL;
    if ( dd->is_class()==false and ( dd->has_self_as_arg() or dd->is_method() ) ) {
        if ( def_trial->def->self.type )
            self = &def_trial->def->self;
        else if ( th->current_self )
            self = th->current_self;
        else
            return;
    }
    DL;
    
    // ELSE -> preparation of Callable *c
    unsigned room_for_callable = sizeof(Callable) + sizeof(Variable *) * ( dd->nb_args + has_varargs - 1 );
    Callable *c = (Callable *)th->info_stack.get_room_for( room_for_callable );
    c->def_data = dd;
    c->self = self;
    c->pertinence = dd->pertinence;
    c->cpp_thread_func = NULL;
    c->condition = NULL;
    c->nb_args_including_varargs = dd->nb_args + dd->has_varargs();
    c->first_variable = NULL;
    
    if ( has_varargs ) { // initialization of vararg
        th->check_for_room_in_variable_stack( sp, 1 );
        c->arguments[ dd->nb_args ] = sp;
        sp->init( global_data.VarArgs, 0 );
        reinterpret_cast<VarArgs *>( sp->data )->init();
        ++sp;
    }
    DL;
    
    // arguments
    for(unsigned i=0;i<dd->nb_args;++i) c->arguments[i] = NULL;
    for(unsigned i=0;i<def_trial->def->nb_nargs;++i) {
        for(unsigned j=0;;++j) {
            if ( j==dd->nb_args ) { // not found
                if ( has_varargs ) { reinterpret_cast<VarArgs *>( c->arguments[dd->nb_args]->data )->push_named( def_trial->def->args[i].name, def_trial->def->args + i + def_trial->def->nb_uargs ); break; }
                th->info_stack.pop_contiguous( room_for_callable ); return;
            }
            if ( def_trial->def->args[i + def_trial->def->nb_uargs].name == dd->args[j] ) { c->arguments[j] = def_trial->def->args + i + def_trial->def->nb_uargs; break; }
        }
    }
    for(unsigned i=0;i<is_function*def_trial->nb_nargs;++i) {
        for(unsigned j=0;;++j) {
            if ( j==dd->nb_args ) { // not found
                if ( has_varargs ) { reinterpret_cast<VarArgs *>( c->arguments[dd->nb_args]->data )->push_named( def_trial->names[i], def_trial->def_v + 1 + i + def_trial->nb_uargs ); break; }
                th->info_stack.pop_contiguous( room_for_callable ); return;
            }
            if ( def_trial->names[i] == dd->args[j] ) { c->arguments[j] = def_trial->def_v + 1 + i + def_trial->nb_uargs; break; }
        }
    }
    DL;
    // unnamed ones
    if ( has_varargs ) {
        for(unsigned i=0;i<def_trial->def->nb_uargs;++i)
            if ( i >= dd->nb_args or c->arguments[ i ] )
                reinterpret_cast<VarArgs *>( c->arguments[dd->nb_args]->data )->push_unnamed( def_trial->def->args + i );
            else
                c->arguments[ i ] = def_trial->def->args + i;
        for(unsigned i=0;i<is_function*def_trial->nb_uargs;++i) {
            unsigned j = i + def_trial->def->nb_uargs;
            if ( j >= dd->nb_args or c->arguments[ j ] )
                reinterpret_cast<VarArgs *>( c->arguments[dd->nb_args]->data )->push_unnamed( def_trial->def_v + i + 1 );
            else
                c->arguments[ j ] = def_trial->def_v + i + 1;
        }
    }
    else {
        for(unsigned i=0;i<def_trial->def->nb_uargs;++i) 
            c->arguments[ i ] = def_trial->def->args + i;
        for(unsigned i=0;i<is_function*def_trial->nb_uargs;++i)
            c->arguments[ i + def_trial->def->nb_uargs ] = def_trial->def_v + i + 1;
    }
    DL;
    // check mandatory args
    for(unsigned i=0;i<dd->min_nb_args;++i) {
        if ( not c->arguments[i] ) {
            th->info_stack.pop_contiguous( room_for_callable ); 
            return;
        }
    }
    DL;
    
    // register c
    if ( dd->try_block == NULL )
        def_trial->push_in_ok_callable( c, true );
    else
        def_trial->push_in_callable_to_try( c );
    DL;
}


void append_to_possibilities( DefinitionData *def_data, std::vector<ErrorList::Provenance> &possibilities ) {
    const char *txt = ( def_data->sourcefile ? def_data->sourcefile->sar_text() : NULL );
    possibilities.push_back( ErrorList::Provenance( def_data->offset_in_sar_file * (txt!=NULL) + txt,
                             (def_data->sourcefile ? def_data->sourcefile->provenance : NULL ) ) );
}
    
void get_all_possibilities( DefTrial *def_trial, Thread *th, std::vector<ErrorList::Provenance> &possibilities ) {
    DefinitionData *first_dd = def_trial->def->def_data;
    DefinitionData *dd = first_dd;
    Nstring name = dd->name;
    // found def
    append_to_possibilities( dd, possibilities );
    for( dd=dd->next_with_same_name; dd; dd=dd->next_with_same_name )
        append_to_possibilities( dd, possibilities );
    // parent scopes
    for( Variable *n_def_v = th->last_named_variable_in_current_scope; n_def_v; n_def_v = n_def_v->prev_named )
        if ( n_def_v->name == name and reinterpret_cast<Definition *>(n_def_v->data)->def_data != first_dd and n_def_v->type == global_data.Def and n_def_v->is_primary_def() )
            for( dd=reinterpret_cast<Definition *>(n_def_v->data)->def_data; dd; dd=dd->next_with_same_name )
                append_to_possibilities( dd, possibilities );
    // main scope
    Variable *n_def_v = th->main_scope->find_var( name );
    if ( n_def_v and reinterpret_cast<Definition *>(n_def_v->data)->def_data != first_dd and n_def_v->type == global_data.Def and n_def_v->is_primary_def() )
        for( dd=reinterpret_cast<Definition *>(n_def_v->data)->def_data; dd; dd=dd->next_with_same_name )
            append_to_possibilities( dd, possibilities );
    // self
    
}

void append_def_data_if_basic_conditions_are_checked( Thread *th, Variable *&sp, DefinitionData *dd, unsigned &has_func_or_class ) {
    if ( dd->has_varargs() ) {
        if ( dd->is_class() )
            append_def_data_if_basic_conditions_are_checked_( N<true>(), N<true>(), th, sp, dd, has_func_or_class );
        else
            append_def_data_if_basic_conditions_are_checked_( N<false>(), N<true>(), th, sp, dd, has_func_or_class );
    }
    else {
        if ( dd->is_class() )
            append_def_data_if_basic_conditions_are_checked_( N<true>(), N<false>(), th, sp, dd, has_func_or_class );
        else
            append_def_data_if_basic_conditions_are_checked_( N<false>(), N<false>(), th, sp, dd, has_func_or_class );
    }
}

void init_scope_for_callable( Thread *th, Callable *c ) {
    th->last_named_variable_in_current_scope = NULL;
    for(unsigned i=0;i<c->def_data->nb_args + c->def_data->has_varargs();++i) {
        if ( c->arguments[i] ) {
            c->arguments[i]->prev_named = th->last_named_variable_in_current_scope;
            c->arguments[i]->name = c->def_data->args[i];
            th->last_named_variable_in_current_scope = c->arguments[i];
        }
    }
}

const void *play_with_callables( Thread *th, DefTrial *def_trial, const void *tok, Variable *&sp ) {
    // some callables to test ?
    if ( def_trial->first_callable_to_try ) {
        Callable *c = def_trial->first_callable_to_try;
        def_trial->current_callable = c;
        def_trial->first_callable_to_try = c->next;
        
        th->current_self = c->self;
        init_scope_for_callable( th, c );
        th->set_current_sourcefile( c->def_data->sourcefile );
        return c->def_data->try_block;
    }
        
    // here we can assume that there's no trial block to execute
    return play_with_ok_callables_after_all_trials_are_done( def_trial, th, tok, sp );
}


void fill_callable_lists( DefTrial *def_trial, Thread *th, const void *tok, Variable *&sp ) {
    DefinitionData *first_dd = def_trial->def->def_data;
    DefinitionData *dd = first_dd;
    Nstring name = dd->name;
    
    //     static std::ofstream f_c( "f_c" );
    //     f_c << name << "\n";
    
    // found def
    SplittedVec<DefinitionData *,32,16> defs;
    defs.push_back( dd );
    for( dd=dd->next_with_same_name; dd; dd=dd->next_with_same_name )
        defs.push_back_unique( dd );
    // parent scopes
    for( Variable *n_def_v = th->last_named_variable_in_current_scope; n_def_v; n_def_v = n_def_v->prev_named )
        if ( n_def_v->name == name and reinterpret_cast<Definition *>(n_def_v->data)->def_data != first_dd and n_def_v->type == global_data.Def and n_def_v->is_primary_def() )
            for( dd=reinterpret_cast<Definition *>(n_def_v->data)->def_data; dd; dd=dd->next_with_same_name )
                defs.push_back_unique( dd );
    // main scope
    Variable *n_def_v = th->main_scope->find_var( name );
    if ( n_def_v and reinterpret_cast<Definition *>(n_def_v->data)->def_data != first_dd and n_def_v->type == global_data.Def and n_def_v->is_primary_def() )
        for( dd=reinterpret_cast<Definition *>(n_def_v->data)->def_data; dd; dd=dd->next_with_same_name )
            defs.push_back_unique( dd );
    // self
    
    // sort 
    for(unsigned i=0;i<defs.size();++i)
        for(unsigned j=i+1;j<defs.size();++j)
            if ( defs[i]->pertinence < defs[j]->pertinence )
                std::swap( defs[i], defs[j] );
    for(unsigned i=0;i<defs.size();++i)
        th->check_for_room_in_variable_stack( sp, defs[i]->size_needed_in_stack );
        
    // append_def_data_if_basic_conditions_are_checked
    unsigned has_func_or_class = 0;
    for(unsigned i=0;i<defs.size();++i)
        append_def_data_if_basic_conditions_are_checked( th, sp, defs[i], has_func_or_class );
    
    
    // check that we don't have function and classes with same names
    if ( has_func_or_class == 3 ) {
        std::vector<ErrorList::Provenance> possibilities;
        get_all_possibilities( def_trial, th, possibilities );
        th->add_error( "'"+std::string(name)+"' is used in the same time for at least one function or method and one class. It is forbidden.", tok, possibilities );
    }
    // remove callable without enough pertinence
    def_trial->rm_callables_without_enough_pertinence();
    
    
    // room for trial blocks
    def_trial->first_default_variable = reinterpret_cast<Variable *>( sp );
}

const void *instantiate( DefTrial *def_trial, Thread *th, const void *tok, Variable *&sp, Type *type ) {
    //
    if ( def_trial->def )
        def_trial->def->cached_type = type;
    
    // partial_instanciation
    if ( def_trial->partial_instanciation ) {
        if ( def_trial->return_var )
            def_trial->return_var->init( type, Variable::TYPE_OF_ONLY );
        if ( type->contains_virtual_methods )
            *reinterpret_cast< Type ** >( def_trial->return_var->data ) = type;
        //
        th->set_current_sourcefile( NULL );
        return tok_end_def_block;
    }
    // if error var in args -> return_var is an error
    Callable *c = def_trial->current_callable;
    if ( c ) {
        for(unsigned i=0;i<c->def_data->nb_args+c->def_data->has_varargs();++i) {
            if ( c->arguments[i] and c->arguments[i]->type == global_data.Error ) {
                if ( def_trial->return_var )
                    assign_error_var( def_trial->return_var );
                th->set_current_sourcefile( NULL );
                return tok_end_def_block;
            }
        }
    }
    
    //
    if ( not def_trial->return_var )
        def_trial->return_var = sp++;
    def_trial->return_var->init( type, 0 );
    th->current_self = def_trial->return_var;
                
    // call init
    th->check_for_room_in_variable_stack( sp, 1 );
    if ( *def_trial->def_v->cpt_use == 0 )
        make_copy( sp++, def_trial->def_v );
    else
        -- *def_trial->def_v->cpt_use;
    def_trial->return_var->inc_ref();
    make_copy( def_trial->def_v, def_trial->return_var );
    if ( not def_trial->def_v->replace_by_method_named( th, STRING_init_NUM ) )
        if ( not th->replace_var_by_ext_method_named( def_trial->def_v, STRING_init_NUM ) )
            std::cerr << "init not found !!" << std::endl;
    if ( def_trial->def_v->type != global_data.Def ) {
        th->add_error( "in classes 'init' must be a method or an attribute of type 'Def', not an attribute of type '"+std::string(def_trial->def_v->type->name)+"'.", tok );
        th->set_current_sourcefile( NULL );
        return tok_end_def_block;
    }
    def_trial->def = reinterpret_cast<Definition *>( def_trial->def_v->data );
    def_trial->return_var = NULL;
    def_trial->first_callable_to_try = NULL;
    def_trial->first_ok_callable = NULL;
    def_trial->min_pertinence_unconditionnal = FLOAT64_MIN_VAL;
    fill_callable_lists( def_trial, th, tok, sp );
        
    return play_with_callables( th, def_trial, tok, sp );
}

const void *test_static_args( Thread *th, const void *tok, Callable *c, Variable *&sp ) {
    Type *t = c->def_data->type_cache[c->testing_type_nb];
    
    // Variable *bool_var = th->main_scope->find_var( STRING_Bool_NUM );
    unsigned nargs = c->def_data->nb_args + c->def_data->has_varargs();
    
    th->check_for_room_in_variable_stack( sp, 1 + 4 * nargs );
    
    Variable *equal_def_var = th->main_scope->find_var( STRING__1_andreassign_2__eq__3_NUM );
    
    // result here
    Variable *res = sp++;
    res->init( global_data.Bool, 0 );
    Bool( res->data, 1 << 7-res->get_bit_offset() ) = true;
    
    // call sequence
    for(unsigned num_arg=0;num_arg<nargs;++num_arg) {
        TypeVariable *tv = t->find_var( NULL, c->def_data->args[num_arg] );
        if ( tv ) {
            assign_ref_on( sp++, equal_def_var ); // _1_andreassign_2__eq__3
            assign_ref_on( sp++, res ); // arg
            assign_ref_on( sp++, c->arguments[num_arg] ); // arg
            assign_ref_on( sp++, &tv->v ); // type->...
        }
        else
            std::cout << "Weird : unable to find class arg in static attributes !!" << std::endl;
    }
    if ( th->compile_mode )
        return exec_tok_apply( N<1>(), th, tok, /*nb_unnamed*/3,/*nb_named*/0,/*named*/NULL, /*expect_res*/false, tok_test_next_type, sp, nargs );
    return exec_tok_apply( N<0>(), th, tok, /*nb_unnamed*/3,/*nb_named*/0,/*named*/NULL, /*expect_res*/false, tok_test_next_type, sp, nargs );
}

const void *play_with_ok_callables_after_all_trials_are_done( DefTrial *def_trial, Thread *th, const void *tok, Variable *&sp ) {
    // if ambiguity or no possibilities ( and not in destroy context )
    if ( def_trial->def ) { // 
        if ( def_trial->first_ok_callable==NULL or ( def_trial->first_ok_callable->next and def_trial->first_ok_callable->pertinence==def_trial->first_ok_callable->next->pertinence) ) {
            if ( def_trial->first_ok_callable==NULL ) {
                // look in property names
                if ( def_trial->def->def_data and def_trial->def->def_data->property_name.v > 0 ) {
                    if ( th->main_scope->has_prop( def_trial->def->def_data->property_name ) ) {
                        if ( def_trial->return_var ) {
                            def_trial->return_var->init( global_data.Bool, Variable::CONST );
                            Bool( def_trial->return_var->data, 1 << 7-def_trial->return_var->get_bit_offset() ) = false;
                            th->set_current_sourcefile( NULL );
                            return tok_end_def_block;
                        }
                        else {
                            th->add_error( "You're trying to modify a property which is a static const variable.", tok );
                        }
                    }
                }
                
                //
                bool has_error = false;
                for(unsigned i=1;i<=def_trial->     nb_uargs+def_trial->     nb_nargs;++i) has_error |= ( def_trial->def_v    [i].type==global_data.Error );
                for(unsigned i=0;i< def_trial->def->nb_uargs+def_trial->def->nb_nargs;++i) has_error |= ( def_trial->def->args[i].type==global_data.Error );
                if ( has_error == false ) {
                    std::vector<ErrorList::Provenance> possibilities;
                    get_all_possibilities( def_trial, th, possibilities );
                    bool has_self = def_trial->def->self.type or th->current_self;
                    th->add_error( ( has_self ? "Impossible to find function, class or method '" : "Impossible to find function or class '" ) + def_trial->to_string() +
                            "' with checked conditions.", tok, possibilities, 1 );
                }
            }
            else { // several possibilities
                std::vector<ErrorList::Provenance> possibilities;
                for(Callable *c=def_trial->first_ok_callable;c;c=c->next)
                    append_to_possibilities( c->def_data, possibilities );
                bool has_self = def_trial->def->self.type or th->current_self;
                th->add_error( ( has_self ? "Ambiguous overload for function, class or method '" : "Ambiguous overload for function or class '" ) + def_trial->to_string() +
                        "'.", tok, possibilities, 1 );
            }
            if ( def_trial->return_var )
                assign_error_var( def_trial->return_var );
            th->set_current_sourcefile( NULL );
            return tok_end_def_block;
        }
    }
    
    // execution 
    Callable *c = def_trial->first_ok_callable;
    // remove front element of def_trial->first_ok_callable
    def_trial->current_callable = c;
    def_trial->first_ok_callable = c->next;
    c->first_variable = reinterpret_cast<Variable *>( sp );
    // set variables in thread
    th->current_self = c->self;
    if ( c->def_data->parent_type ) 
        th->current_self->type = c->def_data->parent_type;
        
    if ( not c->def_data->is_class() )
        init_scope_for_callable( th, c );
    else
        th->last_named_variable_in_current_scope = NULL;
    
    if ( def_trial->current_callable->cpp_thread_func ) {
        //th->display_stack( tok );
        return def_trial->current_callable->cpp_thread_func( th, tok, def_trial->return_var, 
            def_trial->current_callable->arguments, def_trial->current_callable->self, sp );
    }
    //
    if ( c->def_data->is_class() ) {
        // simple case : nb args == 0
        if ( c->def_data->nb_args==0 ) {
            // -> no need to execute neither 'class' block nor 'test_same_args' block
            if ( c->def_data->type_cache.size()==1 ) 
                return instantiate( def_trial, th, tok, sp, c->def_data->type_cache[0] );
            // else -> type construction
            th->set_current_sourcefile( c->def_data->sourcefile );
            return c->def_data->block;
        }
        // other simple case : no type instanciated
        if ( c->def_data->type_cache.size()==0 ) {
            th->set_current_sourcefile( c->def_data->sourcefile );
            return c->def_data->block;
        }
        // else, we have to test arguments
        c->testing_type_nb = 0;
        return test_static_args( th, tok, c, sp );
    }
    // else -> simple function
    if ( def_trial->partial_instanciation )
       th->add_error( "partial_instanciation works only with classes (which is not the case of "+std::string(c->def_data->name)+"). Return value will be used but not destroyed", tok );
    
    // init and virtual ->
    if ( c->self and c->def_data->name == STRING_init_NUM and c->self->type->contains_virtual_methods )
        *reinterpret_cast<Type **>( c->self->data ) = c->self->type;
    
    if ( c->def_data->is_abstract() )
        th->add_error( "Abstract function (type="+std::string( c->self->type->name )+")", tok );
    
    th->set_current_sourcefile( c->def_data->sourcefile );
    return c->def_data->block;
}

std::string DefTrial::to_string() const {
    std::ostringstream ss;
    
    if ( def->self.type )
        ss << def->self.type->to_string() << ".";
    
    // def
    ss << *def;
    
    // args
    Variable *uargs = def_v + 1, *nargs = uargs + nb_uargs;
    ss << '(';
    for(unsigned i=0;i<nb_uargs;++i)
        ss << uargs[i].type->to_string() << ( i+1 < nb_uargs+nb_nargs ? "," : "" );
    for(unsigned i=0;i<nb_nargs;++i)
        ss << names[i] << "=" << nargs[i].type->to_string() << ( i+1 < nb_uargs+nb_nargs ? "," : "" );
    ss << ')';
    
    return ss.str();
}

void DefTrial::push_in_callable_to_try( Callable *c ) {
    c->next = first_callable_to_try;
    first_callable_to_try = c;
}

const void *end_of_a_try_block( Thread *th, bool ok, const void *tok, Variable *&sp ) {
    DefTrial *def_trial = th->current_def_trial;
    if ( ok ) {
        Callable *c = def_trial->current_callable;
        def_trial->push_in_ok_callable( c, /*fully asserted*/ c->condition == NULL );
    }
    return play_with_callables( th, def_trial, tok, sp );
}

