#ifndef OP_WITH_SEQ_H
#define OP_WITH_SEQ_H

#include <vector>

struct OpWithSeq {
    static const int NUMBER         = -3;
    static const int SYMBOL         = -2;
    static const int WRITE_ADD      = -10;
    static const int WRITE_INIT     = -11;
    static const int WRITE_RET      = -12;
    static const int WRITE_REASSIGN = -13;
    static const int SEQ            = -14; //
    static const int INV            = -20; // 1 / a
    static const int NEG            = -21; // - a
    static const int NUMBER_M1      = -22; // ~( 1 << 63 ) in a long long word

    OpWithSeq( int t );
    OpWithSeq( int method, const char *name, OpWithSeq *ch ); // WRITE_...
    OpWithSeq( int method, void *ptr_res, OpWithSeq *ch ); // WRITE_...
    OpWithSeq( const char *cpp_name );
    void init_gen();
    
    static OpWithSeq *new_number( const Rationnal &n );
    static OpWithSeq *new_number( double n );
    static OpWithSeq *new_number_M1();
    static void clear_number_set();
    
    ~OpWithSeq();
    
    void add_child( OpWithSeq *c );
    double val() const { return num / den; }
    bool in_Z() const { double a = std::abs(val()); return std::abs( a - int(a+0.5) ) < 1e-6; }
    bool is_minus_one() const { return type == NUMBER and num == -den; }
    bool is_mul_minus_one() const;
    unsigned nb_instr() const;
    void graphviz_rec( std::ostream &ss ) const;
    bool has_couple( const OpWithSeq *a, const OpWithSeq *b ) const;
    void remove_parent( OpWithSeq *parent );
    bool eq_nb( double val ) const { return type == NUMBER and num / den == val; }
    std::string asm_str() const;
    
    int min_parent_date() const {
        assert( parents.size() );
        int res = parents[0]->ordering;
        for(unsigned i=1;i<parents.size();++i)
            res = std::min( res, parents[i]->ordering );
        return res;
    }

    int type;
    std::vector<OpWithSeq *> children;
    std::vector<OpWithSeq *> parents;
    double num, den;
    double access_cost;
    int nb_simd_terms;
    char *cpp_name_str;
    void *ptr_res;
    void *ptr_val; // != NULL if value if somewhere in memory
    int reg, stack, ordering, nb_times_used;
    mutable unsigned id;
    static unsigned current_id;
    int integer_type;
    int nstring_type;
};

OpWithSeq *make_OpWithSeq_rec( const struct Op *op );
void simplifications( OpWithSeq *op );
void make_binary_ops( OpWithSeq *op );
void update_cost_access_rec( OpWithSeq *op );
void update_nb_simd_terms_rec( OpWithSeq *op );
void asm_simplifications_prep( OpWithSeq *seq );
void asm_simplifications_post( OpWithSeq *seq );

bool same( const OpWithSeq *a, const OpWithSeq *b );

OpWithSeq *new_inv( OpWithSeq *ch );
OpWithSeq *new_neg( OpWithSeq *ch );
OpWithSeq *new_add_or_mul( int type, const std::vector<OpWithSeq *> &l );
OpWithSeq *new_sub_or_div( int type, OpWithSeq *p, OpWithSeq *n );
OpWithSeq *new_pow( OpWithSeq *m, double e );

void make_OpWithSeq_simple_ordering( OpWithSeq *seq, std::vector<OpWithSeq *> &ordering, bool want_asm = false );

std::ostream &operator<<( std::ostream &os, const OpWithSeq &op );

#endif // OP_WITH_SEQ_H
