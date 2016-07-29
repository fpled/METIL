#ifndef FUNCTIONNAL_H
#define FUNCTIONNAL_H

#include "splittedvec.h"
#include "variable.h"

struct Thread;
struct VarArgs;

/**
    f( 36, _1 )
    _1.f(  )
*/
struct Functionnal {
    static const int NOT_A_PREDEFINED_FUNCTIONNAL = -1;
    static const int SELF = -2;

    void init();
    void init( Functionnal *f, bool inc_refs = true );
    void copy_data_from( Functionnal *f );
    
    bool comes_from_a_predefined_functionnal() const { return get_var_nb != NOT_A_PREDEFINED_FUNCTIONNAL; }
    bool comes_from_an_apply_with_functionnal_arg() const { return var.type and attribute.v == 0; }
    bool comes_from_a_get_attr_with_functionnal_lhs() const { return attribute.v != 0; }
    
    int get_var_nb; /// -1 -> no var, -2 -> self, > 0 -> var nb
    Variable var; /// definition ( 'f' in something like 'f(_1,3)' ) if variables, Functionnal else
    SplittedVec<Variable,4,4> variables;
    Nstring attribute; /// _1.attribute
    unsigned nb_named; /// associated with variables
    
    operator bool() const { return get_var_nb!=-1 or var.type or variables.size() or attribute!=0; }
};

const void *destroy( Functionnal &d, Thread *th, Variable * &sp );
std::ostream &operator<<( std::ostream &os, const Functionnal &d );

void extract_arg_defined_in_functionnal( Thread *th, const void *tok, Variable *return_var, Variable *self_var, Functionnal *f, VarArgs *va );
void extract_var_defined_in_functionnal( Variable *return_var, Functionnal *f );
void extract_var_nb_defined_in_functionnal( Variable *return_var, Functionnal *f, Int32 n );

#endif // FUNCTIONNAL_H
