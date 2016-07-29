//
// C++ Interface: lexem
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LEXEM_H
#define LEXEM_H

// #include "common/usual_strings.h"
#include "splittedvec.h"

/**
	@author Grover <hugo@gronordi>
*/
class Lexem {
public:
    Lexem();
    ~Lexem();

    enum {
        VARIABLE = -1, // must be -1 ( @see num_operator_le )
        NUMBER = -2,
        STRING = -3,
        CR = -4,
        PAREN = -5,
        CCODE = -6,
        BLOCK = -9,
        NONE = -10,
        APPLY = -12,
        SELECT = -13,
        CHANGE_BEHAVIOR = -14,
    };

    int type; /// >0 => operator. <=0 => @see enum
    int num; /// Used by type==CR for nb_spaces. If type==CCODE, num in size_cvar
    
    const char *s; /// beginning in sar
    unsigned si;   /// size in sar
    
    Lexem *children[2], *parent, *next, *prev, *sibling;

    class LexemDefinition *def; /// if class or def
    class LexemNumber *number; /// if number
};

inline int is_in_main_block(const Lexem *t) { while( t->prev ) t = t->prev; return not t->parent; }
inline int need_larg(const Lexem *t) { return t and t->type>=0 and (t->num & 1); }
inline int need_rarg(const Lexem *t) { return t and t->type>=0 and (t->num & 2); }

void display_graph(const Lexem *t,const char *file_name="res.dot");

/// a,b,c -> [a b c]
template<class TL> void get_children_of_type(const Lexem *t,int type,TL &res) {
    if ( not t ) return;
    if ( t->type != type ) {
        res.push_back( t );
        return;
    }
    get_children_of_type( t->children[0], type, res );
    res.push_back( t->children[1] );
}

inline unsigned nb_children_of_type(const Lexem *t,int type) {
    SplittedVec<const Lexem *,8> res;
    get_children_of_type(t,type,res);
    return res.size();
}

const Lexem *leftmost_child(const Lexem *t);
const Lexem *rightmost_child(const Lexem *t);
int offset_to_doc_of( const Lexem *l, const char *sar_txt );

inline const Lexem *child_if_block(const Lexem *t) { if (t->type==Lexem::BLOCK) return t->children[0]; return t; } ///
inline       Lexem *child_if_block(      Lexem *t) { if (t->type==Lexem::BLOCK) return t->children[0]; return t; } ///

inline const Lexem *child_if_paren(const Lexem *t) { if (t->type==Lexem::PAREN) return t->children[0]; return t; } ///

inline const Lexem *next_if_CR( const Lexem *cc ) {
    if ( cc and cc->type == Lexem::CR )
        return cc->next;
    return cc;
}

#endif
