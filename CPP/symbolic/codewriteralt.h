#ifndef CODE_WRITER_ALT_H
#define CODE_WRITER_ALT_H

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

struct CodeWriterAlt {
    void init( const char *a, Int32 ai, const char *b, Int32 bi, const char *c, Int32 ci, const char *d, Int32 di);
    void init( CodeWriterAlt &c );
    
    void make_cw_parallele (bool is_para);
    
    void reassign( Thread *th, const void *tok, CodeWriterAlt &c );
    void add_expr( Thread *th, const void *tok, Variable *str, const Ex &expr, Definition &b );
    void add_expr( const Ex &op, Nstring method, char *name );
    std::string to_string( Thread *th, const void *tok, Int32 nb_spaces );
    std::string to_graphviz( Thread *th, const void *tok );
    
    std::string invariant( Thread *th, const void *tok, Int32 nb_spaces, const VarArgs &variables );
    
    operator bool() const { return true; }
    
    struct OpToWrite {
        Ex ex; 
        Nstring method;
        char *name;
    };
    struct NumberToWrite {
        Rationnal n;
        Nstring method;
        char *name;
    };
    
    SplittedVec<OpToWrite    ,32> op_to_write;
    SplittedVec<NumberToWrite,32> nb_to_write;
    
    
    bool is_parallele;
    bool has_init_methods;
    char *basic_type, *basic_simd;
    char *basic_integer_type, *basic_integer_simd;
private:
    struct OpWithSeq *make_seq();

    bool want_float;
};
    
    
void destroy( Thread *th, const void *tok, CodeWriterAlt &c );
    
    
#endif // CODE_WRITER_ALT_H

