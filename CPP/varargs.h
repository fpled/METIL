//
// C++ Interface: definition
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef VARARGS_H
#define VARARGS_H

#include "variable.h"
#include "config.h"
#include "splittedvec.h"
#include "hashstring.h"

struct Thread;

/**
	@author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct VarArgs {
    void init();
    void init( VarArgs *v );
    
    unsigned nb_uargs() const { return variables.size() - nb_named_args; }
    unsigned nb_nargs() const { return nb_named_args; }
    unsigned nb_args() const { return variables.size(); }
    
    Variable *uarg( unsigned i ) { return &variables[i]; }
    const Variable *uarg( unsigned i ) const { return &variables[i]; }
    Variable *narg( unsigned i ) { return &variables[i+nb_uargs()]; }
    const Variable *narg( unsigned i ) const { return &variables[i+nb_uargs()]; }
    Nstring name( unsigned i ) const { return narg(i)->name; }
    void copy_data_from( VarArgs *va );
    
    void push_unnamed( Variable *v ) {
        int s = variables.size();
        variables.new_elem();
        for(int i=s;i>s-(int)nb_named_args;--i)
            make_copy_with_name( &variables[i], &variables[i-1] );
        make_copy( &variables[ s-nb_named_args ], v );
        v->inc_ref();
    }
    
    void push_named( Nstring n, Variable *v ) {
        Variable *nv = variables.new_elem();
        make_copy( nv, v );
        nv->name = n;
        nv->inc_ref();
        ++nb_named_args;
    }
    
    void push_named( const char *s, Int32 si, Variable *v ) {
        push_named( Nstring( s, si, hashstring(s,si), true ), v );
    }
    
    void get_ref_on_uarg(Thread *th,const void *tok,Int32 a,Variable *return_var); // check if a is correct
    void get_ref_on_narg(Thread *th,const void *tok,Int32 a,Variable *return_var); // check if a is correct    
    
    void init_from( VarArgs &a, VarArgs &b );
    
    operator bool() const { return variables.size(); }
    void init_names();
    
    bool contains_data_of_same_type_only(int s) const;
    bool contains_only_Ops() const;
    
    SplittedVec<Variable,8,8> variables;
    Nstring *names; // initialised ony if recquired
    unsigned nb_names;
    
    unsigned nb_named_args;
};

const void *destroy( VarArgs &d, Thread *th, Variable * &sp );
const void *call_using_varargs( Thread *th, const void *tok, Variable *def, VarArgs &va, Variable * &sp, Variable *return_var, bool want_end_tok_def = true );
const void *call_select_using_varargs( Thread *th, const void *tok, Variable *def, VarArgs &va, Variable * &sp, Variable *return_var );

#endif // VARARGS_H

