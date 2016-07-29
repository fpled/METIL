#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "typeconfig.h"
#include "splittedvec.h"
#include "variable.h"
#include "symbolic/op.h"
#include <string>
#include <map>
#include <simplevector.h>

struct Thread;
struct Variable;
struct Definition;

struct CodeWriter {
    void init( const char *s, Int32 si );
    void init( CodeWriter &c );
    void reassign( Thread *th, const void *tok, CodeWriter &c );
    void add_expr( Thread *th, const void *tok, Variable *str, Op *expr, Definition &b );
    void add_expr( Op *op, Nstring method, char *name );
    std::string to_string( Thread *th, const void *tok, Int32 nb_spaces );
    
    operator bool() const { return true; }
    
    struct OpToWrite {
        Op *op; 
        Nstring method;
        char *name;
    };
    struct NumberToWrite {
        Rationnal n;
        Nstring method;
        char *name;
    };
    struct ParentsOpAndNumReg {
        ParentsOpAndNumReg() : num_reg( -1 ), used( 0 ), num_in_seq( -1 ), test_front( false ) {  }
        SplittedVec<Op *,4,64> parents;
        SplittedVec<OpToWrite *,1,4> res;
        int num_reg;
        int used, to_be_used;
        int num_in_seq;
        bool test_front;
    };
    struct RegList {
        void init() { nb_declared_registers = 0; free_ones.init(); }
        
        unsigned nb_declared_registers;
        SplittedVec<int,64> free_ones;
    };
    struct AlreadyCalculated {
        Op *op;
        int num_reg;
    };
    
    SplittedVec<OpToWrite,32> op_to_write;
    SplittedVec<NumberToWrite,32> nb_to_write;
    SimpleVector<AlreadyCalculated> already_calculated;
    RegList free_registers;

    bool has_init_methods;
    char *basic_type;
private:
    void write_code( std::ostream &os, SplittedVec<Op *,256,1024> &front, Int32 nb_spaces, bool &put_a_cr );
    void write_code_with_cond_0_and_1( Thread *th, const void *tok, std::ostream &os, Op *cond, Int32 nb_spaces, SplittedVec<Op *,1024,4096> &of );
    void write_particular_cases_with_cond_0_and_1( Thread *th, const void *tok, std::ostream &os, SplittedVec<Op *,32> &subs_values, Int32 nb_spaces, SimpleVector<AlreadyCalculated> &already_calculated );
};
    
    
void destroy( Thread *th, const void *tok, CodeWriter &c );
    
    
#endif // CODE_WRITER_H

