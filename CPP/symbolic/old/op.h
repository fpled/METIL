#ifndef OP_H
#define OP_H

#include "splittedvec.h"
#include "config.h"
#include <usual_strings.h>

struct Type;
struct Thread;
struct VarArgs;
struct Variable;

struct Op {
    static const int NUMBER = -3;
    static const int SYMBOL = -2;
    static const int SUMSEQ = -1;

    int type;
    SplittedVec<Op *,2,8> parents;
    Op *additional_info;
    unsigned cleared_id;
    unsigned cpt_use;
    int integer_type;
    
    /// if NUMBER
    struct NumberData {
        Rationnal val;
    };
    /// if SYMBOL
    struct SymbolData {
        char *cpp_name_str;
        char *tex_name_str;
    };
    /// if type >= 0
    struct FuncData {
        static const unsigned max_nb_children = 2;
        Op *children[max_nb_children];
    };
    
    //
    static Op &new_number( const Rationnal &val ); ///
    static Op &new_symbol( const char *cpp_name_str, unsigned cpp_name_si, const char *tex_name_str, unsigned tex_name_si );
    static Op &new_function( int type, Op &a );
    static Op &new_function( int type, Op &a, Op &b );
    void clear_additional_info_rec();
    void clear_additional_info_rec( unsigned clear_id );
    Op &inc_ref() { ++cpt_use; return *this; }
    void destroy();
    
    bool is_zero();
    bool is_one();
    bool is_minus_one();
    bool necessary_positive_or_null();
    
    // ..._data 
    NumberData *number_data() { return reinterpret_cast<NumberData *>( this + 1 ); }
    SymbolData *symbol_data() { return reinterpret_cast<SymbolData *>( this + 1 ); }
    FuncData   *func_data  () { return reinterpret_cast<FuncData   *>( this + 1 ); }
};

Op *diff( Thread *th, const void *tok, Op *a, Op *b );
Op *subs( Thread *th, const void *tok, Op *self, VarArgs &a, VarArgs &b );
Op &subs_rec( Thread *th, const void *tok, Op &a, SplittedVec<Op *,1024,4096> &of );
Rationnal subs_numerical( Thread *th, const void *tok, Op *self, const Rationnal &a );
void get_sub_symbols( Op *op, SplittedVec<Op *,32> &symbols );
void discontinuities_separation( Thread *th, const void *tok, Variable *return_var, Op *expr, VarArgs &symbols );
bool depends_on_rec( Op *op, Op *var );

void graphviz_repr( std::ostream &os, Op *op );
void cpp_repr     ( std::ostream &os, Op *op );
void tex_repr     ( std::ostream &os, Op *op );

std::string graphviz_repr( Op *op );
std::string cpp_repr     ( Op *op );
std::string tex_repr     ( Op *op );

inline std::ostream &operator<<(std::ostream &os,Op &op) { cpp_repr( os, &op ); return os; }

inline void dec_ref( Op *op ) { if ( not op->cpt_use ) { op->destroy(); free( op ); } else --op->cpt_use; }

inline bool are_nb(Op &a,Op &b) { return a.type==Op::NUMBER and b.type==Op::NUMBER; }
inline bool is_a_sub( Op &a ) { return a.type == STRING_mul_NUM and a.func_data()->children[0]->is_minus_one(); }
inline bool is_a_inv( Op &a ) { return a.type == STRING_pow_NUM and a.func_data()->children[1]->is_minus_one(); }
inline bool is_a_mul_by_neg_number( Op &a ) { return a.type == STRING_mul_NUM and a.func_data()->children[0]->type == Op::NUMBER and a.func_data()->children[0]->number_data()->val.is_neg(); }

Op &operator+( Op &a, Op &b );
Op &operator-( Op &a, Op &b );
Op &operator-( Op &a        );
Op &operator*( Op &a, Op &b );
Op &operator/( Op &a, Op &b );

Op &mod  ( Op &a, Op &b );
Op &pow  ( Op &a, Op &b );
Op &atan2( Op &a, Op &b );

Op &abs      ( Op &a );
Op &log      ( Op &a );
Op &heaviside( Op &a );
Op &eqz      ( Op &a );
Op &exp      ( Op &a );
Op &sin      ( Op &a );
Op &cos      ( Op &a );
Op &tan      ( Op &a );
Op &asin     ( Op &a );
Op &acos     ( Op &a );
Op &atan     ( Op &a );
Op &sinh     ( Op &a );
Op &cosh     ( Op &a );
Op &tanh     ( Op &a );

Op *__integration__( Thread *th, const void *tok, Op *expr, Op *var, Op *beg, Op *end, Int32 deg_poly_max );

#endif


