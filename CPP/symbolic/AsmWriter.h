#ifndef ASM_WRITER_ALT_H
#define ASM_WRITER_ALT_H

#include "typeconfig.h"
#include "splittedvec.h"
#include "variable.h"
#include "symbolic/ex.h"
#include <string>
#include <map>
#include <simplevector.h>

struct Thread;
struct Variable;
struct Definition;

struct AsmWriter {
    void init();
    void init( AsmWriter &c );
    void reassign( Thread *th, const void *tok, AsmWriter &c );
    
    void add_expr( Thread *th, const void *tok, void *ptr_res, const Ex &expr, Definition &b );
    void add_association( Thread *th, const void *tok, const Ex &expr, void *ptr_val, Definition &b );
    
    void *to_code( Thread *th, const void *tok );
    std::string to_graphviz( Thread *th, const void *tok );
    
    operator bool() const { return true; }
    
    struct OpToWrite {
        Ex ex; 
        Nstring method;
        void *ptr_res;
    };
    struct NumberToWrite {
        Rationnal n;
        Nstring method;
        void *ptr_res;
    };
    struct Association {
        Ex ex;
        void *ptr_val;
        int nstring_type;
    };
    
    SplittedVec<OpToWrite    ,32> op_to_write;
    SplittedVec<NumberToWrite,32> nb_to_write;
    SplittedVec<Association  ,32> associations;
    
private:
    struct OpWithSeq *make_seq();

    bool has_init_methods;
    bool want_float;
};
    
    
void destroy( Thread *th, const void *tok, AsmWriter &c );
    
    
#endif // ASM_WRITER_ALT_H

