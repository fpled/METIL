//
// C++ Interface: lexemdefinition
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LEXEMDEFINITION_H
#define LEXEMDEFINITION_H

#include "splittedvec.h"

struct Lexem;
struct ErrorList;
class SourceFile;

/**

*/
class LexemDefinition {
public:
    void init( Lexem *o, const char *provenance, ErrorList *error_list );
    bool needs_try_block() const;
    void set_offset_to_property_name();

    struct Argument {
        const Lexem *tname;
        const Lexem *default_value;
        const Lexem *type_constraint;
    };
    typedef SplittedVec<const Lexem *,8> TParentList;
    typedef SplittedVec<const Lexem *,2> TInheritanceList;
    typedef SplittedVec<Argument,8> TArgList;
    
    TArgList arguments;
    unsigned min_nb_arg;

    const Lexem *tname;
    const Lexem *condition;
    const Lexem *return_type;
    const Lexem *pertinence;
    TInheritanceList inheritance;
    double default_pertinence; /// DOUBLE_MAX_VAL if pertinence is calculated

    Lexem *block; /// if defined in METIL language
    TParentList parents; ///
    const Lexem *parent_class; /// only if method
    bool method, is_class, self_as_arg, varargs, abstract;
    unsigned offset_to_property_name, type_property;
};

#endif
