//
// C++ Interface: tokheader
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TOKHEADER_H
#define TOKHEADER_H

#include "tok.h"
#include "nstring.h"
#include <iostream>

/**
 */
struct TokHeader {
    char md5_ctrl_sum[ 32 ];
    unsigned nb_pos_types_to_rellocate;
    unsigned nb_pos_strings_to_rellocate;
    unsigned nb_strings;
    unsigned offset_of_property_names;
    unsigned nb_property_names;
    unsigned offset_of_first_tok_in_block;
    
    unsigned data[4];
    // followed by offsets (unsigned) of type to rellocate
    //  and offsets (unsigned) of string number to rellocate
    //  and StringInfo{ num, size, hash_val % 1024, char [] } * nb_strings

    
    struct StringInfo {
        int num;
        unsigned size;
        unsigned hash_val;
        char str[4];
    };
    struct StringInfoIterator {
        void operator++() {
            p += 3 + ( reinterpret_cast<const StringInfo *>(p)->size + sizeof(unsigned) - 1 ) / sizeof(unsigned);
        }
        const StringInfo *operator->() const { return (const StringInfo *)p; }
        const unsigned *p;
    };

    void rellocate(); /// set string numbers in conformance tothose in rc.string_set

    StringInfoIterator strings_begin() const {
        StringInfoIterator i;
        i.p = data + nb_pos_types_to_rellocate + nb_pos_strings_to_rellocate;
        return i;
    }
    
    int *begin_property_names() {
        return reinterpret_cast<int *>( (char *)this + offset_of_property_names );
    }
    
    const void *tok_data() const {
        if ( offset_of_first_tok_in_block )
            return reinterpret_cast<const void *>( (char *)this + offset_of_first_tok_in_block );
        return NULL;
    }
    void *tok_data() {
        if ( offset_of_first_tok_in_block )
            return reinterpret_cast<void *>( (char *)this + offset_of_first_tok_in_block );
        return NULL;
    }
};

std::ostream &operator<<( std::ostream &os, const TokHeader &tok_header );

/**
 */
struct TokData {
    char *data;
    unsigned size;
};



#endif
