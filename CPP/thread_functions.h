#include <metil_sys_dir.h>

template<int compile_mode> const void *exec_tok_end_tok_file(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_tok_file *tok,Variable * &sp) {
    // pop PC
    const void *test_tok = th->pop_pc();
    if ( test_tok )
        return test_tok;
    
    // there's nothing more to read -> THIS IS THE END
    //     if ( th->child_threads.size() )
    //         th->add_error( "There are unfinished threads in current program. Please use pthread_join( thread ).", tok );
        
    //
    while ( th->variables_to_del_at_next_end_block.size() )
        if ( ( test_tok = destroy_var( th, tok, tok, sp, &th->variables_to_del_at_next_end_block.pop_back() ) ) != NULL )
            return test_tok;
    // -> look for variables to delete
    while ( sp > th->beg_variable_stack )
        if ( ( test_tok = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return test_tok;
    if ( th->variables_to_del_at_next_end_block.size() )
        return tok;
    // -> remaining type with circular refs ?
    //     while ( th->types.size() ) {
    //         Type &t = th->types.back();
    //         if ( &t != global_data.Def ) {
    //             if ( t.cpt_use >= 0 ) {
    //                 if ( t.nb_variables ) {
    //                     while ( t.nb_variables ) {
    //                         Variable *v = &t.variables[ --t.nb_variables ].v;
    //                         th->push_var_in_variables_to_del_at_next_end_block_if_to_be_destroyed( v );
    //                     }
    //                     std::cout << " " << t.name << std::endl;
    //                     t.destroy();
    //                     th->types.pop_back();
    //                     return tok;
    //                 }
    //            }
    //            t.destroy();
    //         }
    //         th->types.pop_back();
    //     }
    //     if ( th->variables_to_del_at_next_end_block.size() )
    //         return tok;

    // -> bye bye
    return NULL;
}
template<int compile_mode> const void *exec_tok_push_room_for_new_variable(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Push_room_for_new_variable *tok,Variable * &sp) {
    assign_void_var( sp++ );
    return tok->next();
}
template<int compile_mode> const void *exec_tok_check_room_in_variable_stack(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Check_room_in_variable_stack *tok,Variable * &sp) {
    th->check_for_room_in_variable_stack( sp, tok->nb_variables );
    return tok->next();
}
void get_variable_named(Thread *th,const void *tok,Variable *return_var,Nstring name) {
    // current scope
    for(Variable *v=th->last_named_variable_in_current_scope; v; v=v->prev_named) {
        if ( v->name == name ) {
            assign_ref_on( return_var, v );
            return;
        }
    }
    // self
    if ( th->current_self ) {
        TypeVariable *tv = th->current_self->type->find_var( th->current_self, name );
        if ( tv ) {
            Variable *v = return_var;
            if ( tv->v.type == global_data.Def and tv->v.is_primary_def() ) {
                assign_ref_on( return_var, th->current_self );
                v->replace_by_method( th, reinterpret_cast<Definition *>( tv->v.data ) );
            }
            else if ( tv->v.type == global_data.Property ) {
                assign_ref_on( return_var, th->current_self );
                v->replace_by_property( reinterpret_cast<Property *>( tv->v.data ) );
            }
            else if ( tv->v.is_static() ) {
                assign_ref_on( return_var, &tv->v );
            }
            else {
                assign_ref_on( return_var, th->current_self );
                v->replace_by_non_static_attr( tv );
            }
            return;
        }
    }
    
    // static variables in englobing defs hors classes
    
    // main scope
    Variable *v = th->main_scope->find_var( name );
    if ( v )
        assign_ref_on( return_var, v );
    else {
        th->add_error( "Impossible to find any variable named '"+std::string(name)+"'.", tok );
        // for(Variable *v = th->last_named_variable_in_current_scope; v; v=v->prev_named) std::cout << v->name << std::endl;
        assign_error_var( return_var );
    }
}

template<int compile_mode> const void *exec_tok_variable_named(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Variable_named *tok,Variable * &sp) {
    get_variable_named( th, tok, sp++, tok->name );
    return tok->next();
}

void calc_name( Thread *th, const void *tok, Variable *return_var, const char *s, unsigned si ) {
    if ( return_var )
        get_variable_named( th, tok, return_var, Nstring( s, si ) );
}

template<int compile_mode> const void *exec_tok_variable_in_args(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Variable_in_args *tok,Variable * &sp) {
    assign_ref_on( sp++, th->current_def_trial->current_callable->arguments[tok->num_arg] );
    return tok->next();
}
template<int compile_mode> const void *exec_tok_variable_in_stack(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Variable_in_stack *tok,Variable * &sp) {
    assign_ref_on( sp++, sp + tok->offset_in_nb_variables );
    return tok->next();
}

template<int compile_mode> const void *exec_replace_prop( const N<compile_mode> &n__compile_mode, Thread *th, Variable * &sp, const void *tok, const void *tok_next ) {
    Property *p = reinterpret_cast<Property *>( sp[-1].data );
    // std::cerr << p->name_() << std::endl;
    if ( p->get_().v == 0 ) {
        //         th->register_var_in_current_scope( tok, sp-1, tok->name, need_verification );
        th->add_error( "Can't assign (without ref) a property without get_... method (looking for a 'get_"+std::string(p->name_())+"' one).", tok );
        const void *tok_del = rm_last_var_in_sp( th, tok, tok_next, sp ); if ( tok_del ) return tok_del;
        return tok_next;
    }
    int *cpt_use = sp[-1].cpt_use;
    if ( *cpt_use==0 and ( sp[-1].data!=sp[-1].parent_data or sp[-1].type!=sp[-1].parent_type ) )
        std::cout << "TODO: del_p" << __FILE__ << " " << __LINE__ << std::endl;
    //
    th->check_for_room_in_variable_stack( sp, 1 );
    assign_void_var( sp - 1 );
    if ( p->self.type ) {
        Variable *o = sp++;
        make_copy( o, &p->self ); // copy of object on which Property applies
        if ( *cpt_use ) o->inc_ref();
        if ( not o->replace_by_method_named( th, p->get_() ) ) // a.get_...
            if ( not th->replace_var_by_ext_method_named( o, p->get_() ) )
                th->add_error( "pb with 'get_"+std::string(p->name_())+"'.", tok );
    }
    else {
        Variable *o = th->find_var( p->get_() );
        if ( not o ) {
            th->add_error( "at this point of file, it is impossible to find any function named '"+std::string( p->get_() )+"'.", tok );
            assign_error_var( sp - 1 );
            return tok_next;
        }
        assign_ref_on( sp++, o );
    }
    if ( *cpt_use==0 ) { dec_ref(th,global_data.Property); free( p ); } else -- *cpt_use;
    return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok, sp );
}

template<int compile_mode,int calculated_name,int type_,int need_verification> const void *exec_tok_assign(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Assign *tok,Variable * &sp,
        const N<calculated_name> &n__calculated_name,const N<type_> &n__type,const N<need_verification> &n__need_verification) {

    unsigned wanted_attributes = tok->has_attr_const() * Variable::CONST + tok->has_attr_static() * Variable::STATIC + tok->has_attr_virtual() * Variable::VIRTUAL + type_ * Variable::TYPE_OF_ONLY;
    
    // Functionnal in argument
    if ( sp[-1].type == global_data.Functionnal and tok->has_attr_want_ref() == false and *sp[-1].cpt_use ) {
        if ( type_ )
            th->add_error( "a Functionnal can't produce a partial instanciation.", tok );
        int *old_cpt_use = sp[-1].cpt_use;
        assert( *old_cpt_use or ( sp[-1].data == sp[-1].parent_data and sp[-1].type == sp[-1].parent_type ) );
        Functionnal *old_functionnal = reinterpret_cast<Functionnal *>( sp[-1].data );
        sp[-1].init( global_data.Functionnal, wanted_attributes );
        Functionnal *new_functionnal = reinterpret_cast<Functionnal *>( sp[-1].data );
        new_functionnal->init( old_functionnal, false );
        if ( (*old_cpt_use)-- == 0 ) free( old_functionnal );
        return tok->next();
    }

    if ( sp[-1].type == global_data.Property and tok->has_attr_want_ref() == false )
        return exec_replace_prop( n__compile_mode, th, sp, (const void *)tok, tok->next() );
            
    th->register_var_in_current_scope( tok, sp-1, tok->name, need_verification );
    if ( tok->has_attr_property() ) th->main_scope->add_property( tok->name );

    if ( type_ ) {
        // a ~= Int32
        th->check_for_room_in_variable_stack( sp, 1 );
        make_copy( sp, sp-1 ); // copy of Def
        ++sp;
        return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok->next(), sp, /*partial_instanciation*/true, 1, /*return_var*/NULL, wanted_attributes );
    }
            
    if ( calculated_name ) {
        th->add_error( "calculated_name", tok );
    }
    assert( calculated_name == 0 );
    
    sp[-1].attributes = wanted_attributes;
    
    // if we don't want a reference and variable is used elsewhere, we have to make a copy
    if ( tok->has_attr_want_ref()==false and *sp[-1].cpt_use ) {
        th->check_for_room_in_variable_stack( sp, 2 );
        sp->init( sp[-1].type, wanted_attributes ); // partial instanciation a.type
        make_copy( sp+1, sp-1 ); // b as first arg of init
        make_copy( sp-1, sp ); // partial instanciation a.type
        sp[-1].attributes &= ~Variable::TYPE_OF_ONLY;
        sp->inc_ref();
        sp->attributes &= ~Variable::CONST;
        if ( not sp->replace_by_method_named( th, STRING_init_NUM ) ) // a.init
            if ( not th->replace_var_by_ext_method_named( sp, STRING_init_NUM ) )
                std::cerr << "init not found !!" << std::endl;;
        sp += 2;
        return exec_tok_apply( n__compile_mode, th, tok, 
                               /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                               /*expect_res*/false, tok->next(), sp );
    }
    // else
    sp[-1].attributes = wanted_attributes;
    return tok->next();
}
static int tok_interact[] = {Tok::INTERACT,0};

template<int compile_mode> const void *exec_tok_apply(const N<compile_mode> &n__compile_mode,Thread *th,const void *tok,unsigned nb_unnamed,unsigned nb_named,
        const Nstring *named,bool expect_res,const void *next_tok,Variable * &sp,unsigned nb_calls,bool partial_instanciation,Variable *return_var,unsigned supplementary_attributes) {
    
    if ( th->nb_def_trial() == th->max_recursive_depth ) {
        std::ostringstream ss; ss << "max recursive depth (=" << th->max_recursive_depth << ") excedeed.";
        th->add_error(ss.str(),tok);
        th->max_recursive_depth *= 2;
        return metil_interactive(th,tok_interact,tok,sp,">>> ","... ");
    }
    
    //
    Variable *def_v = sp - ( 1 + nb_unnamed + nb_named );
    
    // Functionnal in arguments
    for(unsigned na=1;na<=nb_unnamed + nb_named;++na) {
        if ( def_v[na].type == global_data.Functionnal ) {
            // if property, we delay the test
            if ( def_v->type == global_data.Property )
                continue;
            // if def name of arguments contain 'function'
            if ( na-1 >= nb_unnamed ) { // named
                std::string name( named[ na-1-nb_unnamed ] );
                if ( name.size()>=8 and std::string(name.begin(),name.begin()+8) == "function" )
                    continue;
            }
            else { // unnamed
                if ( def_v->type == global_data.Def ) {
                    Definition *def = reinterpret_cast<Definition *>( def_v->data );
                    if ( na-1+def->nb_uargs < def->def_data->nb_args ) {
                        std::string name = def->def_data->args[ na-1+def->nb_uargs ];
                        if ( name.size()>=8 and std::string(name.begin(),name.begin()+8) == "function" )
                            continue;
                    }
                }
            }
            
            //
            if ( expect_res == false ) {
                th->add_error( "composition of something with an argument of type Functionnal is useless if you don't plan to use it.", tok );
                sp = def_v;
                return next_tok;
            }
            if ( not return_var ) return_var = def_v - 1;
            // there's a return_var
            return_var->init( global_data.Functionnal, 0 );
            Functionnal *f = reinterpret_cast<Functionnal *>( return_var->data );
            f->init();
            make_copy( &f->var, def_v );
            f->nb_named = nb_named;
            for(unsigned i=0;i<nb_unnamed;++i) make_copy( f->variables.new_elem(), def_v + i + 1 );
            for(unsigned i=0;i<nb_named  ;++i) { Variable *v = f->variables.new_elem(); make_copy( v, def_v + nb_unnamed + i + 1 ); v->name = named[i]; }
            sp = def_v;
            return next_tok;
        }
    }
    
    // Def
    if ( def_v->type == global_data.Def ) {
        // init def_trial
        DefTrial *def_trial = (DefTrial *)th->info_stack.get_room_for( sizeof( DefTrial ) );
        
        def_trial->def_v = def_v;
        def_trial->def = reinterpret_cast<Definition *>( def_v->data );
        //std::cout << def_trial->def->def_data->name << std::endl;
        def_trial->nb_uargs = nb_unnamed;
        def_trial->nb_nargs = nb_named;
        def_trial->names = named;
        def_trial->min_pertinence_unconditionnal = FLOAT64_MIN_VAL;
        def_trial->first_callable_to_try = NULL;
        def_trial->first_ok_callable = NULL;
        def_trial->current_callable = NULL;
        if ( expect_res )
            def_trial->return_var = def_v - 1;
        else
            def_trial->return_var = NULL;
        def_trial->supplementary_attributes = supplementary_attributes;
        if ( return_var ) // overrides
            def_trial->return_var = return_var;
        def_trial->old_next_tok = next_tok;
        def_trial->old_sourcefile = th->current_sourcefile;
        def_trial->old_tok = tok;
        def_trial->old_last_named_variable_in_current_scope = th->last_named_variable_in_current_scope;
        def_trial->old_current_self = th->current_self;
        def_trial->prev_def_trial = th->current_def_trial;
        def_trial->partial_instanciation = partial_instanciation;
        def_trial->nb_call_to_do_until_old_next_tok = nb_calls;
    
        th->current_def_trial = def_trial;
    
        // look up for callables
        fill_callable_lists( def_trial, th, tok, sp );
        
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
    // Lambda
    else if ( def_v->type == global_data.Lambda ) {
        Lambda *l = reinterpret_cast<Lambda *>( def_v->data );
        // checks
        if ( nb_named or nb_unnamed != l->names.size() ) {
            if ( nb_named )
                th->add_error( "TODO: named arguments in lambda functions.", tok );
            else {
                std::ostringstream ss;
                ss << "uncorrect number of arguments (=" << nb_unnamed+nb_named << ") for this lambda function (" << l->names.size() << " argument(s) recquired).";
                th->add_error( ss.str(), tok );
            }
            if ( return_var )
                assign_error_var( return_var );
            else if ( expect_res )
                assign_error_var( def_v - 1 );
            return next_tok;
        }
        
        // init def_trial
        DefTrial *def_trial = (DefTrial *)th->info_stack.get_room_for( sizeof( DefTrial ) );
        
        def_trial->def_v = def_v;
        def_trial->def = NULL;
        //std::cout << def_trial->def->def_data->name << std::endl;
        def_trial->nb_uargs = nb_unnamed;
        def_trial->nb_nargs = nb_named;
        def_trial->names = named;
        def_trial->min_pertinence_unconditionnal = FLOAT64_MIN_VAL;
        def_trial->first_callable_to_try = NULL;
        def_trial->first_ok_callable = NULL;
        def_trial->current_callable = NULL;
        if ( expect_res )
            def_trial->return_var = def_v - 1;
        else
            def_trial->return_var = NULL;
        def_trial->supplementary_attributes = supplementary_attributes;
        if ( return_var ) // overrides
            def_trial->return_var = return_var;
        def_trial->old_next_tok = next_tok;
        def_trial->old_sourcefile = th->current_sourcefile;
        def_trial->old_tok = tok;
        def_trial->old_last_named_variable_in_current_scope = th->last_named_variable_in_current_scope;
        def_trial->old_current_self = th->current_self;
        def_trial->prev_def_trial = th->current_def_trial;
        def_trial->partial_instanciation = partial_instanciation;
        def_trial->nb_call_to_do_until_old_next_tok = nb_calls;
    
        th->current_def_trial = def_trial;
    
        // scope
        //         th->last_named_variable_in_current_scope = ;
        for(unsigned i=0;i<nb_unnamed;++i) {
            Variable *nv = assign_ref_on( sp++, def_v + i + 1 );
            nv->prev_named = th->last_named_variable_in_current_scope;
            nv->name = l->names[i];
            th->last_named_variable_in_current_scope = nv;
        }
   
        // here we can assume that there's no trial block to execute
        return l->tok;
    }
    // block( value )
    else if ( def_v->type == global_data.BlockOfTok ) {
        // push Scope
        DefTrial *os = (DefTrial *)th->info_stack.get_room_for( sizeof(DefTrial) );
        os->def_v = def_v;
        os->old_tok = tok;
        os->old_next_tok = next_tok; /// we will use old_next_tok only if nb_call_to_do_until_old_next_tok == 1
        os->old_sourcefile = th->current_sourcefile;
        os->old_last_named_variable_in_current_scope = th->last_named_variable_in_current_scope; ///
        os->old_current_self = th->current_self;
        os->first_callable_to_try = NULL;
        os->first_ok_callable = NULL;
        os->current_callable = NULL;
        os->prev_def_trial = th->current_def_trial;
        th->current_def_trial = os;
        
        // 
        Block *b = reinterpret_cast<Block *>( def_v->data );
        os->def_trial_of_creation_if_for = b->def_trial_during_creation;
        Variable *v = def_v + 1;
        if ( b->want_tuple ) {
            if ( v->type != global_data.VarArgs ) {
                th->add_error( "TODO : extraction of parameters of for loops from type != Varargs.", tok );
                th->info_stack.pop_until( (char *)os );
                return next_tok;
            }
            VarArgs *va = reinterpret_cast<VarArgs *>( v->data );
            if ( b->names.size() > va->nb_uargs() ) {
                std::ostringstream ss; ss << "not enough values in list (we want " << b->names.size() << " but we have " << va->nb_uargs() << ").";
                th->add_error( ss.str(), tok );
                th->info_stack.pop_until( (char *)os );
                return next_tok;
            }
            Variable *ov = b->last_named;
            th->check_for_room_in_variable_stack( sp, b->names.size(), v );
            for(unsigned i=0;i<b->names.size();++i) {
                Variable *nv = assign_ref_on( sp++, va->uarg(i) );
                nv->name = b->names[i];
                nv->prev_named = ov;
                ov = nv;
            }
            th->last_named_variable_in_current_scope = ov;
        }
        else {
            assert( b->names.size() == 1 );
            v->name = b->names[0];
            v->prev_named = b->last_named;
            th->last_named_variable_in_current_scope = v;
        }
        th->set_current_sourcefile( b->sourcefile );
        th->current_self = b->self;
        return b->tok;
    }
    else if ( def_v->type == global_data.Property ) {
        if ( nb_calls != 1 )
            th->add_error( "TODO nb_calls != 1 in def_v->type == global_data.Property.", tok );
        th->check_for_room_in_variable_stack( sp, 1, def_v );
        for(int i=nb_unnamed+nb_named;i>=0;--i)
            make_copy( def_v+i+1, def_v+i );
        if ( partial_instanciation ) {
            if ( expect_res )
                assign_ref_on( def_v, th->main_scope->find_var(STRING___property_call_partial_inst_with_return___NUM) );
            else
                assign_ref_on( def_v, th->main_scope->find_var(STRING___property_call_partial_inst___NUM) );        
        } else {
            if ( expect_res )
                assign_ref_on( def_v, th->main_scope->find_var(STRING___property_call_with_return___NUM) );
            else
                assign_ref_on( def_v, th->main_scope->find_var(STRING___property_call___NUM) );        
        }
        return exec_tok_apply(n__compile_mode,th,tok,nb_unnamed+1,nb_named,named,expect_res,next_tok,++sp,nb_calls,false,return_var,supplementary_attributes);
    }
    else if ( def_v->type == global_data.Functionnal ) {
        // offset data
        th->check_for_room_in_variable_stack( sp, 1, def_v );
        for(int i=nb_unnamed+nb_named;i>=0;--i)
            make_copy( def_v+i+1, def_v+i );
        // Ref on Functionnal
        def_v[1].init( global_data.InternalVariable, 0 );
        Variable *ref_functionnal = reinterpret_cast<Variable *>( def_v[1].data );
        make_copy( ref_functionnal, def_v );
        // re-call with apply_functionnal...
        make_copy( def_v, th->main_scope->find_var( expect_res ? STRING_apply_functionnal_with_return_NUM : STRING_apply_functionnal_NUM ) ); def_v->inc_ref();
        return exec_tok_apply(n__compile_mode,th,tok,nb_unnamed+1,nb_named,named,expect_res,next_tok,++sp,nb_calls,partial_instanciation,return_var,supplementary_attributes);
    }
    // else -> def_v.apply(...)
    if ( not def_v->replace_by_method_named( th, STRING_apply_NUM ) ) {
        if ( not th->replace_var_by_ext_method_named( def_v, STRING_apply_NUM ) ) {
            th->add_error( "no method 'apply' was found for object of type '"+std::string(def_v->type->name)+"'.", tok );
            if ( expect_res ) {
                if ( return_var ) assign_error_var( return_var );
                else assign_error_var( def_v-1 );
            }
            sp = def_v;
            return next_tok;
        }
    }
    return exec_tok_apply(n__compile_mode,th,tok,nb_unnamed,nb_named, named,expect_res,next_tok,sp,nb_calls,partial_instanciation,return_var,supplementary_attributes);
}

template<int compile_mode> const void *exec_tok_apply(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Apply *tok,Variable * &sp) {
    return exec_tok_apply( n__compile_mode, th, tok, tok->nb_unnamed, tok->nb_named, tok->named, tok->has_attr_expect_res(), tok->next(), sp );
}

template<int compile_mode> const void *exec_tok_partial_instanciation(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Partial_instanciation *tok,Variable * &sp) {
    return exec_tok_apply( n__compile_mode, th, tok, 0, 0, NULL, true, tok->next(), sp, 1, /*partial_instanciation*/true );
}

template<int compile_mode> const void *exec_tok_select( const N<compile_mode> &n__compile_mode, Thread *th, const void *tok,
        unsigned nb_unnamed, unsigned nb_named, const Nstring *named, bool expect_res, const void *next_tok, Variable * &sp, Variable *return_var ) {
    unsigned nb_args = nb_unnamed + nb_named;
    Variable *def_v = sp - ( 1 + nb_args );
    
    // optimization
    if ( def_v->type == global_data.Def and return_var ) {
        Definition *res, *def = reinterpret_cast<Definition *>( def_v->data );
        if ( *def_v->cpt_use == 0 ) { // means that we must re-use data of def_v (elsewhere we won't have the possibility to delete them)
            res = def;
            
            Variable *args = (Variable *)malloc( sizeof(Variable) * ( def->nb_uargs + def->nb_nargs + nb_args ) );
            for(unsigned i=0;i<def->nb_uargs;++i)
                make_copy( args + i, def->args + i );
            for(unsigned i=0;i<def->nb_nargs;++i)
                make_copy_with_name( args + def->nb_uargs + nb_unnamed + i, def->args + def->nb_uargs + i );
            //
            free( res->args );
            res->args = args;
            
            make_copy( return_var, def_v );
        }
        else { // means that def_v do not need to be deleted. We have to create a new Definition object
            def_v->dec_ref();
            //             Variable *return_var = def_v - 1;
            return_var->init( global_data.Def, 0 );
            res = reinterpret_cast<Definition *>( return_var->data );
            res->init( def, nb_unnamed, nb_named );
        }
        // copy supplementary args and names
        for(unsigned i=0;i<nb_unnamed;++i)
            make_copy( res->args + res->nb_uargs++, def_v + 1 + i );
        for(unsigned i=0;i<nb_named;++i) {
            res->args[ res->nb_uargs + res->nb_nargs ].name = named[ i ];
            make_copy( res->args + res->nb_uargs + res->nb_nargs++, def_v + 1 + nb_unnamed + i );
        }
        
        sp = def_v;
        return next_tok;
    }
    
    // else -> call toto.select(...)
    if ( not def_v->replace_by_method_named( th, STRING_select_NUM ) )
        if ( not th->replace_var_by_ext_method_named( def_v, STRING_select_NUM ) )
            std::cerr << "select not found !!" << std::endl;
    return exec_tok_apply( n__compile_mode, th, tok, nb_unnamed, nb_named, named, return_var, next_tok, sp, /*nb_calls*/1, /*pi*/false, return_var );
}

template<int compile_mode> const void *exec_tok_select(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Select *tok,Variable * &sp) {
    unsigned nb_args = tok->nb_named + tok->nb_unnamed;
    return exec_tok_select( n__compile_mode, th, tok, tok->nb_unnamed, tok->nb_named, tok->named, tok->has_attr_expect_res(), tok->next(), sp, sp - nb_args - 2 );
//     unsigned nb_args = tok->nb_unnamed + tok->nb_named;
//     Variable *def_v = sp - ( 1 + nb_args );
//     
//     // optimization
//     if ( def_v->type == global_data.Def and tok->has_attr_expect_res() ) {
//         Definition *res, *def = reinterpret_cast<Definition *>( def_v->data );
//         if ( *def_v->cpt_use == 0 ) { // means that we must re-use data of def_v (elsewhere we won't have the possibility to delete them)
//             res = def;
//             
//             Variable *args = (Variable *)malloc( sizeof(Variable) * ( def->nb_uargs + def->nb_nargs + nb_args ) );
//             for(unsigned i=0;i<def->nb_uargs;++i)
//                 make_copy( args + i, def->args + i );
//             for(unsigned i=0;i<def->nb_nargs;++i)
//                 make_copy_with_name( args + def->nb_uargs + tok->nb_unnamed + i, def->args + def->nb_uargs + i );
//             //
//             free( res->args );
//             res->args = args;
//             
//             make_copy( def_v - 1, def_v );
//         }
//         else { // means that def_v do not need to be deleted. We have to create a new Definition object
//             def_v->dec_ref();
//             Variable *return_var = def_v - 1;
//             return_var->init( global_data.Def, 0 );
//             res = reinterpret_cast<Definition *>( return_var->data );
//             res->init( def, tok->nb_unnamed, tok->nb_named );
//         }
//         // copy supplementary args and names
//         for(unsigned i=0;i<tok->nb_unnamed;++i)
//             make_copy( res->args + res->nb_uargs++, def_v + 1 + i );
//         for(unsigned i=0;i<tok->nb_named;++i) {
//             res->args[ res->nb_uargs + res->nb_nargs ].name = tok->named[ i ];
//             make_copy( res->args + res->nb_uargs + res->nb_nargs++, def_v + 1 + tok->nb_unnamed + i );
//         }
//         
//         sp = def_v;
//         return tok->next();
//     }
//     
//     // else -> call toto.select(...)
//     if ( not def_v->replace_by_method_named( th, STRING_select_NUM ) )
//         if ( not th->replace_var_by_ext_method_named( def_v, STRING_select_NUM ) )
//             std::cerr << "select not found !!" << std::endl;
//     return exec_tok_apply( n__compile_mode, th, tok, tok->nb_unnamed, tok->nb_named, tok->named, tok->has_attr_expect_res(), tok->next(), sp );
}

template<int compile_mode> const void *exec_tok_change_behavior(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Change_behavior *tok,Variable * &sp) {
    std::cout << "Todo " << __FILE__ << " " << __LINE__ << std::endl;
    return exec_tok_apply( n__compile_mode, th, tok, tok->nb_unnamed, tok->nb_named, tok->named, tok->has_attr_expect_res(), tok->next(), sp );
}

template<int compile_mode> const void *exec_tok_end_def_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_def_block *tok,Variable * &sp) {
    DefTrial *def_trial = th->current_def_trial;
    
    // return issues
    if ( def_trial->return_var and def_trial->return_var->type == global_data.Void ) {
        th->add_error( std::string(def_trial->current_callable->self?"Method":"Function")+" does not return anything.", tok );
        assign_error_var( def_trial->return_var );
    }
    
    //
    if ( compile_mode ) {
        std::cerr << "TODO : several ok functions" << std::endl;
    }
    
    // delete variables
    const void *tok_del;
    while ( sp > def_trial->def_v )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    while ( th->variables_to_del_at_next_end_block.size() ) {
        //if ( *th->variables_to_del_at_next_end_block.back().cpt_use == 0 and th->variables_to_del_at_next_end_block.back().type == global_data.Def )
        //    std::cout << "-> " << reinterpret_cast<Definition *>(th->variables_to_del_at_next_end_block.back().data)->def_data->name << std::endl;
        if ( ( tok_del = destroy_var( th, tok, tok, sp, &th->variables_to_del_at_next_end_block.pop_back() ) ) != NULL )
            return tok_del;
    }
    
    // def_trial made for destroy ?
    if ( def_trial->def == NULL and def_trial->nb_call_to_do_until_old_next_tok == 1 and def_trial->current_callable ) {
        Variable *var_to_destroy = def_trial->current_callable->self;
        for(unsigned i=0;i<var_to_destroy->type->nb_sub_objects + 1;++i)
            var_to_destroy->transient_data[i].destroy( th );
        dec_ref( th, var_to_destroy->parent_type );
        free( var_to_destroy->parent_data );
    }
    
    // else, pop pc
    if ( def_trial->return_var ) def_trial->return_var->attributes |= def_trial->supplementary_attributes;
    th->set_current_sourcefile( def_trial->old_sourcefile );
    tok_del = def_trial->old_next_tok;
    if ( not tok_del )
        std::cout << "fin du thread" << std::endl;
    // pop scope
    th->last_named_variable_in_current_scope = def_trial->old_last_named_variable_in_current_scope;
    th->current_self = def_trial->old_current_self;
    
    // pop def_trial
    if ( def_trial->nb_call_to_do_until_old_next_tok == 1 or def_trial->def ) {
        th->info_stack.pop_until( (char *)def_trial );
        th->current_def_trial = def_trial->prev_def_trial; // we assume that memory for def_trial has not been freed
    }
    
    // re-call ?
    if ( def_trial->nb_call_to_do_until_old_next_tok > 1 ) {
        if ( def_trial->def ) // not a def_trial made for destroy
            return exec_tok_apply( n__compile_mode, th, def_trial->old_tok, 
                                   def_trial->nb_uargs, def_trial->nb_nargs, def_trial->names,
                                   def_trial->return_var, def_trial->old_next_tok,
                                   sp, def_trial->nb_call_to_do_until_old_next_tok-1 );
        //
        --def_trial->nb_call_to_do_until_old_next_tok;
        return play_with_callables( th, def_trial, def_trial->old_tok, sp );
    }
    
    return tok_del;
}

template<int compile_mode> const void *exec_tok_end_class_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_class_block *tok,Variable * &sp) {
    Callable *c = th->current_def_trial->current_callable;
    DefinitionData *dd = c->def_data;
    // copy of arguments (if not already done)
//     unsigned nargs = dd->nb_args + dd->has_varargs();
//     if ( nargs and c->arguments[0] ) {
//         // properties ?
//         for(unsigned i=0;i<nargs;++i) {
//             if ( c->arguments[i]->type == global_data.Property ) {
//                 Property *p = reinterpret_cast<Property *>( c->arguments[i]->data );
//                 if ( p->get_().v ) {
//                     th->check_for_room_in_variable_stack( sp, 2 );
//                     //
//                     assign_void_var( sp );
//                     if ( th->last_named_variable_in_current_scope == c->arguments[i] ) // replace c->arguments[i] by sp in scope
//                         th->last_named_variable_in_current_scope = sp;
//                     else {
//                         Variable *v=th->last_named_variable_in_current_scope;
//                         while ( v->prev_named != c->arguments[i] ) v = v->prev_named;
//                         v->prev_named = sp;
//                     }
//                     sp->prev_named = c->arguments[i]->prev_named;
//                     sp->name = c->arguments[i]->name;
//                     sp->attributes = c->arguments[i]->attributes;
//                     c->arguments[i] = sp++;
//                     //
//                     if ( p->self.type ) {
//                         assign_ref_on( sp, &p->self );
//                         if ( not sp->replace_by_method_named( th, p->get_() ) )
//                             std::cerr << "Weird " << __FILE__ << " " << __LINE__ << std::endl;
//                         return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok, ++sp );
//                     }
//                     else {
//                         Variable *d = th->find_var( p->get_() );
//                         if ( not d ) {
//                             th->add_error( "variable is a property with 'get_...' function ('"+std::string(p->get_())+"'), but the function is impossible to find.", tok );
//                             assign_error_var( c->arguments[i] );
//                         }
//                         else {
//                             assign_ref_on( sp++, d );
//                             return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok, sp );
//                         }
//                     }
//                 }
//                 else {
//                     th->add_error( "variable is a property without 'get_...' function which is mandatory for class arguments.", tok );
//                     assign_error_var( c->arguments[i] );
//                 }
//             }
//         }
//         
//         // rm ref on arguments from scope
//         Variable *last_to_rm = c->arguments[ nargs - 1 ];
//         if ( th->last_named_variable_in_current_scope == last_to_rm )
//             th->last_named_variable_in_current_scope = NULL;
//         else {
//             Variable *v=th->last_named_variable_in_current_scope;
//             while ( v->prev_named != last_to_rm ) v = v->prev_named;
//             v->prev_named = NULL;
//         }
//         
//         // create, register and init partial instanciation of arguments on stack
//         th->check_for_room_in_variable_stack( sp, 3 * nargs );
//         for(unsigned i=0;i<nargs;++i) {
//             // create
//             Variable *new_var = sp + i;
//             new_var->init( c->arguments[i]->type, Variable::STATIC | Variable::CONST );
//             new_var->name = dd->args[i];
//             new_var->prev_named = th->last_named_variable_in_current_scope;
//             th->last_named_variable_in_current_scope = new_var;
//             // push ref
//             Variable *var_to_init = sp + nargs + 2 * i;
//             make_copy( var_to_init, new_var ); new_var->inc_ref();
//             var_to_init->attributes = Variable::STATIC;
//             // ref.init
//             if ( not var_to_init->replace_by_method_named( th, STRING_init_NUM ) ) // a.toto.init
//                 if ( not th->replace_var_by_ext_method_named( var_to_init, STRING_init_NUM ) )
//                     std::cerr << "init not found !!" << std::endl;;
//             // c->variables[i]
//             make_copy( var_to_init + 1, c->arguments[i] ); c->arguments[i]->inc_ref();
//         }
//         
//         sp += 3 * nargs;
//         // init them
//         c->arguments[0] = NULL;
//         return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/false, tok, sp, nargs );
//     }
    // construction
    Type *type = th->types->new_elem();
    *dd->type_cache.new_elem() = type;
    type->init( dd, th->last_named_variable_in_current_scope );
    
    // instantiation
    return instantiate( th->current_def_trial, th, tok, sp, type );
}

template<int compile_mode> const void *exec_tok_definition(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Definition *tok,Variable * &sp) {
    // new DefinitionData
    DefinitionData *dd = DefinitionData::make_new( tok->nb_args, tok->attributes );
    dd->size_needed_in_stack = tok->size_needed_in_stack;
    dd->offset_in_sar_file = tok->offset_in_sar_file;
    dd->doc = reinterpret_cast<const char *>(tok) + tok->offset_to_doc;
    dd->try_block = ( tok->offset_to_try_block ? 
            reinterpret_cast<const void *>( reinterpret_cast<const char *>(tok) + tok->offset_to_try_block ) : NULL );
    dd->block = reinterpret_cast<const void *>( reinterpret_cast<const char *>(tok) + tok->offset_to_block );
    dd->sourcefile = th->current_sourcefile;
    dd->name = tok->name;
    dd->property_name = tok->property_name;
    if ( tok->name.v < 0 )
        std::cout << "TODO : calculated name" << std::endl;
    dd->pertinence = tok->pertinence;
    dd->min_nb_args = tok->min_nb_args;
    dd->nb_args = tok->nb_args;
    dd->next_with_same_name = NULL;
    dd->cpp_thread_try_func = NULL;
    for(unsigned i=0;i<tok->nb_args;++i) {
        if ( tok->args[i].v < 0 ) {
            th->add_error( "TODO : calculated args.", tok );
        } else
            dd->args[i] = tok->args[i];
    }
    if ( dd->has_varargs() )
        dd->args[dd->nb_args] = STRING_varargs_NUM;
    
    
    // register 
    const void *next_tok = reinterpret_cast<const char *>(tok) + tok->offset_to_next_inst;
    Variable *def_v = th->find_var_in_current_scope( dd->name );
    // can we use a ref on def_v with dd appended ? ( def_v->can_be_redefined() means a new definition
    //  because the old def_datas come from an ancestor )
    if ( def_v and def_v->type == global_data.Def and def_v->can_be_redefined()==false and def_v->is_primary_def() ) { 
        if ( tok->has_attr_virtual() )
            def_v->attributes |= Variable::VIRTUAL;
        assign_ref_on( sp++, def_v );
        Definition *def = reinterpret_cast<Definition *>( def_v->data );
        //
//         if ( dd->pertinence >= def->def_data->pertinence or def->def_data->cpt_use ) {
            dd->next_with_same_name = def->def_data;
            def->def_data = dd;
//         } else {
//            DefinitionData *odd = def->def_data;
//            while ( odd->next_with_same_name and odd->next_with_same_name->cpt_use==0 and dd->pertinence < odd->next_with_same_name->pertinence ) odd = odd->next_with_same_name;
//            dd->next_with_same_name = odd->next_with_same_name;
//            odd->next_with_same_name = dd;
//         }
    } else { // -> creation of a new Definition
        Variable *n_def_v = sp++; n_def_v->init( global_data.Def, Variable::CONST | Variable::IS_PRIMARY_DEF | ( Variable::VIRTUAL * tok->has_attr_virtual() ) );
        Definition *n_def = reinterpret_cast<Definition *>( n_def_v->data ); n_def->init( dd );
        th->register_var_in_current_scope( tok, n_def_v, dd->name, false );
        //
        if ( def_v ) {
            th->rm_var_in_current_scope( def_v );
            if ( def_v->type != global_data.Def or def_v->is_primary_def()==false )
                th->add_error( "There's already a variable named '"+std::string(dd->name)+"' in current scope, but it is neither a primary method, nor a primary function, nor a primary class.", tok );
            else { // def_v comes from ancestor
                n_def->def_data->next_with_same_name = reinterpret_cast<Definition *>(def_v->data)->def_data;
                reinterpret_cast<Definition *>(def_v->data)->def_data->inc_this_and_children_cpt_use();
            }
        }
    }
    
    // property
    if ( tok->property_name.v ) {
        Variable *prop_v = th->find_var_but_not_a_property_name_in_current_scope( tok->property_name );
        if ( prop_v ) {
            assign_ref_on( sp++, prop_v );
            if ( prop_v->type == global_data.Property ) {
                Property *p = reinterpret_cast<Property *>( prop_v->data );
                p->names[ tok->type_property ] = tok->name;
            } else
                th->add_error( "There's already a variable named '"+std::string(tok->property_name)+"' in current scope, but it's not a Property.", tok );
        } else {
            prop_v = sp++;
            prop_v->init( global_data.Property, Variable::CONST );
            Property *p = reinterpret_cast<Property *>( prop_v->data );
            p->init();
            p->name_() = tok->property_name;
            p->names[ tok->type_property ] = tok->name;
            th->register_var_in_current_scope( tok, prop_v, tok->property_name, false );
        }
    }
    
    return next_tok;
}

template<int compile_mode> const void *exec_tok_def_return(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Def_return *tok,Variable * &sp) {
    // rewind stack...
    DefTrial *def_trial_ok_for_return = NULL;
    const void *t = tok->next_in_exception_mode(); // we only want to pop data in info
    while ( true ) {
        int type = *reinterpret_cast<const int *>(t);
        if ( type == Tok::END_DEF_BLOCK or type == Tok::END_CLASS_BLOCK ) {
            if ( def_trial_ok_for_return == NULL or th->current_def_trial == def_trial_ok_for_return ) {
                // manage return value
                if ( th->current_def_trial->return_var )
                    make_copy( th->current_def_trial->return_var, --sp );
                return t;
            }
            // 
            t = th->pop_def_trial();
        }
        else if ( type == Tok::END_TOK_FILE ) {
            const void *test_tok = th->pop_pc();
            if ( test_tok == NULL ) {
                Variable *v = sp - 1;
                if ( v->type == global_data.Int32 )
                    th->return_value = *reinterpret_cast<Int32 *>( v->data );
                else if ( v->type == global_data.Bool )
                    th->return_value = Bool( v->data, 1 << (7-v->get_bit_offset()) );
                else {
                    std::cerr << "TODO : th->return_value of type " << v->type->name << "." << std::endl;
                    th->return_value = 1;
                }
                break;
            }
            t = test_tok;
        }
        else if ( type == Tok::END_TRY_BLOCK )
            t = th->pop_def_trial();
        else if ( type == (int)Tok::END_OF_A_FOR_BLOCK ) {
            if ( not def_trial_ok_for_return )
                def_trial_ok_for_return = th->current_def_trial->def_trial_of_creation_if_for;
            t = th->pop_def_trial();
        }
        else {
            if  ( type == (int)Tok::END_OF_AN_IF_BLOCK or 
                  type == (int)Tok::END_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE or
                  type == (int)Tok::END_CATCH_BLOCK or
                  type == (int)Tok::END_TRY_EXCEPTION_BLOCK or
                  type == (int)Tok::get_END_OF_A_WHILE_BLOCK_type(0) or
                  type == (int)Tok::get_END_OF_A_WHILE_BLOCK_type(1) or
                  type == (int)Tok::END_OF_A_WHILE_BLOCK_FOLLOWED_BY_AN_ELSE ) {
                th->last_named_variable_in_current_scope = th->old_scope->last_named_variable; // should not be necessary
                th->info_stack.pop_contiguous( sizeof(OldScope) );
                th->old_scope = th->old_scope->prev;
            }
            t = Tok::get_next_in_exception_mode( t );
        }

    }
    return t;
}

template<int compile_mode> const void *exec_tok_throw(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Throw *tok,Variable * &sp) {
    const void *t = tok->next_in_exception_mode(); // we only want to pop data in info
    const void *old_t = t;
    Variable *e = --sp;
    while ( true ) {
        int type = *reinterpret_cast<const int *>(t);
        if ( type == Tok::END_DEF_BLOCK or type == Tok::END_CLASS_BLOCK )
            t = th->pop_def_trial();
        else if ( type == Tok::END_TOK_FILE ) {
            const void *test_tok = th->pop_pc();
            if ( not test_tok ) {
                th->add_error( "uncatched exception (type=" + std::string( e->type->name ) + ")", old_t );
                return t;
            }
            t = test_tok;
        }
        else if ( type == Tok::CATCH ) {
            // register var
            const Tok::Catch *c = reinterpret_cast<const Tok::Catch *>( t );
            if ( c->arg.v < 0 )
                th->add_error( "TODO : calculated name catch arg", t );
            th->register_var_in_current_scope( t, e, c->arg, false );
            // new scope
            OldScope *os = (OldScope *)th->info_stack.get_room_for( sizeof(OldScope) );
            os->last_named_variable = th->last_named_variable_in_current_scope;
            os->variable_sp = sp; // that has been modified during pop_pc, pop_def_trial...
            os->prev = th->old_scope;
            th->old_scope = os;
            
            sp = e + 1;
            return c->next();
        }
        else if ( type == Tok::END_TRY_BLOCK )
            t = th->pop_def_trial();
        else if ( type == (int)Tok::END_OF_A_FOR_BLOCK )
            t = th->pop_def_trial();
        else {
            if  ( type == (int)Tok::END_OF_AN_IF_BLOCK or 
                  type == (int)Tok::END_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE or
                  type == (int)Tok::END_CATCH_BLOCK or
                  type == (int)Tok::END_TRY_EXCEPTION_BLOCK or
                  type == (int)Tok::get_END_OF_A_WHILE_BLOCK_type(0) or
                  type == (int)Tok::get_END_OF_A_WHILE_BLOCK_type(1) or
                  type == (int)Tok::END_OF_A_WHILE_BLOCK_FOLLOWED_BY_AN_ELSE ) {
                th->last_named_variable_in_current_scope = th->old_scope->last_named_variable; // should not be necessary
                th->info_stack.pop_contiguous( sizeof(OldScope) );
                th->old_scope = th->old_scope->prev;
            }
            t = Tok::get_next_in_exception_mode( t );
        }
    }
    return t;
}

template<int compile_mode,int want_continue> const void *exec_tok_break_or_continue(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Break_or_continue *tok,Variable * &sp,const N<want_continue> &n__want_continue) {
    // rewind stack...
    DefTrial *def_trial_ok_for_return = NULL;
    bool break_in_a_for_loop = false;
    const void *t = tok->next_in_exception_mode(); // we only want to pop data in info
    while ( true ) {
        int type = *reinterpret_cast<const int *>(t);
        if ( type == Tok::END_DEF_BLOCK or type == Tok::END_CLASS_BLOCK ) {
            // we test prev_def_trial because th->current_def_trial may be the __for__ procedure, not he one where block was created
            if ( break_in_a_for_loop == false or ( th->current_def_trial and th->current_def_trial->prev_def_trial == def_trial_ok_for_return ) )
                return t;
            //
            t = th->pop_def_trial();
        }
        else if ( type == Tok::END_TOK_FILE ) {
            SourceFile *sf = th->current_sourcefile;
            const void *test_tok = th->pop_pc();
            if ( test_tok == NULL ) {
                th->set_current_sourcefile( sf );
                th->add_error( std::string(want_continue?"continue":"break")+" in main function is forbidden : main() should return an integer.", tok );
                return t;
            }
            t = test_tok;
        }
        else if ( type == Tok::END_TRY_BLOCK )
            t = th->pop_def_trial();
        else if ( type == Tok::END_OF_A_FOR_BLOCK ) {
            // continue case
            if ( want_continue ) return t; // data in stack will be deleted by END_OF_A_FOR_BLOCK
            // break case
            def_trial_ok_for_return = th->current_def_trial->def_trial_of_creation_if_for;
            break_in_a_for_loop = true;
            t = th->pop_def_trial();
        }
        else if ( type == Tok::get_END_OF_A_WHILE_BLOCK_type(0) or type == Tok::get_END_OF_A_WHILE_BLOCK_type(1) or type == Tok::END_OF_A_WHILE_BLOCK_FOLLOWED_BY_AN_ELSE ) {
            // continue case
            if ( want_continue ) return t; // do the END_OF_A_WHILE_BLOCK_...
            // break case
            th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
            th->info_stack.pop_contiguous( sizeof(OldScope) );
            th->old_scope = th->old_scope->prev;
            //th->pop_pc(); // mainly for sourcefile and for update of info_stack
            return Tok::get_next_in_exception_mode( t ); // we can exit without modifying the stack because in make_dot_tok_data, stack_prev was invalidated
        }
        else if ( type == Tok::END_OF_AN_IF_BLOCK or
                  type == (int)Tok::END_CATCH_BLOCK or
                  type == (int)Tok::END_TRY_EXCEPTION_BLOCK or
                  type == Tok::END_OF_AN_IF_BLOCK_FOLLOWED_BY_AN_ELSE ) {
            th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
            th->info_stack.pop_contiguous( sizeof(OldScope) );
            th->old_scope = th->old_scope->prev;
            t = Tok::get_next_in_exception_mode( t );
        }
        else
            t = Tok::get_next_in_exception_mode( t );
    }
    return t;
}

template<int compile_mode> const void *exec_tok_try(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Try *tok,Variable * &sp) {
    OldScope *os = (OldScope *)th->info_stack.get_room_for( sizeof(OldScope) );
    os->last_named_variable = th->last_named_variable_in_current_scope;
    os->variable_sp = sp;
    os->prev = th->old_scope;
    th->old_scope = os;
    return tok->next();
}
template<int compile_mode> const void *exec_tok_catch(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Catch *tok,Variable * &sp) {
    return tok->next_in_exception_mode();
}
template<int compile_mode> const void *exec_tok_end_catch_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_catch_block *tok,Variable * &sp) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    // old scope becomes current scope
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    return tok->next();
}
template<int compile_mode> const void *exec_tok_end_try_exception_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_try_exception_block *tok,Variable * &sp) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    // old scope becomes current scope
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    return tok->next();
}

template<int compile_mode,int want_while> const void *exec_tok_if_or_while(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::If_or_while *tok,Variable * &sp,const N<want_while> &n__want_while) {
    Variable *cond_var = sp - 1;
    if ( cond_var->type != global_data.Error ) {
        
        // bool conversion
        int cond = direct_conv_to_bool( cond_var );
        if ( cond < 0 ) {
            // case unknown
            if ( cond_var->type == global_data.Op ) {
                if ( reinterpret_cast<Ex *>( cond_var->data )->known_at_compile_time() ) {
                    cond = not reinterpret_cast<Ex *>( cond_var->data )->value().is_zero();
                } else {
                    th->add_error( "TODO: if or while with symbolic conditions unknown at compile time", tok );
                    make_copy( th->variables_to_del_at_next_end_block.new_elem(), cond_var ); --sp;
                    return tok->next_in_exception_mode();
                }
            }
            else
                return conv_last_var_to( n__compile_mode, th, tok, tok, sp, th->main_scope->find_var(STRING_Bool_NUM) );
        }
        
        // push new scope and execute
        // th->display_stack(tok);
        if ( cond or tok->offset_if_fail ) {
            OldScope *os = (OldScope *)th->info_stack.get_room_for( sizeof(OldScope) );
            os->last_named_variable = th->last_named_variable_in_current_scope;
            os->variable_sp = sp - 1; // including the condition
            os->prev = th->old_scope;
            os->nb_errors = th->error_list->data.size();
            th->old_scope = os;
            if ( cond )
                return tok->next();
            return reinterpret_cast<const char *>(tok) + tok->offset_if_fail;
        }
    }
    
    // if cond == false and no 'else' -> del cond
    const void *tok_del;
    if ( ( tok_del = rm_last_var_in_sp( th, tok, tok->next_in_exception_mode(), sp ) ) != NULL )
        return tok_del;
    return tok->next_in_exception_mode();
}
template<int compile_mode> const void *exec_tok_end_of_an_if_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_of_an_if_block *tok,Variable * &sp) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    // old scope becomes current scope
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    return tok->next();
}
template<int compile_mode> const void *exec_tok_end_of_an_if_block_followed_by_an_else(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_of_an_if_block_followed_by_an_else *tok,Variable * &sp) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    // old scope becomes current scope
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    //
    return reinterpret_cast<const char *>(tok) + tok->offset_to_next_inst;
}
template<int compile_mode,int in_else_block> const void *exec_tok_end_of_a_while_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_of_a_while_block *tok,Variable * &sp,const N<in_else_block> &n__in_else_block) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    //
    unsigned old_nb_errors = th->old_scope->nb_errors;
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    if ( in_else_block )
        return tok->next();
    if ( th->error_list->data.size() != old_nb_errors )
        return tok->next_in_exception_mode();
    return reinterpret_cast<const char *>(tok) + tok->offset_to_condition;
}
template<int compile_mode> const void *exec_tok_end_of_a_while_block_followed_by_an_else(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_of_a_while_block_followed_by_an_else *tok,Variable * &sp) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    //
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    if ( th->error_list->data.size() )
        return tok->next_in_exception_mode();
    return reinterpret_cast<const char *>(tok) + tok->offset_to_condition;

}

template<int compile_mode> const void *exec_tok_end_of_a_for_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_of_a_for_block *tok,Variable * &sp) {
    DefTrial *def_trial = th->current_def_trial;
    // del scope
    const void *tok_del;
    while ( sp > def_trial->def_v )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    
    
    // else, pop pc
    th->set_current_sourcefile( def_trial->old_sourcefile );
    // pop scope
    th->last_named_variable_in_current_scope = def_trial->old_last_named_variable_in_current_scope;
    th->current_self = def_trial->old_current_self;
    // pop def_trial
    th->info_stack.pop_until( (char *)def_trial );
    th->current_def_trial = def_trial->prev_def_trial;

    //
    return def_trial->old_next_tok;
}

template<int compile_mode> const void *exec_tok_get_attr(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Get_attr *tok,Variable * &sp) {
    Nstring name = tok->name;
    Variable *v = sp - 1; // parent var
    // calculated name
    if ( name.v < 0 ) {
        v = sp - 2; // parent var
        Variable *s = sp - 1; // string
        name = Nstring( *reinterpret_cast<const char **>(s->data), *reinterpret_cast<Int32 *>(reinterpret_cast<const char **>(s->data)+1) );
        make_copy( th->variables_to_del_at_next_end_block.new_elem(), --sp );
    }
    // Error
    if ( v->type == global_data.Error )
        return tok->next();
    
    // Functionnal
    if ( v->type == global_data.Functionnal ) {
        Variable tmp; make_copy( &tmp, v );
        v->init( global_data.Functionnal, 0 );
        Functionnal *f = reinterpret_cast<Functionnal *>( v->data );
        f->init();
        make_copy( &f->var, &tmp );
        f->attribute = name;
        return tok->next();
    }
    
    // look in attributes
    TypeVariable *tv = v->type->find_var( v, name );
    
    // if contains_virtual_methods and $name is a property, look for variable in true type
    if ( tv == NULL and v->type->contains_virtual_methods ) {
        for(unsigned i=0;i<th->main_scope->properties.size();++i) {
            if ( th->main_scope->properties[i] == name ) {
                Type *true_type = *reinterpret_cast<Type **>( v->data );
                if ( true_type ) // may be uninitialised
                    tv = true_type->find_var( v, name );
                break;
            }
        }
    }
    
    //
    if ( tv ) {
        if ( tv->v.type == global_data.Def and tv->v.is_primary_def() )
            v->replace_by_method( th, reinterpret_cast<Definition *>( tv->v.data ) );
        else if ( tv->v.type == global_data.Property )
            v->replace_by_property( reinterpret_cast<Property *>( tv->v.data ) );
        else if ( tv->v.is_static() ) {
            tv->v.inc_ref();
            if ( *v->cpt_use == 0 ) { // need to destroy parent var in stack
                // look recursively for destroy methods ( in v->parent and in children )
                if ( v->type_of_only()==false ) {
                    unsigned nb_calls_to_destroy = 0;
                    push_destroy_methods_rec( th, tok, tok->next(), sp, v->parent_type, v->parent_data, 0, v->parent_transient_data, nb_calls_to_destroy, v->parent_type, v->parent_data );
                    if ( nb_calls_to_destroy ) {
                        make_copy( v, &tv->v );
                        return play_with_callables( th, th->current_def_trial, tok, sp );
                    }
                }
                // if no destroy method at all -> just free
                for(unsigned i=0;i<v->parent_type->nb_sub_objects + 1;++i)
                    v->parent_transient_data[i].destroy( th );
                //std::cout << "free " << v->type->name << " " << v->parent_type->name << " ooking for " << name << std::endl;
                dec_ref( th, v->parent_type );
                free( v->parent_data );
            } else // else -> static with no need to destroy parent var in stack
                v->dec_ref();
            make_copy( v, &tv->v );
        } else // non static
            v->replace_by_non_static_attr( tv );
        return tok->next();
    }

        
    // if Property -> replace var by corresponding value
    if ( v->type == global_data.Property ) {
        Property *p = reinterpret_cast<Property *>( v->data );
        Nstring get_ = p->get_();
        if ( get_.v ) {
            th->check_for_room_in_variable_stack( sp, 1, v );
            if ( p->self.type ) {
                Variable *m = sp++;
                if ( *v->cpt_use == 0 ) {
                    assert( v->parent_data == v->data and v->parent_type == v->type );
                    make_copy( m, &p->self );
                    dec_ref( th, global_data.Property );
                    free( p );
                }
                else {
                    v->dec_ref();
                    make_copy( m, &p->self );
                    m->inc_ref();
                }
                if ( not m->replace_by_method_named( th, get_ ) )
                    if ( not th->replace_var_by_ext_method_named( m, get_ ) )
                        std::cerr << "Weird looking for " << get_ << " " << __FILE__ << " " << __LINE__ << std::endl;
                assign_void_var( v );
                return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok, sp );
            }
            else {
                Variable *d = th->find_var( get_ );
                if ( not d ) {
                    th->add_error( "variable is a property with 'get_...' function ('"+std::string(get_)+"'), but the function is impossible to find.", tok );
                    if ( *v->cpt_use == 0 ) { dec_ref( th, global_data.Property ); free( p ); } else v->dec_ref();
                    assign_error_var( v );
                    return tok->next();
                }
                if ( *v->cpt_use == 0 ) {
                    assert( v->parent_data == v->data and v->parent_type == v->type );
                    for(unsigned i=0;i<v->parent_type->nb_sub_objects + 1;++i)
                        v->parent_transient_data[i].destroy( th );
                    dec_ref( th, global_data.Property );
                    free( p );
                } else v->dec_ref();
                assign_void_var( v );
                th->check_for_room_in_variable_stack( sp, 1 );
                assign_ref_on( sp++, d );
                return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok, sp );
            }
        }
        else {
            std::ostringstream ss;
            ss << "variable is a property but there's is no 'get_" << p->name_() << "' function, which is mandatory for reading";
            if ( p->reassign_().v ) ss << ". There's a " << p->reassign_();
            if ( p->apply_on_().v ) ss << ". There's a " << p->apply_on_();
            if ( p->init_().v ) ss << ". There's a " << p->init_();
            if ( p->type_of_().v ) ss << ". There's a " << p->type_of_();
            th->add_error( ss.str(), tok );
        }
    }
    
    // look for external methods ( def toto(self,...) ... )
    if ( th->replace_var_by_ext_method_named( v, name ) )
        return tok->next();
       
    // look for external properties ( def get_toto(self,...) ... )
    if ( th->replace_var_by_ext_property_named( v, name ) )
        return tok->next();
       
    // class property
    if ( th->main_scope->has_prop( name ) ) {
        th->push_var_in_variables_to_del_at_next_end_block_if_to_be_destroyed( v );
        v->init( global_data.Bool, Variable::CONST );
        Bool( v->data, 1 << (7-v->get_bit_offset()) ) = false;
        return tok->next();
    }
    
    // look for get_attr method
    tv = v->type->find_var( v, STRING_get_attr_NUM );
    if ( tv and tv->v.type == global_data.Def and tv->v.is_primary_def() ) {
        th->check_for_room_in_variable_stack( sp, 2, v );
        make_copy( sp, v );
        assign_void_var( v );
        sp->replace_by_method( th, reinterpret_cast<Definition *>( tv->v.data ) );
        std::string s( name );
        assign_string( th, tok, ++sp, &s[0], s.size() );
        return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/true, tok->next(), ++sp );
    }
       
    // not found
    if ( v->type != global_data.Error )
        th->add_error( "Impossible to find any attribute, method or external method named '"+std::string(name)+"' for variable of type '"+std::string(v->type->name)+"'.", tok );
    v->type = global_data.Error; // dirty -> data won't be deleted correctly
    return tok->next();
}
template<int compile_mode> const void *exec_tok_self(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Self *tok,Variable * &sp) {
    if ( th->current_self ) {
        assign_ref_on( sp++, th->current_self );
        return tok->next();
    }
    th->add_error( "There's no 'self' in current context.", tok );
    assign_error_var( sp++ );
    return tok->next();
}

template<int compile_mode> const void *exec_tok_proposition_of_default_value(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Proposition_of_default_value *tok,Variable * &sp) {
    if ( th->current_def_trial->current_callable->arguments[ tok->num_arg ] )
        return reinterpret_cast<const char *>(tok) + tok->offset_to_next_inst;
    return tok->next();
}

template<int compile_mode> const void *exec_tok_assign_to_arg_nb(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Assign_to_arg_nb *tok,Variable * &sp) {
    Callable *c = th->current_def_trial->current_callable;
    c->arguments[ tok->num_arg ] = sp - 1;
    c->arguments[ tok->num_arg ]->prev_named = th->last_named_variable_in_current_scope;
    c->arguments[ tok->num_arg ]->name = c->def_data->args[ tok->num_arg ];
    th->last_named_variable_in_current_scope = c->arguments[ tok->num_arg ];
    return tok->next();
}
template<int compile_mode> const void *exec_tok_pass(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Pass *tok,Variable * &sp) {
    return tok->next();
}

template<int compile_mode> const void *exec_tok_number_int32(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Number_int32 *tok,Variable * &sp) {
    sp->init( global_data.Int32, 0 ); *reinterpret_cast<Int32 *>( sp->data ) = tok->val; ++sp;
    return tok->next();
}

template<int compile_mode> const void *exec_tok_number_unsigned32(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Number_unsigned32 *tok,Variable * &sp) {
    sp->init( global_data.Unsigned32, 0 ); *reinterpret_cast<Unsigned32 *>( sp->data ) = tok->val; ++sp;
    return tok->next();
}

template<int compile_mode> const void *exec_tok_pertinence_in_try_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Pertinence_in_try_block *tok,Variable * &sp) {
    bool conv_ok = direct_conv_to_Float64( sp - 1, th->current_def_trial->current_callable->pertinence );
    if ( not conv_ok )
        return conv_last_var_to( n__compile_mode, th, tok, tok, sp, th->main_scope->find_var(STRING_Float64_NUM) );
    if ( th->current_def_trial->current_callable->pertinence < th->current_def_trial->min_pertinence_unconditionnal )
        return end_of_a_try_block( th, /*callable is OK ?*/false, tok, sp );
    return tok->next();
}
template<int compile_mode> const void *exec_tok_condition_in_try_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Condition_in_try_block *tok,Variable * &sp) {
    int cond = direct_conv_to_bool( sp - 1 );
    if ( cond < 0 )
        return conv_last_var_to( n__compile_mode, th, tok, tok, sp, th->main_scope->find_var(STRING_Bool_NUM) );
    if ( cond )
        return tok->next();
    return end_of_a_try_block( th, /*callable is OK ?*/false, tok, sp );
}

template<int compile_mode> const void *exec_tok_end_try_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_try_block *tok,Variable * &sp) {
    return end_of_a_try_block( th, /*callable is OK ?*/true, tok, sp );
}

template<int compile_mode> const void *exec_tok_append_inheritance_data(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Append_inheritance_data *tok,Variable * &sp) {
    // prev tok comes from partial_instanciation (that is we don"t need to destroy anything)
    sp[-1].attributes |= Variable::STATIC;
    th->register_var_in_current_scope( tok, sp-1, STRING_0_inheritance_NUM, /*need_verification*/false );
    
    Type *type = sp[-1].type;
    //     if ( *sp->cpt_use == 0 ) {
    //         for(unsigned i=0;i<sp->parent_type->nb_sub_objects + 1;++i)
    //             sp->parent_transient_data[i].destroy( th );
    //         dec_ref( th, sp->parent_type );
    //         free( sp->parent_data );
    //     }
    //
    th->check_for_room_in_variable_stack( sp, type->nb_variables );
    for(unsigned num_variable=0;num_variable<type->nb_variables;++num_variable) {
        TypeVariable *t = type->variables + num_variable;
        Variable *v = assign_ref_on( sp++, &t->v );
        if ( t->v.is_primary_def() ) { // make a copy of def and def_data
            v->attributes |= Variable::IS_PRIMARY_DEF;
            v->data = (char *)malloc( sizeof( Definition ) );
            Definition *d = reinterpret_cast<Definition *>( v->data );
            d->init( reinterpret_cast<Definition *>( t->v.data )->def_data->make_copy() );
        }
        v->attributes |= Variable::CAN_BE_REDEFINED;
        th->register_var_in_current_scope( tok, v, t->v.name, /*need_verification*/ t->num_attribute >= 0 );
    }
    
    return tok->next();
}

template<int compile_mode> const void *exec_tok_test_next_type(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Test_next_type *tok,Variable * &sp) {
    Callable *c = th->current_def_trial->current_callable;
    
    --sp;
    bool res = Bool( sp->data, 1 << (7-sp->get_bit_offset()) );
    if ( *sp->cpt_use == 0 ) {
        assert( sp->type == sp->parent_type );
        dec_ref( th, sp->parent_type );
        free( sp->data );
    }
    else
        sp->dec_ref();
    
    // all args are ==
    if ( res == true )
        return instantiate( th->current_def_trial, th, tok, sp, c->def_data->type_cache[c->testing_type_nb] );
    // test next already created type
    if ( ++c->testing_type_nb < c->def_data->type_cache.size() )
        return test_static_args( th, tok, c, sp );
    // -> need a new type
    th->set_current_sourcefile( c->def_data->sourcefile );
    return c->def_data->block;
}

template<int compile_mode,int want_tuple> const void *exec_tok_for(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::For *tok,Variable * &sp,const N<want_tuple> &n__want_tuple) {
    // block (arg of __for__)
    th->check_for_room_in_variable_stack( sp, 1 );
    Variable *block_var = sp++; block_var->init( global_data.BlockOfTok, 0 );
    Block *block = reinterpret_cast<Block *>( block_var->data );
    block->init( th->last_named_variable_in_current_scope, tok->next(), th->current_sourcefile, th->current_self, th->current_def_trial );
    for(unsigned i=0;i<tok->nb_names;++i)
        block->names.push_back( tok->names[i] );
    block->want_tuple = want_tuple;
    
    // exec
    return exec_tok_apply( n__compile_mode, th, tok, /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/false, tok->next_in_exception_mode(), sp, 1 );
}

template<int compile_mode> const void *exec_tok_string(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::String *tok,Variable * &sp) {
    assign_string( th, tok, sp++, tok->characters, tok->nb_characters );
    return tok->next();
}



template<int compile_mode> const void *exec_tok_import(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Import *tok,Variable * &sp) {
    Variable *str_v = sp - 1;
    if ( str_v->type == global_data.Error )
        return tok->next();
        
    if ( str_v->type != global_data.String ) {
        std::cout << "TODO -> import not a string" << std::endl;
    }
    //
    char *p = *reinterpret_cast<char **>( str_v->data );
    unsigned si = *reinterpret_cast<Int32 *>( reinterpret_cast<char **>( str_v->data ) + 1 );
    
    char *tmp_str = (char *)malloc( si + 1 );
    memcpy( tmp_str, p, si );
    tmp_str[si] = 0;
    
    // find complete file name
    char *complete_name = find_path( tmp_str, th->current_sourcefile->directory );
    if ( not complete_name ) {
        th->add_error( "impossible to find file '"+std::string(tmp_str)+"' neither in current directory ('"+
                std::string( th->current_sourcefile->directory )+ "') nor in directories specified in METILPATH, nor in "+std::string(MET_INSTALL_DIR)+".", tok );
        free(tmp_str);
        // del str
        const void *tok_del;
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok->next(), sp ) ) != NULL )
            return tok_del;
        return tok->next();
    }
    free(tmp_str);
    
    // already imported
    if ( th->currently_in_main_scope() ) {
        if ( th->main_scope->has_already_imported( complete_name ) ) {
            free( complete_name );
            return tok->next();
        }
    }
    
    //
    SourceFile *sourcefile = global_data.get_sourcefile( complete_name, th->error_list, &th->interpreter_behavior );
    
    //
    const void *tok_del;
    if ( sourcefile and sourcefile->tok_header() and sourcefile->tok_header()->tok_data() ) {
        if ( th->currently_in_main_scope() )
            th->main_scope->imported_sf.push_back( sourcefile );
            
        sourcefile->add_property_names( th->main_scope );
        
        OldPC *pc = th->old_pc_list.new_elem();
        pc->sourcefile = th->current_sourcefile;
        pc->tok = tok;
        pc->next_tok = tok->next();
        //
        th->set_current_sourcefile( sourcefile );
        const void *new_tok = sourcefile->tok_header()->tok_data();
        // del str
        if ( ( tok_del = rm_last_var_in_sp( th, tok, new_tok, sp ) ) != NULL )
            return tok_del;
        return new_tok;
    }
    // del str
    if ( ( tok_del = rm_last_var_in_sp( th, tok, tok->next(), sp ) ) != NULL )
        return tok_del;
    return tok->next();
}

template<int compile_mode> const void *exec_tok_exec(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Exec *tok,Variable * &sp) {
    Variable *str_v = sp - 1;
    if ( str_v->type == global_data.Error )
        return tok->next();
        
    if ( str_v->type != global_data.String ) {
        std::cout << "TODO -> exec not a string" << std::endl;
    }
    //
    char *p = *reinterpret_cast<char **>( str_v->data );
    unsigned si = *reinterpret_cast<Int32 *>( reinterpret_cast<char **>( str_v->data ) + 1 );
    
    char *line = (char *)malloc( si + 2 );
    line[ 0 ] = 0;
    memcpy( line + 1, p, si );
    line[ si + 1 ] = 0;
    //     std::cout << si << std::endl;
    //     std::cout << "-----------------" << std::endl;
    //     std::cout << line + 1 << std::endl;
    //     std::cout << "-----------------" << std::endl;
    
    //
    SourceFile *sourcefile = global_data.get_sourcefile_from_line( line, th->error_list, &th->interpreter_behavior );
    
    //
    const void *tok_del;
    if ( sourcefile and sourcefile->tok_header() and sourcefile->tok_header()->tok_data() ) {
        if ( th->currently_in_main_scope() )
            th->main_scope->imported_sf.push_back( sourcefile );
            
        sourcefile->add_property_names( th->main_scope );
        
        OldPC *pc = th->old_pc_list.new_elem();
        pc->sourcefile = th->current_sourcefile;
        pc->tok = tok;
        pc->next_tok = tok->next();
        //
        th->set_current_sourcefile( sourcefile );
        const void *new_tok = sourcefile->tok_header()->tok_data();
        // del str
        if ( ( tok_del = rm_last_var_in_sp( th, tok, new_tok, sp ) ) != NULL )
            return tok_del;
        return new_tok;
    }
    // del str
    if ( ( tok_del = rm_last_var_in_sp( th, tok, tok->next(), sp ) ) != NULL )
        return tok_del;
    return tok->next();
}

template<int compile_mode,int want_or> const void *exec_tok_and_or_or(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::And_or_or *tok,Variable * &sp,const N<want_or> &n__want_or) {
    Variable *c = sp - 1;
    if ( c->type != global_data.Bool ) {
        th->add_error( "not a bool after CONVERSION_TO_BOOL ? Weird...", tok );
        sp -= 2; // hum
        // not converted to bool -> call __and__ or __or__
        //         Variable *f = sp - 2;
        //         Variable *wf = th->find_var( want_or ? STRING___or___NUM : STRING___and___NUM );
        //         make_copy( f, wf ); ++wf->cpt_use;
        return tok->next();
    }
    bool cond = Bool( c->data, 1 << (7-c->get_bit_offset()) );
    // rm bool
    if ( *c->cpt_use == 0 ) {
        assert(c->data==c->parent_data and c->type==c->parent_type);
        for(unsigned i=0;i<c->parent_type->nb_sub_objects + 1;++i)
            c->parent_transient_data[i].destroy( th );
        dec_ref( th, c->parent_type );
        free( c->data );
    } else
        c->dec_ref();
    --sp;
    // continue (rm var and read next)
    if ( cond xor want_or ) {
        const void *tok_del = rm_last_var_in_sp( th, tok, tok->next(), sp ); if ( tok_del ) return tok_del;
        return tok->next();
    }
    // stop condition
    return tok->next_in_exception_mode();
}
template<int compile_mode> const void *exec_tok_conversion_to_bool(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Conversion_to_bool *tok,Variable * &sp) {
    Variable *c = sp - 1;
    // cant_be_converted_to_bool
    if ( c->type->cant_be_converted_to_bool ) {
        make_copy( sp++, c ); c->inc_ref();
        return tok->next();
    }
    //
    int cond = direct_conv_to_bool( c );
    if ( cond < 0 ) {
        assign_ref_on( sp++, c );
        return conv_last_var_to( n__compile_mode, th, tok, tok->next(), sp, th->main_scope->find_var(STRING_Bool_NUM) );
    }
    // else
    Variable *v = sp++;
    v->init( global_data.Bool, 0 );
    Bool( v->data, 1 << 7 ) = cond;
    return tok->next();
}

template<int compile_mode,int expect_result> const void *exec_tok_make_varargs(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Make_varargs *tok,Variable * &sp,const N<expect_result> &n__expect_result) {
    Variable *ret = sp - 1 - tok->nb_unnamed_children - tok->nb_names;
    
    ret->init( global_data.VarArgs, 0 );
    VarArgs *va = reinterpret_cast<VarArgs *>( ret->data );
    va->init();
    va->nb_named_args = tok->nb_names;
    for(unsigned i=0;i<tok->nb_unnamed_children + tok->nb_names;++i)
        make_copy( va->variables.new_elem(), ret + 1 + i );
    // names
    for(unsigned i=0;i<tok->nb_names;++i)
        va->uarg(i)->name = tok->names[i];
    
    sp = ret + 1;
    return tok->next();
}

template<int compile_mode> const void *exec_tok_interact(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Interact *tok,Variable * &sp) {
    return metil_interactive( th, tok, tok->next(), sp );
}

template<int compile_mode> const void *exec_tok_get_calculated_attr(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Get_calculated_attr *tok,Variable * &sp) {
    std::cout << "Todo " << __FILE__ << " " << __LINE__ << std::endl;
    return tok->next();
}

template<int compile_mode> const void *exec_tok_get_value_prop(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Get_value_prop *tok,Variable * &sp) {
    if ( sp[-1].type == global_data.Property )
        return exec_replace_prop( n__compile_mode, th, sp, (const void *)tok, tok->next() );
    return tok->next();
}

template<int compile_mode> const void *exec_tok_lambda(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Lambda *tok,Variable * &sp) {
    sp->init( global_data.Lambda, 0 );
    Lambda *l = reinterpret_cast<Lambda *>( sp->data );
    l->init();
    ++sp;
    
    l->sourcefile = th->current_sourcefile;
    l->tok = tok->next();
    for(unsigned i=0;i<tok->nb_arguments;++i)
        l->names.push_back( tok->arguments[i] );
        
    return tok->next_in_exception_mode();
}

template<int compile_mode> const void *exec_tok_label(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Label *tok,Variable * &sp) {
    sp->init( global_data.Label, 0 );
    Label *l = reinterpret_cast<Label *>( sp->data );
    l->init();
    
    l->sourcefile = th->current_sourcefile;
    l->tok = tok->next();
        
    th->register_var_in_current_scope( tok, sp++, tok->name, true );
        
    return tok->next_in_exception_mode();
}

template<int compile_mode> const void *exec_tok_end_of_a_label_block(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_of_a_label_block *tok,Variable * &sp) {
    // del scope
    const void *tok_del;
    while ( sp > th->old_scope->variable_sp )
        if ( ( tok_del = rm_last_var_in_sp( th, tok, tok, sp ) ) != NULL )
            return tok_del;
    // old scope becomes current scope
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    return tok->next();
}

template<int compile_mode> const void *exec_tok_number(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Number *tok,Variable * &sp) {
    Variable *v = sp++; v->init( global_data.Rationnal, 0 );
    Rationnal &r = *reinterpret_cast<Rationnal *>( v->data );
    r.den.init( 1 );
    r.num.n = tok->nb_values - 1;
    r.num.val = tok->values[0];
    if ( r.num.n ) {
        r.num.ext = (int *)malloc( r.num.n * sizeof(int) );
        for(unsigned i=0;i<r.num.n;++i)
            r.num.ext[i] = tok->values[i+1];
    }
    
    // * 10 ** expo
    Rationnal::BI _10( 10 );
    if ( tok->expo > 0 ) { for(int i=0;i< tok->expo;++i) r.num = r.num * _10; }
    else                 { for(int i=0;i<-tok->expo;++i) r.den = r.den * _10; }
    
    r.cannonicalize();
    return tok->next();
}

template<int compile_mode> const void *exec_tok_constify(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Constify *tok,Variable * &sp) {
    Variable *v = sp-1;
    v->attributes |= Variable::CONST;
    return tok->next();
}


template<int compile_mode> const void *exec_tok_exec_in_prev_scope(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::Exec_in_prev_scope *tok,Variable * &sp) {
    // register scope
    OldScope *os = (OldScope *)th->info_stack.get_room_for( sizeof(OldScope) );
    os->last_named_variable = th->last_named_variable_in_current_scope;
    os->prev = th->old_scope;
    th->old_scope = os;
    
    //
    if ( not th->current_def_trial or not th->current_def_trial->prev_def_trial )
        th->add_error( "There's no old scope !", tok );
    th->last_named_variable_in_current_scope = th->current_def_trial->old_last_named_variable_in_current_scope;
    return tok->next();    
}

template<int compile_mode> const void *exec_tok_end_exec_in_prev_scope(const N<compile_mode> &n__compile_mode,Thread *th,const Tok::End_exec_in_prev_scope *tok,Variable * &sp) {
    // old scope becomes current scope
    th->last_named_variable_in_current_scope = th->old_scope->last_named_variable;
    th->old_scope = th->old_scope->prev;
    th->info_stack.pop_contiguous( sizeof(OldScope) );
    return tok->next();    
}

