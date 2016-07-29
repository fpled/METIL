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
#ifndef DEFINITION_H
#define DEFINITION_H

#include "variable.h"
#include "config.h"

struct DefinitionData;
struct Thread;

/**
    @author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct Definition {
    ///
    void init( DefinitionData *def_data = NULL );
    /// nb_uargs and nb_nargs will be equal to those of def, even of nb_supplementary_uargs or unsigned nb_supplementary_nargs
    void init( Definition *def, unsigned nb_supplementary_uargs=0, unsigned nb_supplementary_nargs=0 );
    ///
    void reassign( Definition *d, Thread *th, Variable * &sp );
        
    DefinitionData *def_data;
    
    unsigned nb_uargs, nb_nargs;
    Variable *args;
    Variable self;
    Float64 max_pertinence;
    bool want_self;
    Type *cached_type;
};

const void *destroy( Definition &d, Thread *th, Variable * &sp );
std::ostream &operator<<( std::ostream &os, const Definition &d );
const void *get_def_from_type( Thread *th, const void *tok, Variable *&sp, Definition &ret, Type *type );
bool are_radically_different( Definition &a, Definition &b );
bool are_similar_for_sure( Definition &a, Definition &b );

#endif
