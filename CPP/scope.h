//
// C++ Interface: scope
//
// Description:
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SCOPE_H
#define SCOPE_H

#include "variable.h"
#include "splittedvec.h"
#include "type.h"
#include "usual_strings.h"
#include "sourcefile.h"
#include <string.h>

/**
    @author Grover <hugo@gronordi>
*/
class Scope {
public:
    static const unsigned hash_size = 256;
    
    Scope() {
        for(unsigned i=0;i<hash_size;++i)
            hash_table[i] = NULL;
        cpt_use = 0;
    }
    
    Variable *find_var_but_not_a_property_name( Nstring ns ) const {
        unsigned n = ns.v % hash_size;
        for(Variable *v=hash_table[n]; v; v=v->prev_named) {
            if ( v->name == ns )
                return v;
        }
        return NULL;
    }
    
    Variable *find_var( Nstring ns ) const {
        Variable *r = find_var_but_not_a_property_name( ns );
        if ( r )
            return r;
        // look in properties
        for(unsigned i=0;i<properties.size();++i)
            if ( properties[i] == ns )
                return find_var( STRING_false_NUM );
        return NULL;
    }
    
    bool has_prop( Nstring ns ) const {
        for(unsigned i=0;i<properties.size();++i)
            if ( properties[i] == ns )
                return true;
        return false;
    }
    
    void register_var( Variable *v, Nstring ns ) {
        unsigned n = ns.v % hash_size;
        Variable *o = hash_table[ n ];
        if ( o ) { // scope is not updated when variables are destroyed...
            while ( o->prev_named ) o = o->prev_named;
            o->prev_named = v;
        }
        else
            hash_table[ n ] = v;
        v->prev_named = NULL;
        v->name = ns;
    }
    
    void rm_var( Variable *v ) {
        std::cout << "TODO" << __FILE__ << " " << __LINE__ << std::endl;
    }
    
    int nb_accessible_variables() const {
        unsigned res = 0;
        for(unsigned i=0;i<hash_size;++i)
            for(Variable *v=hash_table[i]; v; v=v->prev_named)
                ++res;
        return res;
    }

    Nstring accessible_variables_nb(unsigned n) const {
        for(unsigned i=0;i<hash_size;++i)
            for(Variable *v=hash_table[i]; v; v=v->prev_named)
                if ( n-- == 0 )
                    return v->name;
        return 0;
    }
    
    void add_property( Nstring name ) {
        for(unsigned i=0;i<properties.size();++i)
            if ( properties[i] == name )
                return;
        properties.push_back( name );
    }
    
    bool has_already_imported( const char *name ) const {
        for(unsigned i=0;i<imported_sf.size();++i)
            if ( strcmp( imported_sf[i]->provenance, name ) == 0 )
                return true;
        return false;
    }
    
    Variable *hash_table[hash_size]; /// size might be > (if this was created using make_new)
    SplittedVec<Nstring,64> properties;
    SplittedVec<SourceFile *,64> imported_sf;
    unsigned cpt_use;
};

#endif
