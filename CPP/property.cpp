//
// C++ Implementation: definition
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "property.h"
#include "thread.h"
#include <usual_strings.h>
#include "scope.h"

void Property::init() {
    for(unsigned i=0;i<6;++i)
        names[i].v = 0;
    self.type = NULL;
}

void Property::init( Property *p ) {
    for(unsigned i=0;i<6;++i)
        names[i].v = p->names[i].v;
    
    if ( p->self.type ) {
        make_copy( &self, &p->self );
        self.inc_ref();
    }
    else
        self.type = NULL;
}

void Property::reassign( Property *d, Thread *th, Variable * &sp ) {
    Property old = *this;
    init( d );
    destroy( old, th, sp );
}

const void *destroy( Property &d, Thread *th, Variable * &sp ) {
    if ( d.self.type )
        th->push_var_if_to_be_destroyed( &d.self, sp );
    
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}
    
// const void *get_property_value( Property &self, Variable *return_var, Thread *th, const void *tok, Variable * &sp ) {
// //     if ( return_var ) {
// //         if ( self.get_().v ) {
// //         }
// //         th->add_error( "Property value is requested but there is not any 'get_"+std::string(self.name_())"' function.", tok );
// //     }
//     std::cout << " get_property_value " << std::endl;
//     th->set_current_sourcefile( NULL;
//     return tok_end_def_block;
// }

const void *property_init_or_reassign( Variable *self_var, Property &prop, Thread *th, const void *tok, Variable **args, Variable * &sp, Nstring name_proc, Nstring type_proc ) {
    unsigned nb_uargs = 0, nb_nargs = 0;
    for(unsigned i=0; args[i] ;++i) {
        nb_uargs += ( args[i]->name.v == 0 );
        nb_nargs += ( args[i]->name.v != 0 );
    }
    Nstring *names = (Nstring *)( args + nb_uargs + nb_nargs + 1 );
    if ( name_proc.v ) {
        th->check_for_room_in_variable_stack( sp, 1 + nb_uargs + nb_nargs );
        if ( prop.self.type ) {
            // def
            Variable *s = assign_ref_on( sp++, &prop.self );
            if ( not s->replace_by_method_named( th, name_proc ) ) // a.name_proc...
                if ( not th->replace_var_by_ext_method_named( s, name_proc ) )
                    std::cerr << "weird !!" << __FILE__ << " " << __LINE__ << std::endl;
            // args
            for(unsigned i=0;i<nb_uargs+nb_nargs;++i)
                assign_ref_on( sp++, args[i] );
            // call
            th->set_current_sourcefile( NULL );
            if ( th->compile_mode ) 
                return exec_tok_apply( N<1>(), th, tok, nb_uargs, nb_nargs, /*names*/names, /*expect_res*/false, tok_end_def_block, sp );
            return exec_tok_apply( N<0>(), th, tok, nb_uargs, nb_nargs, /*names*/names, /*expect_res*/false, tok_end_def_block, sp );
        }
        else {
            th->add_error( "TODO : property_init( ) without self.", tok );
        }
    }
    else if ( prop.get_().v ) {
        th->check_for_room_in_variable_stack( sp, 2 + nb_uargs + nb_nargs );
        // __property_...__ procedure
        if ( type_proc == STRING_init_NUM )
            assign_ref_on( sp++, th->main_scope->find_var( STRING___property_init___NUM ) );
        else
            assign_ref_on( sp++, th->main_scope->find_var( STRING___property_reassign___NUM ) );
        // prop as first arg
        assign_ref_on( sp++, self_var );
        // args
        for(unsigned i=0;i<nb_uargs+nb_nargs;++i)
            assign_ref_on( sp++, args[i] );
        if ( th->compile_mode ) 
            return exec_tok_apply( N<1>(), th, tok, 1+nb_uargs, nb_nargs, /*names*/names, /*expect_res*/false, tok_end_def_block, sp );
        return exec_tok_apply( N<0>(), th, tok, 1+nb_uargs, nb_nargs, /*names*/names, /*expect_res*/false, tok_end_def_block, sp );
    }
    else
        th->add_error( "Error: can't init Property without a '"+std::string(type_proc)+"_"+std::string(prop.name_())+"' or a 'get_"+
            std::string(prop.name_())+"' function.", tok );
    
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

// const void *property_reassign( Property &prop, Thread *th, const void *tok, Variable *arg, Variable * &sp ) {
//     if ( prop.reassign_().v ) {
//         th->check_for_room_in_variable_stack( sp, 2, arg );
//         // prop.self.reassign_...
//         Variable *var_to_reass = sp;
//         if ( prop.self.type )   
//             assign_ref_on( sp++, &prop.self );
//         else if ( th->current_self )
//             assign_ref_on( sp++, th->current_self );
//         else {
//             th->add_error( "can't reassign with a property without object.", tok );
//             th->set_current_sourcefile( NULL;
//             return tok_end_def_block;
//         }
//         if ( not var_to_reass->replace_by_method_named( th, prop.reassign_() ) ) // a.reassign_...
//             if ( not th->replace_var_by_ext_method_named( var_to_reass, prop.reassign_() ) )
//                 std::cerr << "init not found !!" << std::endl;
//         // value
//         assign_ref_on( sp++, arg );
//         // call
//         th->set_current_sourcefile( NULL;
//         if ( th->compile_mode )
//             return exec_tok_apply( N<1>(), th, tok, 
//                                    /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
//                                    /*expect_res*/false, tok_end_def_block,
//                                    sp );
//         return exec_tok_apply( N<0>(), th, tok, 
//                                /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
//                                /*expect_res*/false, tok_end_def_block,
//                                sp );
//     }
//     th->add_error( "TODO : can't reassign with a property without reassign method.", tok );
//     
//     th->set_current_sourcefile( NULL;
//     return tok_end_def_block;
// }

void __get_type_of_function__( Thread *th, const void *tok, Property &prop, Variable *return_var ) {
    if ( prop.self.type ) {
        assign_ref_on( return_var, &prop.self );
        if ( not return_var->replace_by_method_named( th, prop.type_of_() ) ) // a.name_proc...
            if ( not th->replace_var_by_ext_method_named( return_var, prop.type_of_() ) )
                std::cerr << "weird !!" << __FILE__ << " " << __LINE__ << std::endl;
    }
    else {
        Variable *def = th->find_var( prop.type_of_() );
        if ( def )
            assign_ref_on( return_var, def );
        else {
            assign_error_var( return_var );
            th->add_error( "(while resolving property) impossible to find any function named '"+std::string(prop.type_of_())+"'.", tok );
        }
    }
}

// const void *primitive_func_without_property( Thread *th, const void *tok, Variable **args, Variable *return_var, unsigned num_pos, unsigned nb_args ) {
//     Variable *var = args[num_pos];
//     Property &prop = *reinterpret_cast<Property *>( var->data );
//     if ( prop.get_().v == 0 ) {
//         th->
//         th->set_current_sourcefile( NULL;
//         return tok_end_def_block;
//     }
//     
//     if ( prop.self.type ) {
//         assign_ref_on( var, &prop.self );
//         if ( not var->replace_by_method_named( th, prop.type_of_() ) ) // a.name_proc...
//             if ( not th->replace_var_by_ext_method_named( var, prop.type_of_() ) )
//                 std::cerr << "weird !!" << __FILE__ << " " << __LINE__ << std::endl;
//     }
//     else {
//         Variable *def = th->find_var_in_current_scope( prop.type_of_() );
//         if ( def )
//             assign_ref_on( return_var, def );
//         else {
//             assign_error_var( return_var );
//             th->add_error( "(while resolving property) impossible to find any function named '"+std::string(prop.type_of_())+"'.", tok );
//         }
//     }
//     
//     th->set_current_sourcefile( NULL;
//     return tok_end_def_block;
// }

