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
#ifndef PROPERTY_H
#define PROPERTY_H

#include "variable.h"
#include "config.h"

struct Thread;

/**
    @author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct Property {
    void init();
    void init( Property *p );
    void reassign( Property *p, Thread *th, Variable * &sp );
    
    Nstring get_()      const { return names[0]; }
    Nstring init_()     const { return names[1]; }
    Nstring reassign_() const { return names[2]; }
    Nstring apply_on_() const { return names[3]; }
    Nstring type_of_()  const { return names[4]; }
    Nstring name_()     const { return names[5]; }
    Nstring &name_()          { return names[5]; }
    
    Nstring names[6];
    Variable self;
};

const void *destroy( Property &d, Thread *th, Variable * &sp );

// const void *get_property_value( Property &self, Variable *return_var, Thread *th, const void *tok, Variable * &sp );

const void *property_init_or_reassign( Variable *self_var, Property &prop, Thread *th, const void *tok, Variable **args, Variable * &sp, Nstring name_proc, Nstring type_proc );
// const void *property_reassign( Property &prop, Thread *th, const void *tok, Variable *arg, Variable * &sp );
void __get_type_of_function__( Thread *th, const void *tok, Property &prop, Variable *return_var );
// const void *primitive_func_without_property( Thread *th, const void *tok, Variable **args, Variable *return_var, unsigned num_pos, unsigned nb_args );

#endif // PROPERTY_H

