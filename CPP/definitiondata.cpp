//
// C++ Implementation: definitiondata
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "definitiondata.h"
#include "thread.h"
#include "type.h"
#include <stdlib.h>
#include <string.h>

DefinitionData *DefinitionData::make_new(unsigned nb_args,unsigned attributes) {
    bool has_varargs = attributes & Tok::Definition::VARARGS;
    DefinitionData *res = (DefinitionData *)malloc( sizeof(DefinitionData) + ( nb_args + has_varargs ) * sizeof(Nstring *) );
    res->cpt_use = 0;
    res->next_with_same_name = NULL;
    res->type_cache.init();
    res->nb_args = nb_args;
    res->attributes = attributes;
    res->size_needed_in_stack = 0;
    res->property_name.v = 0;
    res->parent_type = NULL;
    return res;
}

DefinitionData *DefinitionData::make_copy() {
    bool has_varargs = attributes & Tok::Definition::VARARGS;
    int si = sizeof(DefinitionData) + ( nb_args + has_varargs ) * sizeof(Nstring *);
    
    DefinitionData *res = (DefinitionData *)malloc( si );
    memcpy( res, this, si );
    
    res->type_cache.init();
    res->parent_type = NULL; // type of parent class if method
    res->cpt_use = 0;
    if ( next_with_same_name )
        res->next_with_same_name = next_with_same_name->make_copy();

    return res;
}

void DefinitionData::destroy( Thread *th ) {
    for(unsigned i=0;i<type_cache.size();++i)
        dec_ref( th, type_cache[i] );
    type_cache.destroy();
}
    
void DefinitionData::inc_this_and_children_cpt_use() {
    for( DefinitionData *dd=this; dd; dd=dd->next_with_same_name )
        dd->inc_ref();
}
