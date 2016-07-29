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
#include "varargs.h"
#include "thread.h"
#include "globaldata.h"
#include <sstream>

void VarArgs::init() {
    variables.init();
    nb_named_args = 0;
    names = NULL;
    nb_names = 0;
}

void VarArgs::init( VarArgs *v ) {
    variables.init();
    names = NULL; nb_names = 0;
    nb_named_args = v->nb_named_args;
    for(unsigned i=0;i<v->variables.size();++i)
        assign_ref_on( variables.new_elem(), &v->variables[i] );
    for(unsigned i=0;i<nb_named_args;++i)
        narg(i)->name = v->narg(i)->name;
}

void VarArgs::init_from( VarArgs &a, VarArgs &b ) {
    variables.init();
    names = NULL; nb_names = 0;
    nb_named_args = a.nb_named_args + b.nb_named_args;
    for(unsigned i=0;i<a.nb_uargs();++i) assign_ref_on( variables.new_elem(), a.uarg(i) );
    for(unsigned i=0;i<b.nb_uargs();++i) assign_ref_on( variables.new_elem(), b.uarg(i) );
    
    for(unsigned i=0;i<a.nb_nargs();++i) assign_name_and_ref_on( variables.new_elem(), a.narg(i) );
    for(unsigned i=0;i<b.nb_nargs();++i) assign_name_and_ref_on( variables.new_elem(), b.narg(i) );
}


void VarArgs::copy_data_from( VarArgs *v ) {
    init( v );
}
        
void VarArgs::init_names() {
    if ( nb_names != nb_named_args ) {
        if ( names ) free( names );
        names = (Nstring *)malloc( sizeof(Nstring) * nb_named_args );
        nb_names = nb_named_args;
    }
    for(unsigned i=0;i<nb_named_args;++i)
        names[i] = name(i);
}
        
bool VarArgs::contains_data_of_same_type_only(int s) const {
    for(int i=1;i<=s;++i)
        if ( variables[i].type != variables[0].type )
            return false;
    return true;
}

void VarArgs::get_ref_on_uarg(Thread *th,const void *tok,Int32 a,Variable *return_var) {
    if ( return_var ) {
        if ( a < 0 ) {
            std::ostringstream ss; ss << "varargs.get_unnamed_argument_nb(x) with x(=" << a << ") < 0.";
            th->add_error(ss.str(),tok); assign_error_var(return_var);
        }
        else if ( a >= (Int32)nb_uargs() ) {
            std::ostringstream ss; ss << "varargs.get_unnamed_argument_nb(x) with x (=" << a <<") >= varargs.nb_unnamed_arguments (=" << nb_uargs() << ").";
            th->add_error(ss.str(),tok);
            assign_error_var(return_var);
        }
        else
            assign_ref_on( return_var, uarg( a ) );
    }
}

void VarArgs::get_ref_on_narg(Thread *th,const void *tok,Int32 a,Variable *return_var) {
    if ( return_var ) {
        if ( a < 0 ) {
            std::ostringstream ss; ss << "varargs.get_named_argument_nb(x) with x(=" << a << ") < 0.";
            th->add_error(ss.str(),tok); assign_error_var(return_var);
        }
        else if ( a >= (Int32)nb_nargs() ) {
            std::ostringstream ss; ss << "varargs.get_named_argument_nb(x) with x (=" << a <<") >= varargs.nb_named_arguments (=" << nb_nargs() << ").";
            th->add_error(ss.str(),tok);
            assign_error_var(return_var);
        }
        else
            assign_ref_on( return_var, narg( a ) );
    }
}

const void *destroy( VarArgs &d, Thread *th, Variable * &sp ) {
    for(unsigned i=0;i<d.variables.size();++i)
        th->push_var_if_to_be_destroyed( &d.variables[i], sp );
    d.variables.destroy();
    if ( d.names ) free( d.names );
    
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}
    

const void *call_using_varargs( Thread *th, const void *tok, Variable *def, VarArgs &va, Variable * &sp, Variable *return_var, bool want_end_tok_def ) {
    th->check_for_room_in_variable_stack( sp, 1 + va.nb_args(), def, return_var );
    
    assign_ref_on( sp++, def );
    for(unsigned i=0;i<va.nb_args();++i)
        assign_ref_on( sp++, &va.variables[i] );
    
    va.init_names();
    th->set_current_sourcefile( NULL );
    // tok_end_def_block because of function in primitive
    if ( th->compile_mode ) 
        return exec_tok_apply( N<1>(), th, tok, va.nb_uargs(), va.nb_nargs(), /*names*/va.names, /*expect_res*/return_var, want_end_tok_def ? tok_end_def_block : NULL, sp,
           1, false, return_var );
    return exec_tok_apply( N<0>(), th, tok, va.nb_uargs(), va.nb_nargs(), /*names*/va.names, /*expect_res*/return_var, want_end_tok_def ? tok_end_def_block : NULL, sp,
       1, false, return_var );
}

const void *call_select_using_varargs( Thread *th, const void *tok, Variable *def, VarArgs &va, Variable * &sp, Variable *return_var ) {
    if ( not return_var )
        return tok_end_def_block;
    //
    th->check_for_room_in_variable_stack( sp, va.nb_args(), def, return_var );
    
    assign_ref_on( sp++, def );
    for(unsigned i=0;i<va.nb_args();++i)
        assign_ref_on( sp++, &va.variables[i] );
    
    va.init_names();
    th->set_current_sourcefile( NULL );
    // tok_end_def_block because of function in primitive
    if ( th->compile_mode ) 
        return exec_tok_select( N<1>(), th, tok, va.nb_uargs(), va.nb_nargs(), va.names, /*expect_res*/return_var, tok_end_def_block, sp, return_var );
    return exec_tok_select( N<0>(), th, tok, va.nb_uargs(), va.nb_nargs(), va.names, /*expect_res*/return_var, tok_end_def_block, sp, return_var );
}

bool VarArgs::contains_only_Ops() const {
    for(unsigned i=0;i<variables.size();++i)
        if ( variables[i].type != global_data.Op )
            return false;
    return true;
}


