//
// C++ Implementation: lexemdefinition
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <iostream>
#include "lexemdefinition.h"
#include "lexemnumber.h"
#include <usual_strings.h>
#include <string.h>
#include "lexem.h"
#include "errorlist.h"
// #include "tok.h"
#include "config.h"

/// != get_children_of_type because it takes left and right arguments
unsigned nb_child_and( const Lexem *t ) {
    if ( t->type != STRING_and_boolean_NUM ) {
        if ( (t->type == Lexem::PAREN or t->type == Lexem::BLOCK) and t->children[0] )
            return nb_child_and( t->children[0] );
        return 1;
    }
    return nb_child_and(t->children[0]) + nb_child_and(t->children[1]);
}

void get_def_parents( const Lexem *c, LexemDefinition::TParentList &res ) {
    const Lexem *leftmost = c;
    while ( leftmost->prev ) leftmost = leftmost->prev;
    if ( leftmost->parent )
        get_def_parents( leftmost->parent, res );
    if ( c->type == STRING___def___NUM or c->type == STRING___class___NUM )
        res.push_back( c );
}


/** 
 *   (c)def +-> inherits   +-> pertinence +-> when       +-> :         +-> apply     +-> name
 *          |              |              |              |             |             |-> args
 *          |              |              |              |             |-> return_type
 *          |              |              |              |-> condition
 *          |              |              +-> pert value
 *          |              +-> inheritance
 *          +-> block
 */
void LexemDefinition::init( Lexem *t, const char *provenance, ErrorList *error_list ) {
    is_class = (t->type==STRING___class___NUM);

    // def parents
    get_def_parents( t, parents );
    method = ( parents.size()>=2 and parents[parents.size()-2]->type == STRING___class___NUM );
    if ( method ) parent_class = parents[parents.size()-2];
    else parent_class = NULL;
    
    // block
    if ( t->children[1]->type==Lexem::PAREN or t->children[1]->type==Lexem::BLOCK )
        block = t->children[1]->children[0];
    else
        block = t->children[1];
    
    // inheritance
    const Lexem *c = t->children[0];
    if ( c->type == STRING___inherits___NUM ) {
        get_children_of_type( c->children[1], STRING___comma___NUM, inheritance );
        c = c->children[0];
    }
    
    // pertinence
    if ( c->type == STRING___pertinence___NUM ) {
        pertinence = c->children[1];
        c = c->children[0];
    }
    else
        pertinence = NULL;
    
    // condition
    if ( c->type == STRING___when___NUM ) {
        condition = c->children[1];
        c = c->children[0];
    }
    else
        condition = NULL;
    
    // return_type
    if ( c->type == STRING_doubledot_NUM ) {
        return_type = c->children[1];
        c = c->children[0];
    }
    else
        return_type = NULL;
    
    // name and arguments
    if ( c->type == Lexem::APPLY or c->type == Lexem::SELECT ) {
        tname = c->children[0];

        SplittedVec<const Lexem *,8> tl;
        get_children_of_type( c->children[1], STRING___comma___NUM, tl );

        bool mandatory_default_val = false;
        for(unsigned i=0;i<tl.size();++i) {
            if ( not tl[i] ) continue;

            LexemDefinition::Argument arg;
            const Lexem *t = tl[i];
            // look for a=...
            if ( t->type == STRING_reassign_NUM ) {
                arg.default_value = t->children[1];
                if ( arg.default_value->type == STRING_doubledot_NUM ) {
                    error_list->add( "Syntax for argument with type constraint and default value is 'name_arg : type_constraint = default_value' (in this specific order).", arg.default_value->s, provenance );
                    return;
                }
                mandatory_default_val = true;
                t = t->children[0];
            }
            else if ( mandatory_default_val ) {
                error_list->add( "After one argument with default value, all arguments must have default values.", tl[i]->s, provenance );
                return;
            }
            else
                arg.default_value = NULL;
            
            // look for a:Toto
            if ( t->type == STRING_doubledot_NUM ) {
                arg.type_constraint = t->children[1];
                t = t->children[0];
            }
            else
                arg.type_constraint = NULL;
            
            // name
            arg.tname = t;
            // register
            arguments.push_back( arg );
        }
    }
    else {
        tname = c;
    }

    // check that name is correct
    if ( tname->children[1] or (tname->children[0] and tname->children[0]->type!=Lexem::PAREN) ) {
        error_list->add( "syntax error.", tname->s, provenance );
        return;
    }

    // default_pertinence
    if ( pertinence ) {
        if ( pertinence->type==Lexem::NUMBER ) {
            default_pertinence = pertinence->number->to_double();
            pertinence = NULL;
        }
        else if ( pertinence->type==STRING_sub_NUM and pertinence->children[1]==0 and pertinence->children[0]->type==Lexem::NUMBER ) {
            default_pertinence = -pertinence->children[0]->number->to_double();
            pertinence = NULL;
        }
        else
            default_pertinence = FLOAT64_MAX_VAL;
    }
    else {
        default_pertinence = ( condition ? nb_child_and( condition ) : 0 );
        for(unsigned i=0;i<arguments.size();++i) default_pertinence += (arguments[i].type_constraint!=NULL);
        default_pertinence += method;
    }

    // self_as_arg, varargs
    self_as_arg = ( arguments.size() and arguments[0].tname->type==STRING___self___NUM );
    varargs = ( arguments.size() and arguments.back().tname->type==Lexem::VARIABLE and
            arguments.back().tname->si==7 and strncmp(arguments.back().tname->s,"varargs",7)==0 );

    // min_nb_arg
    min_nb_arg = 0;
    for(unsigned i=self_as_arg;i<arguments.size()-varargs;++i)
        min_nb_arg += ( arguments[i].default_value == NULL );


    // basic assertions
    for(unsigned i=self_as_arg;i<arguments.size();++i) {
        Argument &arg = arguments[i];
        if ( arg.tname->type == STRING___self___NUM )
            error_list->add( "'self' as non first argument name of a def is weird.", arg.tname->s, provenance );
        if ( arg.type_constraint and arg.type_constraint->type != Lexem::VARIABLE )
            error_list->add( "type constraints must be simple variables (no composition).", arg.type_constraint->s, provenance );
    }
    
    // property
    set_offset_to_property_name();
    
    // abstract
    abstract = ( block->type == STRING___abstract___NUM );
}

bool LexemDefinition::needs_try_block() const {
    for(unsigned i=0;i<arguments.size();++i)
        if ( arguments[i].default_value )
            return true;
    return ( condition or pertinence );
}

void LexemDefinition::set_offset_to_property_name() {
    if ( tname->type == Lexem::VARIABLE ) {
        static const char *b[] = { "get_", "init_", "reassign_", "apply_on_", "type_of_" };
        static unsigned    l[] = {      4,       5,           9,           9,          8 };
        for(unsigned i=0;i<5;++i) {
            if ( tname->si > l[i] and strncmp( tname->s, b[i], l[i] )==0 ) {
                offset_to_property_name = l[i];
                type_property = i;
                return;
            }
        }
    }
    offset_to_property_name = 0;
    type_property = 0;
}

