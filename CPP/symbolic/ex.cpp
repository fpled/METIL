#include "ex.h"
#include "op.h"
#include "globaldata.h"
#include "thread.h"
#include "../rectilinear_iterator.h"
#include <sstream>
#include <complex>
#include <map>
#include <algorithm>

// #define ALWAYS_DISTRIBUTE_NUMBER
// #define NEVER_DISTRIBUTE_NUMBER
// #define WITHOUT_SIMP
// #define A_POSTERIORI_SIMPLIFICATION_AFTER_EACH_ADD
#define USE_SERIES_FOR_TAYLOR

inline std::complex<Ex> powc ( const std::complex<Ex> &v, const Rationnal &p ) {
    Ex mo = v.real() * v.real() + v.imag() * v.imag();
    Ex m = log ( mo + eqz ( mo ) ) / 2, a = atan2 ( v.imag(), v.real() );
    return std::exp ( Ex ( p ) * std::complex<Ex> ( m, a ) );
}

inline std::complex<Ex> sqrtc ( const Ex &v ) {
    Ex s = pow ( abs ( v ), Rationnal ( 1,2 ) );
    Ex p = heaviside ( v );
    return std::complex<Ex> ( s * p, s * ( 1 - p ) );
}

struct ExPtrCmp {
    bool operator() ( const Ex &a, const Ex &b ) const {
        if ( a.op->type==Op::NUMBER and b.op->type==Op::NUMBER )
        return a.op->number_data()->val < b.op->number_data()->val;
        return a.op < b.op;
    }
};

inline bool same_ex ( const Ex &a, const Ex &b ) {
    return same_op ( a.op, b.op );
}


void Ex::init() {
    static Ex zero ( 0 );
//     op = Op::new_number( 0 );
//     op->inc_ref();
    op = zero.op->inc_ref();
}

void Ex::init ( Op *ex ) {
    op = ex;
    op->inc_ref();
}

void Ex::init ( const Ex &ex ) {
    op = ex.op;
    op->inc_ref();
}

void Ex::init ( const T &v ) {
    op = Op::new_number ( v );
    op->inc_ref();
}

void Ex::init ( int v ) {
    op = Op::new_number ( v );
    op->inc_ref();
}

void Ex::init ( const char *cpp_name_str, unsigned cpp_name_si, const char *tex_name_str, unsigned tex_name_si ) {
    op = Op::new_symbol ( cpp_name_str, cpp_name_si, tex_name_str, tex_name_si );
    op->inc_ref();
}

void Ex::reassign ( const Ex &c ) {
    c.op->inc_ref();
    dec_ref ( op );
    op = c.op;
}

void Ex::destroy() {
    dec_ref ( op );
}

bool Ex::known_at_compile_time() const {
    return ( op->type == Op::NUMBER );
}

std::string Ex::graphviz_repr ( Thread *th, const void *tok ) const {
    std::ostringstream ss;
    SplittedVec<const Op *,32> dep;
    op->graphviz_repr ( ss, dep );
    return ss.str();
}
std::string Ex::graphviz_repr ( Thread *th, const void *tok, const VarArgs &a ) const {
    std::ostringstream ss;
    SplittedVec<const Op *,32> dep;
    for ( unsigned i=0;i<a.nb_uargs();++i )
    dep.push_back ( reinterpret_cast<Ex *> ( a.uarg ( i )->data )->op );
    op->graphviz_repr ( ss, dep );
    return ss.str();
}

std::string Ex::cpp_repr() const {
        std::ostringstream ss;
        op->cpp_repr ( ss );
        return ss.str();
    }

    std::string Ex::tex_repr() const {
        std::ostringstream ss;
        op->tex_repr ( ss );
        return ss.str();
    }

    std::string Ex::ser_repr() const {
        std::ostringstream ss;
        op->ser_repr ( ss );
        return ss.str();
    }

    Ex::T Ex::value ( Thread *th, const void *tok ) const {
    if ( op->type != Op::NUMBER ) {
    th->add_error ( "symbol is not a known number", tok );
        return 0;
    }
return op->number_data()->val;
}

Ex::T Ex::value() const {
    assert ( op->type == Op::NUMBER );
    return op->number_data()->val;
}

bool Ex::depends_on ( const Ex &a ) const {
    return op->depends_on ( a.op );
}

Op *to_inc_op ( const Ex &ex ) {
    return ex.op->inc_ref();
}

std::ostream &operator<< ( std::ostream &os, const Ex &ex ) {
    ex.op->cpp_repr ( os );
    return os;
}

Ex &Ex::operator+= ( const Ex &c ) {
    *this = *this + c;
    return *this;
}

Ex &Ex::operator*= ( const Ex &c ) {
    *this = *this * c;
    return *this;
}

Ex &Ex::operator-= ( const Ex &c ) {
    *this = *this - c;
    return *this;
}

Ex &Ex::operator/= ( const Ex &c ) {
    *this = *this / c;
    return *this;
}

void Ex::set_beg_value ( Thread *th, const void *tok, T b, bool inclusive ) {
    if ( op->set_beg_value ( b, inclusive ) )
        th->add_error ( "symbol ->beg_value already a parent", tok );
}

void Ex::set_end_value ( Thread *th, const void *tok, T e, bool inclusive ) {
    if ( op->set_end_value ( e, inclusive ) )
        th->add_error ( "symbol ->beg_value already a parent", tok );
}

bool Ex::beg_value_valid() const {
    return op->beg_value_valid;
}

void Ex::set_access_cost ( Float64 c ) {
    if ( op->type == Op::SYMBOL )
        op->symbol_data()->access_cost = c;
}
void Ex::set_nb_simd_terms ( Int32 c ) {
    if ( op->type == Op::SYMBOL )
        op->symbol_data()->nb_simd_terms = c;
}

void Ex::set_integer_type ( Int32 c ) {
    op->integer_type = c;
}

bool Ex::end_value_valid() const {
    return op->end_value_valid;
}

bool Ex::beg_value_inclusive() const {
    return op->beg_value_inclusive;
}

bool Ex::end_value_inclusive() const {
    return op->end_value_inclusive;
}

Rationnal Ex::beg_value() const {
    return op->beg_value;
}

Rationnal Ex::end_value() const {
    return op->end_value;
}

// --------------------------------------------------------------------------------------------
struct SumSeq {
    Rationnal c;
    const Op *op;
    Ex to_op() {
        if ( c.is_one() )
            return op;
        return Ex ( c ) * Ex ( op );
    }
};

Ex make_add_seq ( SplittedVec<SumSeq,4,16,true> &items_c ) {
    if ( not items_c.size() )
        return 0;

    //     // only negative numbers ?
    //     bool all_neg = true;
    //     for(unsigned i=0;i<items_c.size();++i)
    //         all_neg &= items_c[i].c.is_neg();
    //     if ( all_neg ) {
    //         for(unsigned i=0;i<items_c.size();++i)
    //             items_c[i].c = abs( items_c[i].c );
    //         Ex res = items_c[0].to_op();
    //         for(unsigned i=1;i<items_c.size();++i)
    //             res = res + items_c[i].to_op();
    //         return - res;
    //     }

    // else
    Ex res = items_c[0].to_op();
    for ( unsigned i=1;i<items_c.size();++i )
        res = res + items_c[i].to_op();
    return res;
}

/// @see operator+
void find_add_items_and_coeff_rec ( const Op *a, SplittedVec<SumSeq,4,16,true> &items, Ex::T pre_mul = 1 ) {
    if ( a->type == STRING_add_NUM ) { // anything_but_a_number + anything_but_a_number
        find_add_items_and_coeff_rec ( a->func_data()->children[0], items, pre_mul );
        find_add_items_and_coeff_rec ( a->func_data()->children[1], items, pre_mul );
    } else if ( a->type == STRING_mul_NUM and a->func_data()->children[0]->type == Op::NUMBER ) { // 10 * a
        find_add_items_and_coeff_rec ( a->func_data()->children[1], items, pre_mul * a->func_data()->children[0]->number_data()->val );
    } else { // a
        SumSeq *s = items.new_elem();
        s->c.init ( pre_mul );
        s->op = a;
    }
}
/// @see operator+
void add_items_and_coeffs ( const SplittedVec<SumSeq,4,16,true> &items_a, SplittedVec<SumSeq,4,16,true> &items_b ) {
    for ( unsigned i=0;i<items_a.size();++i ) {
        for ( unsigned j=0;;++j ) {
            if ( j == items_b.size() ) {
                items_b.push_back ( items_a[i] );
                break;
            }
            // std::cout << Ex(items_a[ i ].op) << " " << items_a[ i ].c << " " << Ex(items_b[ j ].op) << std::endl;
            if ( items_a[ i ].op == items_b[ j ].op ) {
                items_b[ j ].c += items_a[ i ].c;
                if ( not items_b[ j ].c )
                    items_b.erase_elem_nb_in_unordered_list ( j );
                break;
            }
        }
    }
}

void update_inter_add ( Op *res, Op *a, Op *b ) {
    if ( not res->cpt_use ) {
        if ( a->beg_value_valid and b->beg_value_valid )
            res->set_beg_value ( a->beg_value + b->beg_value, a->beg_value_inclusive and b->beg_value_inclusive );
        if ( a->end_value_valid and b->end_value_valid )
            res->set_end_value ( a->end_value + b->end_value, a->end_value_inclusive and b->end_value_inclusive );
    }
}

Ex add_number_and_expr ( const Ex &a, const Ex &b ) { // a is a number
    if ( a.op->number_data()->val.is_zero() )
        return b;
    // 10 + ( 5 + a )
    if ( b.op->type == STRING_add_NUM and b.op->func_data()->children[0]->type == Op::NUMBER ) {
        return Ex ( a.op->number_data()->val + b.op->func_data()->children[0]->number_data()->val ) +
               Ex ( b.op->func_data()->children[1] );
    }
    Op *res = Op::new_function ( STRING_add_NUM, a.op, b.op );
    update_inter_add ( res, a.op, b.op );
    return res;
}

Ex operator+ ( const Ex &a, const Ex &b ) {
    // 10 + ...
    if ( a.op->type == Op::NUMBER ) {
        // 10 + 20
        if ( b.op->type == Op::NUMBER )
            return a.op->number_data()->val + b.op->number_data()->val;
        // 10 + ( ... )
        return add_number_and_expr ( a, b );
    }
    // ( not a number ) + 10 -> 10 + ( not a number )
    if ( b.op->type == Op::NUMBER )
        return add_number_and_expr ( b, a );

    // ( 10 + a ) + ( not a number ) -> 10 + ( a + ( not a number ) )
    if ( a.op->type == STRING_add_NUM and a.op->func_data()->children[0]->type == Op::NUMBER ) {
        // ( 10 + a ) + ( 20 + b ) -> 30 + ( a + b )
        if ( b.op->type == STRING_add_NUM and b.op->func_data()->children[0]->type == Op::NUMBER ) {
            Ex new_num = a.op->func_data()->children[0]->number_data()->val + b.op->func_data()->children[0]->number_data()->val;
            Ex new_sum = Ex ( a.op->func_data()->children[1] ) + Ex ( b.op->func_data()->children[1] );
            return new_num + new_sum;
        }
        // ( 10 + a ) + b -> 10 + ( a + b )
        Ex new_sum = Ex ( a.op->func_data()->children[1] ) + b;
        return Ex ( a.op->func_data()->children[0] ) + new_sum;
    }
    // a + ( 10 + b ) -> 10 + ( a + b )
    if ( b.op->type == STRING_add_NUM and b.op->func_data()->children[0]->type == Op::NUMBER ) {
        Ex new_sum = a + b.op->func_data()->children[1];
        return Ex ( b.op->func_data()->children[0] ) + new_sum;
    }

    // ( 10 * a + b + anything_but_a_number ) + ( 2 * b + a + anything_but_a_number ) -> ...
    SplittedVec<SumSeq,4,16,true> items_a, items_b;
    find_add_items_and_coeff_rec ( a.op, items_a );
    find_add_items_and_coeff_rec ( b.op, items_b );
    unsigned old_size = items_a.size() + items_b.size();
    add_items_and_coeffs ( items_b, items_a );
    if ( old_size != items_a.size() )
        return make_add_seq ( items_a );

    //
    Op *res;
    if ( a.op < b.op )
        res = Op::new_function ( STRING_add_NUM, a.op, b.op );
    else
        res = Op::new_function ( STRING_add_NUM, b.op, a.op );

    // interval
    update_inter_add ( res, a.op, b.op );

    #ifdef A_POSTERIORI_SIMPLIFICATION_AFTER_EACH_ADD
    return add_a_posteriori_simplification ( res );
    #endif

    return res;
}

Ex operator- ( const Ex &a, const Ex &b ) {
    return a + ( -b );
}

Ex operator- ( const Ex &a ) {
    if ( a.op->type == Op::NUMBER )
        return - a.op->number_data()->val;
    if ( a.op->type == STRING_add_NUM and a.op->func_data()->children[0]->type == Op::NUMBER ) { // - ( 1 + a ) -> -1 + (-a)
        Ex new_num = - a.op->func_data()->children[0]->number_data()->val;
        return new_num + ( - Ex ( a.op->func_data()->children[1] ) );
    }
    if ( a.op->type == STRING_mul_NUM and a.op->func_data()->children[0]->type == Op::NUMBER ) { // - 10 * a -> (-10) * a
        Ex new_num = - a.op->func_data()->children[0]->number_data()->val;
        return new_num * Ex ( a.op->func_data()->children[1] );
    }
    return Ex ( -1 ) * a; // (-1) * a
}

void update_inter_mul ( Op *res, Op *a, Op *b ) {
    if ( not res->cpt_use ) {
        if ( a->beg_value_valid and a->end_value_valid and b->beg_value_valid and b->end_value_valid ) {
            Ex::T v0 = a->beg_value * b->beg_value;
            bool i0 = a->beg_value_inclusive and b->beg_value_inclusive;
            Ex::T v1 = a->beg_value * b->end_value;
            bool i1 = a->beg_value_inclusive and b->end_value_inclusive;
            Ex::T v2 = a->end_value * b->beg_value;
            bool i2 = a->end_value_inclusive and b->beg_value_inclusive;
            Ex::T v3 = a->end_value * b->end_value;
            bool i3 = a->end_value_inclusive and b->end_value_inclusive;
            // min
            Ex::T miv = v0;
            bool mii = i0;
            if ( v1 < miv or ( v1==miv and i1 ) ) {
                miv = v1;
                mii = i1;
            }
            if ( v2 < miv or ( v2==miv and i2 ) ) {
                miv = v2;
                mii = i2;
            }
            if ( v3 < miv or ( v3==miv and i3 ) ) {
                miv = v3;
                mii = i3;
            }
            res->set_beg_value ( miv, mii );
            // max
            Ex::T mav = v0;
            bool mai = i0;
            if ( v1 > mav or ( v1==mav and i1 ) ) {
                mav = v1;
                mai = i1;
            }
            if ( v2 > mav or ( v2==mav and i2 ) ) {
                mav = v2;
                mai = i2;
            }
            if ( v3 > mav or ( v3==mav and i3 ) ) {
                mav = v3;
                mai = i3;
            }
            res->set_end_value ( mav, mai );
        } else if ( a->type == Op::NUMBER ) {
            if ( a->number_data()->val.is_pos() ) {
                if ( b->beg_value_valid )
                    res->set_beg_value ( a->number_data()->val * b->beg_value, b->beg_value_inclusive );
                if ( b->end_value_valid )
                    res->set_end_value ( a->number_data()->val * b->end_value, b->end_value_inclusive );
            } else {
                if ( b->end_value_valid )
                    res->set_beg_value ( a->number_data()->val * b->end_value, b->end_value_inclusive );
                if ( b->beg_value_valid )
                    res->set_end_value ( a->number_data()->val * b->beg_value, b->beg_value_inclusive );
            }
        }
    }
}

Ex mul_number_and_expr ( Ex a, Ex b ) { // a is a number
    if ( a.op->number_data()->val.is_one() )
        return b;
    if ( a.op->number_data()->val.is_zero() )
        return a;
    // 10 * ( 5 * a )
    if ( b.op->type == STRING_mul_NUM and b.op->func_data()->children[0]->type == Op::NUMBER ) {
        Ex new_num = a.op->number_data()->val * b.op->func_data()->children[0]->number_data()->val;
        return mul_number_and_expr ( new_num, b.op->func_data()->children[1] );
    }
    // 10 * ( 3 * a + 5 * b ) -> 30 * a + 50 * b ( but 10*(a+5*b) is not transformed )
    if ( b.op->type == STRING_add_NUM ) {
        SplittedVec<SumSeq,4,16,true> items;
        find_add_items_and_coeff_rec ( b.op->func_data()->children[0], items );
        find_add_items_and_coeff_rec ( b.op->func_data()->children[1], items );
        bool has_one = false;
        for ( unsigned i=0;i<items.size();++i )
            has_one |= ( items[i].c.is_one() or items[i].c.is_minus_one() );
        #ifdef ALWAYS_DISTRIBUTE_NUMBER
        has_one = false;
        #endif
        #ifdef NEVER_DISTRIBUTE_NUMBER
        has_one = true;
        #endif
        if ( not has_one ) {
            for ( unsigned i=0;i<items.size();++i )
                items[i].c *= a.op->number_data()->val;
            return make_add_seq ( items );
        }
    }

    //
    Op *res = Op::new_function ( STRING_mul_NUM, a.op, b.op );
    update_inter_mul ( res, a.op, b.op );
    return res;
}

Ex to_op ( const MulSeq &ms ) {
    if ( ms.e.is_one() )
        return ms.op;
    return pow ( Ex ( ms.op ), Ex ( ms.e ) );
}

Ex make_mul_seq ( SplittedVec<MulSeq,4,16,true> &items_c ) {
    if ( not items_c.size() )
        return 1;
    //
    Ex res = 1;
    Rationnal n = 1;
    for ( unsigned i=0;i<items_c.size();++i ) {
        if ( items_c[i].op->type == Op::NUMBER )
            n *= to_op ( items_c[i] ).op->number_data()->val;
        else
            res = res * to_op ( items_c[i] );
    }
    if ( n.is_one() )
        return res;
    return Ex ( n ) * res;
}

/// @see operator*
void mul_items_and_coeffs ( const SplittedVec<MulSeq,4,16,true> &items_a, SplittedVec<MulSeq,4,16,true> &items_b ) {
    for ( unsigned i=0;i<items_a.size();++i ) {
        for ( unsigned j=0;;++j ) {
            if ( j == items_b.size() ) {
                items_b.push_back ( items_a[i] );
                break;
            }
            if ( items_a[ i ].op == items_b[ j ].op ) {
                items_b[ j ].e += items_a[ i ].e;
                if ( not items_b[ j ].e )
                    items_b.erase_elem_nb_in_unordered_list ( j );
                break;
            }
        }
    }
}

Ex operator* ( const Ex &a_, const Ex &b_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    // 10 * ...
    if ( a.op->type == Op::NUMBER ) {
        // 10 * 20
        if ( b.op->type == Op::NUMBER )
            return Op::new_number ( a.op->number_data()->val * b.op->number_data()->val );
        // 10 * ( ... )
        return mul_number_and_expr ( a, b );
    }
    // ( not a number ) * 10 -> 10 * ( not a number )
    if ( b.op->type == Op::NUMBER )
        return mul_number_and_expr ( b, a );

    // ( 10 * a ) * ( not a number ) -> 10 * ( a * ( not a number ) )
    if ( a.op->type == STRING_mul_NUM and a.op->func_data()->children[0]->type == Op::NUMBER ) {
        // ( 10 * a ) * ( 20 * b ) -> 30 * ( a * b )
        if ( b.op->type == STRING_mul_NUM and b.op->func_data()->children[0]->type == Op::NUMBER ) {
            Ex new_num = a.op->func_data()->children[0]->number_data()->val * b.op->func_data()->children[0]->number_data()->val;
            Ex new_mul = Ex ( a.op->func_data()->children[1] ) * Ex ( b.op->func_data()->children[1] );
            return new_num * new_mul;
        }
        // ( 10 * a ) * b -> 10 * ( a * b )
        Ex new_mul = Ex ( a.op->func_data()->children[1] ) * b;
        return Ex ( a.op->func_data()->children[0] ) * new_mul;
    }
    // a * ( 10 * b ) -> 10 * ( a * b )
    if ( b.op->type == STRING_mul_NUM and b.op->func_data()->children[0]->type == Op::NUMBER ) {
        Ex new_mul = a * Ex ( b.op->func_data()->children[1] );
        return Ex ( b.op->func_data()->children[0] ) * new_mul;
    }

    // ( a^10 * b * anything_but_a_number ) * ( a * anything_but_a_number )
    SplittedVec<MulSeq,4,16,true> items_a, items_b;
    find_mul_items_and_coeff_rec ( a.op, items_a );
    find_mul_items_and_coeff_rec ( b.op, items_b );
    unsigned old_size = items_a.size() + items_b.size();
    mul_items_and_coeffs ( items_b, items_a );
    if ( old_size != items_a.size() )
        return make_mul_seq ( items_a );

    //
    Op *res;
    if ( a.op < b.op )
        res = Op::new_function ( STRING_mul_NUM, a.op, b.op );
    else
        res = Op::new_function ( STRING_mul_NUM, b.op, a.op );

    // interval
    update_inter_mul ( res, a.op, b.op );
    return res;
}
Ex inv ( const Ex &a ) {
    return pow ( a, Ex ( -1 ) );
}

Ex operator/ ( const Ex &a, const Ex &b ) {
    return a * inv ( b );
}

bool need_abs_for_pow ( Op *op, const Rationnal &e0, const Rationnal &nn ) {
    if ( op->necessary_positive_or_null() )
        return false;
    if ( not e0.is_even() ) // (a^3)^... or (a^0.5)^... -> non need
        return false;
    if ( nn.is_even() ) // a^(2) -> abs is not useful
        return false;
    // (a^6)^0.5 -> a^3
    return true;
}

void update_inter_pow ( Op *res, Op *a, Op *b ) {
    if ( not res->cpt_use ) {
        if ( b->type == Op::NUMBER ) {
            Unsigned32 p ( b->number_data()->val );
            if ( b->number_data()->val.is_even() )
                res->set_beg_value ( 0, true );
            if ( b->number_data()->val.is_even() and b->number_data()->val.is_pos() ) { // ^2, ^4, ...
                if ( a->beg_value_valid and a->end_value_valid ) {
                    if ( a->beg_value.is_neg_or_null() and a->end_value.is_pos_or_null() ) { // +-----0-----+
                        res->set_beg_value ( 0, true );
                        Ex::T b2 = pow ( a->beg_value, p );
                        Ex::T e2 = pow ( a->end_value, p );
                        //
                        if ( b2 == e2 )
                            res->set_end_value ( b2, a->beg_value_inclusive and a->end_value_inclusive );
                        else if ( b2 > e2 )
                            res->set_end_value ( b2, a->beg_value_inclusive );
                        else
                            res->set_end_value ( e2, a->end_value_inclusive );
                    } else if ( a->beg_value.is_pos_or_null() and a->end_value.is_pos_or_null() ) { // 0  +-------+
                        res->set_beg_value ( pow ( a->beg_value, p ), a->beg_value_inclusive );
                        res->set_end_value ( pow ( a->end_value, p ), a->end_value_inclusive );
                    } else if ( a->beg_value.is_neg_or_null() and a->end_value.is_neg_or_null() ) { // +-------+  0
                        res->set_beg_value ( pow ( a->end_value, p ), a->end_value_inclusive );
                        res->set_end_value ( pow ( a->beg_value, p ), a->beg_value_inclusive );
                    }
                }
            } else if ( b->number_data()->val.is_pos() and not b->number_data()->val.is_integer() ) { // ^0.5, ^1.3, ... -> should be monotonic
                if ( a->beg_value_valid and a->beg_value.is_pos_or_null() )
                    res->set_beg_value ( pow_96 ( a->beg_value, p ), a->beg_value_inclusive );
                if ( a->end_value_valid and a->end_value.is_pos_or_null() )
                    res->set_end_value ( pow_96 ( a->end_value, p ), a->end_value_inclusive );
            }
        }
        //         if ( a->beg_value_valid and a->end_value_valid and b->beg_value_valid and b->end_value_valid ) {
        //             Ex::T v0 = pow_96( a->beg_value, b->beg_value ); bool i0 = a->beg_value_inclusive and b->beg_value_inclusive;
        //             Ex::T v1 = a->beg_value * b->end_value; bool i1 = a->beg_value_inclusive and b->end_value_inclusive;
        //             Ex::T v2 = a->end_value * b->beg_value; bool i2 = a->end_value_inclusive and b->beg_value_inclusive;
        //             Ex::T v3 = a->end_value * b->end_value; bool i3 = a->end_value_inclusive and b->end_value_inclusive;
        //             // min
        //             Ex::T miv = v0; bool mii = i0;
        //             if ( v1 < miv or ( v1==miv and i1 ) ) { miv = v1; mii = i1; }
        //             if ( v2 < miv or ( v2==miv and i2 ) ) { miv = v2; mii = i2; }
        //             if ( v3 < miv or ( v3==miv and i3 ) ) { miv = v3; mii = i3; }
        //             res->set_beg_value( miv, mii );
        //             // max
        //             Ex::T mav = v0; bool mai = i0;
        //             if ( v1 > mav or ( v1==mav and i1 ) ) { mav = v1; mai = i1; }
        //             if ( v2 > mav or ( v2==mav and i2 ) ) { mav = v2; mai = i2; }
        //             if ( v3 > mav or ( v3==mav and i3 ) ) { mav = v3; mai = i3; }
        //             res->set_end_value( mav, mai );
        //         }
    }
}

Ex pow ( const Ex &a_, const Ex &b_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    if ( b.op->type == Op::NUMBER ) { // a ^ 10
        if ( a.op->type == Op::NUMBER ) // 10 ^ 32
            return Op::new_number ( pow_96 ( a.op->number_data()->val, b.op->number_data()->val ) );
        if ( b.op->is_zero() )
            return 1;
        if ( b.op->is_one() )
            return a;
        if ( a.op->type == STRING_abs_NUM ) { // abs(a) ^ n
            if ( b.op->number_data()->val.is_even() ) // abs(a) ^ 4
                return pow ( Ex ( a.op->func_data()->children[0] ), b );
        }
        // ( a * b^2 ) ^ 3 -> a^3 * b^6
        if ( a.op->type == STRING_mul_NUM and b.op->number_data()->val.is_integer() ) {
            SplittedVec<MulSeq,4,16,true> items;
            find_mul_items_and_coeff_rec ( a.op->func_data()->children[0], items );
            find_mul_items_and_coeff_rec ( a.op->func_data()->children[1], items );
            //
            for ( unsigned i=0;i<items.size();++i )
                items[i].e *= b.op->number_data()->val;
            return make_mul_seq ( items );

        }
        // ( a^2 ) ^ 3 -> a^6
        if ( a.op->type == STRING_pow_NUM and a.op->func_data()->children[1]->type == Op::NUMBER ) {
            Rationnal e0 = a.op->func_data()->children[1]->number_data()->val;
            Rationnal nn = e0 * b.op->number_data()->val;
            // (a^0.5)^3 -> a^1.5
            // (a^2)^0.5 -> abs(a)
            if ( need_abs_for_pow ( a.op->func_data()->children[0], e0, nn ) ) {
                if ( nn.is_one() )
                    return abs ( Ex ( a.op->func_data()->children[0] ) );
                return pow ( abs ( Ex ( a.op->func_data()->children[0] ) ), Ex ( nn ) );
            } else {
                if ( nn.is_one() )
                    return a.op->func_data()->children[0]->inc_ref();
                return pow ( Ex ( a.op->func_data()->children[0] ), Ex ( nn ) );
            }
        }
    }
    //
    Op *res = Op::new_function ( STRING_pow_NUM, a.op, b.op );
    update_inter_pow ( res, a_.op, b_.op );
    //
    return res;
}

Ex mod ( const Ex &a_, const Ex &b_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    if ( are_numbers ( a.op, b.op ) ) return mod ( a.op->number_data()->val, b.op->number_data()->val );
    if ( a.op->is_zero() ) return a;
    return Op::new_function ( STRING_mod_NUM, a.op, b.op );
}

Ex atan2 ( const Ex &a_, const Ex &b_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    if ( are_numbers ( a.op, b.op ) ) return atan2_96 ( a.op->number_data()->val, b.op->number_data()->val );
    return Op::new_function ( STRING_atan2_NUM, a.op, b.op );
}

// function pour enrichissement 
// 3D (x,y,z,id_enr,dir)
Ex Fun_enr_3D ( const Ex &a_, const Ex &b_, const Ex &c_ , const Ex &d , const Ex &e , const Ex &f) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );
    Ex c = a_posteriori_simplification ( c_ );
    
    return Op::new_function ( STRING_Fun_enr_3D_NUM, a.op, b.op, c.op, d.op, e.op, f.op);
}

Ex Fun_enr_3D_dx ( const Ex &a_, const Ex &b_, const Ex &c_ , const Ex &d, const Ex &e, const Ex &f) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );
    Ex c = a_posteriori_simplification ( c_ );

    return Op::new_function ( STRING_Fun_enr_3D_dx_NUM, a.op, b.op, c.op, d.op, e.op, f.op);
}

Ex Fun_enr_3D_dy ( const Ex &a_, const Ex &b_, const Ex &c_ , const Ex &d, const Ex &e, const Ex &f) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );
    Ex c = a_posteriori_simplification ( c_ );

    return Op::new_function ( STRING_Fun_enr_3D_dy_NUM, a.op, b.op, c.op, d.op, e.op, f.op );
}

Ex Fun_enr_3D_dz ( const Ex &a_, const Ex &b_, const Ex &c_ , const Ex &d, const Ex &e, const Ex &f) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );
    Ex c = a_posteriori_simplification ( c_ );

    return Op::new_function ( STRING_Fun_enr_3D_dz_NUM, a.op, b.op, c.op, d.op, e.op, f.op );
}

Ex Young_enr_3D ( const Ex &a_, const Ex &b_, const Ex &c_ , const Ex &d, const Ex &e, const Ex &f) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );
    Ex c = a_posteriori_simplification ( c_ );

    return Op::new_function ( STRING_Young_enr_3D_NUM, a.op, b.op, c.op, d.op, e.op, f.op );
}

Ex Poisson_enr_3D ( const Ex &a_, const Ex &b_, const Ex &c_ , const Ex &d, const Ex &e, const Ex &f) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );
    Ex c = a_posteriori_simplification ( c_ );

    return Op::new_function ( STRING_Poisson_enr_3D_NUM, a.op, b.op, c.op, d.op, e.op, f.op );
}
// 2D (x,y,id_enr,dir)
Ex Fun_enr ( const Ex &a_, const Ex &b_, const Ex &c , const Ex &d, const Ex &e ) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    return Op::new_function ( STRING_Fun_enr_NUM, a.op, b.op, c.op, d.op, e.op );
}

Ex Fun_enr_dx( const Ex &a_, const Ex &b_, const Ex &c , const Ex &d, const Ex &e) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    return Op::new_function ( STRING_Fun_enr_dx_NUM, a.op, b.op, c.op, d.op, e.op );
}

Ex Fun_enr_dy ( const Ex &a_, const Ex &b_, const Ex &c , const Ex &d, const Ex &e) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    return Op::new_function ( STRING_Fun_enr_dy_NUM, a.op, b.op, c.op, d.op, e.op );
}

Ex Young_enr ( const Ex &a_, const Ex &b_, const Ex &c , const Ex &d, const Ex &e) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    return Op::new_function ( STRING_Young_enr_NUM, a.op, b.op, c.op, d.op, e.op );
}

Ex Poisson_enr ( const Ex &a_, const Ex &b_, const Ex &c , const Ex &d, const Ex &e ) {
    Ex a = a_posteriori_simplification ( a_ );
    Ex b = a_posteriori_simplification ( b_ );

    return Op::new_function ( STRING_Poisson_enr_NUM, a.op, b.op, c.op, d.op, e.op );
}

Ex abs ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return abs ( a.op->number_data()->val );
    if ( a.op->necessary_positive_or_null() ) return a;
    if ( a.op->type == STRING_mul_NUM ) { // abs( c0 * c1 )
        Op *c0 = a.op->func_data()->children[0];
        Op *c1 = a.op->func_data()->children[1];
        if ( c0->necessary_positive_or_null() )
            return Ex ( c0 ) * abs ( Ex ( c1 ) );
        if ( c1->necessary_positive_or_null() )
            return abs ( Ex ( c0 ) ) * Ex ( c1 );
        if ( c0->necessary_negative() )
            return Ex ( -1 ) * Ex ( c0 ) * abs ( Ex ( c1 ) );
        if ( c1->necessary_negative() )
            return abs ( Ex ( c0 ) ) * Ex ( -1 ) * Ex ( c1 );
    }
    Op *res = Op::new_function ( STRING_abs_NUM, a.op );
    if ( not res->cpt_use )
        res->set_beg_value ( 0, true );
    return res;
}
Ex log ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return log_96 ( a.op->number_data()->val );
    if ( a.op->type == STRING_exp_NUM ) return a.op->func_data()->children[0];
    return Op::new_function ( STRING_log_NUM, a.op );
}
Ex heaviside ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return heaviside ( a.op->number_data()->val );
    if ( a.op->necessary_positive_or_null() ) return 1;
    if ( a.op->necessary_negative() ) return 0;
    if ( a.op->type == STRING_mul_NUM ) { // heaviside( c0 * c1 )
        Op *c0 = a.op->func_data()->children[0];
        Op *c1 = a.op->func_data()->children[1];
        if ( c0->necessary_positive() ) return heaviside ( Ex ( c1 ) );
        if ( c1->necessary_positive() ) return heaviside ( Ex ( c0 ) );
        //if ( c0->necessary_negative() ) return Ex ( 1 ) - heaviside( Ex( c1 ) ) + eqz( Ex( c1 ) );
        //if ( c1->necessary_negative() ) return Ex ( 1 ) - heaviside( Ex( c0 ) ) + eqz( Ex( c0 ) );
        if ( c0->necessary_negative() and not c0->is_minus_one() ) return heaviside ( Ex ( -1 ) * Ex ( c1 ) );
        if ( c1->necessary_negative() and not c1->is_minus_one() ) return heaviside ( Ex ( -1 ) * Ex ( c0 ) );
    }
    Op *res = Op::new_function ( STRING_heaviside_NUM, a.op );
    if ( not res->cpt_use ) {
        res->set_beg_value ( 0, true );
        res->set_end_value ( 1, true );
    }
    return res;
}
Ex pos_part ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return pos_part ( a.op->number_data()->val );
    if ( a.op->necessary_positive_or_null() ) return a;
    if ( a.op->necessary_negative() ) return 0;
    Op *res = Op::new_function ( STRING_pos_part_NUM, a.op );
    if ( not res->cpt_use ) {
        if ( a_.op->end_value_valid )
            res->set_end_value ( a_.op->end_value, a_.op->end_value_inclusive );
        res->set_beg_value ( 0, true );
    }
    return res;
}
Ex eqz ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return eqz ( a.op->number_data()->val );
    if ( a.op->necessary_positive() ) return 0;
    if ( a.op->necessary_negative() ) return 0;
    if ( a.op->type == STRING_mul_NUM ) {
        if ( a.op->func_data()->children[0]->type == Op::NUMBER )
            return eqz ( Ex ( a.op->func_data()->children[1] ) );
        Op *c0 = a.op->func_data()->children[0];
        Op *c1 = a.op->func_data()->children[1];
        if ( c0->necessary_positive() or c0->necessary_negative() )
            return eqz ( Ex ( c1 ) );
        if ( c1->necessary_positive() or c1->necessary_negative() )
            return eqz ( Ex ( c0 ) );
    }
    if ( a.op->type == STRING_pow_NUM ) { // eqz( m ^ e )
        Op *m = a.op->func_data()->children[0];
        Op *e = a.op->func_data()->children[1];
        if ( e->necessary_positive() ) // m ^ 5
            return eqz ( Ex ( m ) );
        if ( e->necessary_negative() ) // m ^ -5
            return Ex ( 0 ); // -> can't be == 0
    } else if ( a.op->type == STRING_log_NUM )
        return eqz ( Ex ( a.op->func_data()->children[0] ) - 1 );
    else if ( a.op->type == STRING_abs_NUM or a.op->type == STRING_sinh_NUM or a.op->type == STRING_tanh_NUM )
        return eqz ( Ex ( a.op->func_data()->children[0] ) );
    else if ( a.op->type == STRING_eqz_NUM or a.op->type == STRING_heaviside_NUM )
        return 1 - a;
    // no simplifications
    Op *res = Op::new_function ( STRING_eqz_NUM, a.op );
    if ( not res->cpt_use ) {
        res->set_beg_value ( 0, true );
        res->set_end_value ( 1, true );
    }
    return res;
}
Ex exp ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return exp_96 ( a.op->number_data()->val );
    if ( a.op->type == STRING_log_NUM ) return a.op->func_data()->children[0];
    Op *res = Op::new_function ( STRING_exp_NUM, a.op );
    if ( not res->cpt_use )
        res->set_beg_value ( 0, false );
    return res;
}
Ex sin ( Ex a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return sin_96 ( a.op->number_data()->val );
    a = a_posteriori_simplification ( a );
    //
    if ( a.op->type == STRING_mul_NUM ) {
        Op *c0 = a.op->func_data()->children[0];
        Op *c1 = a.op->func_data()->children[1];
        if ( c0->is_minus_one() )
            return Ex ( -1 ) * sin ( Ex ( c1 ) );
    }
    if ( a.op->necessary_negative() ) return Ex ( -1 ) * sin ( Ex ( -1 ) * Ex ( a.op ) );
    Op *res = Op::new_function ( STRING_sin_NUM, a.op );
    if ( not res->cpt_use ) {
        res->set_beg_value ( -1, true );
        res->set_end_value ( 1, true );
    }
    return res;
}
Ex cos ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );

    if ( is_a_number ( a.op ) ) return cos_96 ( a.op->number_data()->val );
    if ( a.op->type == STRING_mul_NUM ) {
        Op *c0 = a.op->func_data()->children[0];
        Op *c1 = a.op->func_data()->children[1];
        if ( c0->is_minus_one() )
            return cos ( Ex ( c1 ) );
    }
    if ( a.op->necessary_negative() ) return cos ( Ex ( -1 ) * Ex ( a.op ) );
    Op *res = Op::new_function ( STRING_cos_NUM, a.op );
    if ( not res->cpt_use ) {
        res->set_beg_value ( -1, true );
        res->set_end_value ( 1, true );
    }
    return res;
}
Ex tan ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return tan_96 ( a.op->number_data()->val );
    if ( a.op->type == STRING_mul_NUM ) {
        Op *c0 = a.op->func_data()->children[0];
        Op *c1 = a.op->func_data()->children[1];
        if ( c0->is_minus_one() )
            return Ex ( -1 ) * tan ( Ex ( c1 ) );
    }
    if ( a.op->necessary_negative() ) return Ex ( -1 ) * tan ( Ex ( -1 ) * Ex ( a.op ) );
    return Op::new_function ( STRING_tan_NUM, a.op );
}
Ex asin ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return asin_96 ( a.op->number_data()->val );
    return Op::new_function ( STRING_asin_NUM, a.op );
}
Ex acos ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return acos_96 ( a.op->number_data()->val );
    return Op::new_function ( STRING_acos_NUM, a.op );
}
Ex atan ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return atan_96 ( a.op->number_data()->val );
    return Op::new_function ( STRING_atan_NUM, a.op );
}
Ex sinh ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return sinh_96 ( a.op->number_data()->val );
    return Op::new_function ( STRING_sinh_NUM, a.op );
}
Ex cosh ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return cosh_96 ( a.op->number_data()->val );
    Op *res = Op::new_function ( STRING_cosh_NUM, a.op );
    if ( not res->cpt_use )
        res->set_beg_value ( 1, true );
    return res;
}
Ex tanh ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return tanh_96 ( a.op->number_data()->val );
    return Op::new_function ( STRING_tanh_NUM, a.op );
}

Ex floor ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return floor ( a.op->number_data()->val );
    Op *res = Op::new_function ( STRING_floor_NUM, a.op );
    res->integer_type = 1;
    return res;
}

Ex ceil ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return ceil ( a.op->number_data()->val );
    Op *res = Op::new_function ( STRING_ceil_NUM, a.op );
    res->integer_type = 1;
    return res;
}

Ex round ( const Ex &a_ ) {
    Ex a = a_posteriori_simplification ( a_ );
    if ( is_a_number ( a.op ) ) return round ( a.op->number_data()->val );
    Op *res = Op::new_function ( STRING_round_NUM, a.op );
    res->integer_type = 1;
    return res;
}

Ex min ( const Ex &a, const Ex &b ) {
    return b - pos_part ( b - a );
    //     Ex s = heaviside( b - a );
    //     return s * a + ( 1 - s ) * b;
}

Ex max ( const Ex &a, const Ex &b ) {
    return a + pos_part ( b - a );
    //     Ex s = heaviside( a - b );
    //     return s * a + ( 1 - s ) * b;
}
Ex min ( const Ex &a, const Ex &b, const Ex &c ) {
    return min ( min ( a, b ), c );
}
Ex max ( const Ex &a, const Ex &b, const Ex &c ) {
    return max ( max ( a, b ), c );
}

void sort ( Ex &a, Ex &b, Ex &c ) {
    Ex t = a;
    a = min ( b,t );
    b = max ( b,t );

    t = b;
    b = min ( c,t );
    c = max ( c,t );

    t = a;
    a = min ( b,t );
    b = max ( b,t );
}

Ex select_symbolic ( const Ex &vec_, const Ex &index_ ) {
    Ex vec   = a_posteriori_simplification ( vec_ );
    Ex index = a_posteriori_simplification ( index_ );
    return Op::new_function ( STRING_select_symbolic_NUM, vec.op, index.op );
}


// ------------------------------------------------------------------------------------------------------------
Ex roots ( Thread *th, const void *tok, const Ex &f, const Ex &x ) {
    Op *res = Op::new_function ( STRING_roots_NUM, f.op, x.op );

    //
    int deg = f.poly_deg ( x );
    if ( deg >= 0 ) {
        SEX coeff, expressions;
        expressions.push_back ( f );
        coeff.get_room_for ( deg + 1 );
        polynomial_expansion ( th, tok, expressions, x, deg, coeff );
        for ( unsigned i=0;i<coeff.size();++i )
            res->chroot.push_back ( coeff[i].op->inc_ref() );
    }

    //
    return res;
}

Ex root ( Thread *th, const void *tok, const Ex &r, int n ) {
    if ( r.op->type != STRING_roots_NUM ) {
        th->add_error ( "root must take a roots(...) as first argument.", tok );
        return 0;
    }

    //
    if ( r.op->chroot.size() == 1 ) {
        return 0;
    } else if ( r.op->chroot.size() == 2 ) {
        return - Ex ( r.op->chroot[0] ) / Ex ( r.op->chroot[1] );
    } else if ( r.op->chroot.size() == 3 ) {
        Ex a = r.op->chroot[2], b = r.op->chroot[1], c = r.op->chroot[0];
        Ex delta = pow ( b, 2 ) - 4 * a * c;
        Ex ea = eqz ( a ), eb = eqz ( b ), hd = heaviside ( delta );
        Ex sq_delta = pow ( delta * hd, Rationnal ( 1, 2 ) );
        if ( n == 0 )
            return - ( ( 1 - ea ) * ( b + sq_delta ) + ea * c ) / ( 2 * a + ea * ( b + eb ) ); // first root
        else
            return - ( b - sq_delta ) / ( 2 * a + ea );                                        // second one
        //validity[ 0 ] = 1 - ea * eb;
        //validity[ 1 ] = ( 1 - ea ) * hd;
    } else if ( r.op->chroot.size() == 4 ) {
        Ex z = r.op->chroot[3], ez = eqz ( z );
        Ex a = r.op->chroot[2] / ( z + ez ), b = r.op->chroot[1] / ( z + ez ), c = r.op->chroot[0] / ( z + ez );

        Ex p = 2 * pow ( a,3 ) - 9 * a * b + 27 * c;
        Ex delta = pow ( p, 2 ) + 4 * ( 3 * b - a * a );

        using namespace std;

        // delta < 0
        complex<Ex> u = ( - p + sqrtc ( delta ) ) / Ex ( 54 );
        complex<Ex> v = ( - p - sqrtc ( delta ) ) / Ex ( 54 );
        complex<Ex> j1 ( Rationnal ( 1,2 ), sqrt_96 ( Rationnal ( 3,4 ) ) );
        complex<Ex> j2 ( -Rationnal ( 1,2 ), sqrt_96 ( Rationnal ( 3,4 ) ) );


        complex<Ex> pu = powc ( u, Rationnal ( 1,3 ) );
        complex<Ex> pv = powc ( v, Rationnal ( 1,3 ) );

        std::cout << -a / 3 +      pu +      pv << std::endl;
        std::cout << -a / 3 - j1 * pu + j2 * pv << std::endl;
        std::cout << -a / 3 + j2 * pu - j1 * pv << std::endl;

        if ( n == 0 )
            return real ( complex<Ex> ( -a/3 ) + powc ( u, Rationnal ( 1,3 ) ) + powc ( v, Rationnal ( 1,3 ) ) );
        if ( n == 1 )
            return real ( complex<Ex> ( -a/3 ) - j1 * powc ( u, Rationnal ( 1,3 ) ) + j2 * powc ( v, Rationnal ( 1,3 ) ) );
        if ( n == 2 )
            return real ( complex<Ex> ( -a/3 ) + j2 * powc ( u, Rationnal ( 1,3 ) ) - j1 * powc ( v, Rationnal ( 1,3 ) ) );

//         Ex delta = -2*pow(a,3)+9*a*b-27*c+
//         std::cout << a << " " << b << " " << c << " " << z << std::endl;
//         Ex p = b - pow(a,2) / 3;
//         Ex q = pow(a,3) * Rationnal(2,27) - a * b / 3 + c;
//         Ex delta = 4 * pow(p,3) + 27 * pow(q,2);
//         std::cout << delta << std::endl;
//         //delta >= 0
//         //         Ex u = (-27*q+sqrt(27*delta))/2;
//         //         Ex v = (-27*q-sqrt(27*delta))/2;
//         //         res += heaviside( delta ) * ExVector(
//         //             sgn(u)*pow(abs(u),1.0/3.0)+sgn(v)*pow(abs(v),1.0/3.0)-a)/3.0
//         //         );
//         //delta < 0
//         std::complex<Ex> j( -Rationnal(1,2), sqrt_96(Rationnal(3,4)) );
//         std::complex<Ex> v( -27 * q / 2, 0 );
//         v += sqrtc( - Rationnal(27,4) * delta ) * std::complex<Ex>( 0, 1 );
//         std::complex<Ex> u( powc( v, Rationnal(1,3) ) );
//         Ex x0 = ( 2 * std::real(    u) - a ) / 3;
//         Ex x1 = ( 2 * std::real(  j*u) - a ) / 3;
//         Ex x2 = ( 2 * std::real(j*j*u) - a ) / 3;
        //
//         //Ex va = 1 - ez;
//         if ( n == 0 )
//             return x0;
//         //validity[ 0 ] = ( 1 - va ) * validity[ 0 ] + va;
//         //va *= 1 - heaviside( delta );
//         if ( n == 1 )
//             return x1;
//         if ( n == 2 )
//             return x2;
//         //validity[ 1 ] = ( 1 - va ) * validity[ 1 ] + va;
//         //validity[ 2 ] = ( 1 - va ) * validity[ 2 ] + va;
    }

    //
    return Op::new_function ( STRING_root_NUM, Op::new_number ( n ), r.op );
}

// ------------------------------------------------------------------------------------------------------------
void destroy_rec ( Op *a ) {
    if ( a->op_id != Op::current_op ) { // if not done ?
        a->op_id = Op::current_op;
        dec_ref ( a->additional_info );
        if ( a->type > 0 ) {
            if ( a->func_data()->children[0] )
                destroy_rec ( a->func_data()->children[0] );
            if ( a->func_data()->children[1] )
                destroy_rec ( a->func_data()->children[1] );
        }
    }
}


// ------------------------------------------------------------------------------------------------------------
struct DiffRec {
DiffRec ( Thread *th, const void *tok, const Ex &a ) : th ( th ), tok ( tok ), zero ( 0 ), one ( 1 ) {
        a.op->additional_info = one.op->inc_ref();
        a.op->op_id           = ++Op::current_op;
    }


    ~DiffRec() {
        //         ++Op::current_op;
        //         destroy_rec( expr.op );
        //         if ( d.op->op_id != Op::current_op )
        //             dec_ref( d.op->additional_info );
    }


    void diff_rec ( Op *a ) {
        if ( a->op_id == Op::current_op ) // already done ?
            return;
        a->op_id = Op::current_op;
        //
        #define MAKE_D0( res ) \
        { \
            Op *c0 = a->func_data()->children[0]; diff_rec( c0 ); \
            Op *d0 = c0->additional_info; \
            a->additional_info = to_inc_op( res ); \
        }
        #define MAKE_D0D1( res ) \
        { \
            Op *c0 = a->func_data()->children[0]; Op *c1 = a->func_data()->children[1]; diff_rec( c0 ); diff_rec( c1 ); \
            Op *d0 = c0->additional_info; Op *d1 = c1->additional_info; \
            a->additional_info = to_inc_op( res ); \
        }
        #define MAKE_D0D1_name( res ) \
        { \
        Op *c0 = a->func_data()->children[0]; Op *c1 = a->func_data()->children[1]; Op *c2 = a->func_data()->children[2]; Op *c3 = a->func_data()->children[3]; Op *c4 = a->func_data()->children[4]; diff_rec( c0 ); diff_rec( c1 ); \
        Op *d0 = c0->additional_info; Op *d1 = c1->additional_info; \
        a->additional_info = to_inc_op( res ); \
        }
        #define MAKE_D0D1D2_name( res ) \
        { \
        Op *c0 = a->func_data()->children[0]; Op *c1 = a->func_data()->children[1]; Op *c2 = a->func_data()->children[2]; Op *c3 = a->func_data()->children[3]; Op *c4 = a->func_data()->children[4]; Op *c5 = a->func_data()->children[5]; diff_rec( c0 ); diff_rec( c1 ); diff_rec( c2 ); \
        Op *d0 = c0->additional_info; Op *d1 = c1->additional_info; Op *d2 = c2->additional_info; \
        a->additional_info = to_inc_op( res ); \
        }
        switch ( a->type ) {
        case Op::NUMBER:
            a->additional_info = zero.op->inc_ref();
            break;
        case Op::SYMBOL:
            a->additional_info = zero.op->inc_ref();
            break;
        case STRING_heaviside_NUM:
            a->additional_info = zero.op->inc_ref();
            break;
        case STRING_eqz_NUM:
            a->additional_info = zero.op->inc_ref();
            break;
        case STRING_add_NUM:
            MAKE_D0D1 ( Ex ( d0 ) + Ex ( d1 ) );
            break;
        case STRING_mul_NUM:
            MAKE_D0D1 ( Ex ( d0 ) * Ex ( c1 ) + Ex ( c0 ) * Ex ( d1 ) );
            break;
        case STRING_log_NUM:
            MAKE_D0 ( Ex ( d0 ) / c0 );
            break; // { GET_D0; Op &n = o / c0; Op &r = da * n; of.push_back(&n); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_abs_NUM:
            MAKE_D0 ( ( 2 * heaviside ( Ex ( c0 ) ) - 1 ) * Ex ( d0 ) );
            break;
        case STRING_pos_part_NUM:
            MAKE_D0 ( heaviside ( Ex ( c0 ) ) * Ex ( d0 ) );
            break;
        case STRING_exp_NUM:
            MAKE_D0 ( Ex ( d0 ) * Ex ( a ) );
            break;
        case STRING_floor_NUM:
            a->additional_info = zero.op->inc_ref();
            break;
        case STRING_ceil_NUM:
            a->additional_info = zero.op->inc_ref();
            break;
        case STRING_round_NUM:
            a->additional_info = zero.op->inc_ref();
            break;
        case STRING_select_symbolic_NUM:
            a->additional_info = zero.op->inc_ref();
            break;

        case STRING_sin_NUM:
            MAKE_D0 ( Ex ( d0 ) * cos ( Ex ( c0 ) ) );
            break;
        case STRING_cos_NUM:
            MAKE_D0 ( - Ex ( d0 ) * sin ( Ex ( c0 ) ) );
            break;
        case STRING_tan_NUM:
            MAKE_D0 ( Ex ( d0 ) * ( 1 + pow ( Ex ( a ), 2 ) ) );
            break;
        case STRING_asin_NUM:
            MAKE_D0 ( Ex ( d0 ) / pow ( 1 - pow ( Ex ( c0 ), 2 ), Rationnal ( 1,2 ) ) );
            break;
        case STRING_acos_NUM:
            MAKE_D0 ( - Ex ( d0 ) / pow ( 1 - pow ( Ex ( c0 ), 2 ), Rationnal ( 1,2 ) ) );
            break;
        case STRING_atan_NUM:
            MAKE_D0 ( Ex ( d0 ) / ( 1 + pow ( Ex ( c0 ), 2 ) ) );
            break;
        case STRING_atan2_NUM:
            MAKE_D0D1 ( 1 / ( 1 + pow ( Ex ( c0 ) / Ex ( c1 ) , 2 ) ) * ( Ex ( d0 ) / Ex ( c1 ) - Ex ( d1 ) * Ex ( c0 ) / pow ( Ex ( c1 ) , 2 ) ) );
            break;
        case STRING_pow_NUM:
            if ( a->func_data()->children[1]->type == Op::NUMBER ) {
                Op *c0 = a->func_data()->children[0];
                Op *c1 = a->func_data()->children[1];
                diff_rec ( c0 );
                diff_rec ( c1 );
                Op *d0 = c0->additional_info;
                a->additional_info = to_inc_op (
                                         Ex ( c1 ) * Ex ( d0 ) * pow ( Ex ( c0 ), Ex ( a->func_data()->children[1]->number_data()->val - Rationnal ( 1 ) ) )
                                     );
                break;
            }
            MAKE_D0D1 ( Ex ( d0 ) * Ex ( c1 ) * pow ( Ex ( c0 ), Ex ( c1 ) - Rationnal ( 1 ) ) + /* dx * x ^ ( y - 1 ) */ Ex ( d1 ) * log ( Ex ( c0 ) ) * Ex ( a ) /*dy * log( x ) * x ^ y*/ );
            break;
        case STRING_Fun_enr_3D_NUM:     
            MAKE_D0D1D2_name ( Ex ( d0 ) * Fun_enr_3D_dx( Ex (c0), Ex (c1), Ex(c2) , Ex(c3), Ex(c4), Ex(c5)) + Ex (d1) * Fun_enr_3D_dy( Ex (c0) , Ex(c1), Ex(c2) , Ex(c3), Ex(c4), Ex(c5) ) + Ex (d2) * Fun_enr_3D_dz( Ex (c0) , Ex(c1), Ex(c2) , Ex(c3), Ex(c4), Ex(c5) ) );
            break;
        case STRING_Fun_enr_3D_dx_NUM:     
            MAKE_D0D1D2_name ( Ex ( 0 ) );
            break;
        case STRING_Fun_enr_3D_dy_NUM:     
            MAKE_D0D1D2_name ( Ex ( 0 ) );
            break;
        case STRING_Fun_enr_3D_dz_NUM:     
            MAKE_D0D1D2_name ( Ex ( 0 ) );
            break;            
        case STRING_Young_enr_3D_NUM:     
            MAKE_D0D1D2_name ( Ex ( 0 ) );
            break;
        case STRING_Poisson_enr_3D_NUM:     
            MAKE_D0D1D2_name ( Ex ( 0 ) );
            break;
        case STRING_Fun_enr_NUM:     
            MAKE_D0D1_name ( Ex ( d0 ) * Fun_enr_dx( Ex (c0), Ex (c1), Ex(c2) , Ex(c3), Ex(c4)) + Ex (d1) * Fun_enr_dy( Ex (c0) , Ex(c1), Ex(c2) , Ex(c3), Ex(c4) ) );
            break;
        case STRING_Fun_enr_dx_NUM:     
            MAKE_D0D1_name ( Ex ( 0 ) );
            break;
        case STRING_Fun_enr_dy_NUM:     
            MAKE_D0D1_name ( Ex ( 0 ) );
            break;
        case STRING_Young_enr_NUM:     
            MAKE_D0D1_name ( Ex ( 0 ) );
            break;
        case STRING_Poisson_enr_NUM:     
            MAKE_D0D1_name ( Ex ( 0 ) );
            break;
        default:
            th->add_error ( "for now, no rules to differentiate function of type '"+std::string ( Nstring ( a->type ) ) +"'.", tok );
        }
    }

    Thread *th;
    const void *tok;
    Ex zero, one;
};

Ex Ex::diff ( Thread *th, const void *tok, const Ex &a ) const {
    DiffRec dr ( th, tok, a );
    dr.diff_rec ( op );
    return op->additional_info;
}

void diff ( Thread *th, const void *tok, const VarArgs &expr, const Ex &d, VarArgs &res ) {
    DiffRec dr ( th, tok, d );
    for ( unsigned i=0;i<expr.variables.size();++i ) {
        dr.diff_rec ( reinterpret_cast<Ex *> ( expr.variables[i].data )->op );
        Variable *v = res.variables.new_elem();
        v->init ( global_data.Op, 0 );
        reinterpret_cast<Ex *> ( v->data )->op = reinterpret_cast<Ex *> ( expr.variables[i].data )->op->additional_info->inc_ref();
    }
}

// ------------------------------------------------------------------------------------------------------------
struct MinMax {
    Ex beg;
    Ex end;
};

void get_interval_rec ( Thread *th, const void *tok, Op *op, SplittedVec<MinMax,64> &v ) {
    if ( op->op_id == Op::current_op )
        return;
    op->op_id = Op::current_op;
    op->additional_info = reinterpret_cast<Op *> ( v.new_elem() );
    MinMax *min_max = reinterpret_cast<MinMax *> ( op->additional_info );
    //
    //
    #define up_ch_0  get_interval_rec( th, tok, op->func_data()->children[0], v )
    #define up_ch_1  get_interval_rec( th, tok, op->func_data()->children[1], v )
    #define mm_ch_0  reinterpret_cast<MinMax *>( op->func_data()->children[0]->additional_info )
    #define mm_ch_1  reinterpret_cast<MinMax *>( op->func_data()->children[1]->additional_info )
    switch ( op->type ) {
    case Op::NUMBER:
        min_max->beg = op;
        min_max->end = op;
        break;
    case Op::SYMBOL:
        min_max->beg = op;
        min_max->end = op;
        break;

        // monotonic
    case STRING_heaviside_NUM:
        up_ch_0;
        min_max->beg = heaviside ( mm_ch_0->beg );
        min_max->end = heaviside ( mm_ch_0->end );
        break;
    case STRING_log_NUM:
        up_ch_0;
        min_max->beg = log ( mm_ch_0->beg );
        min_max->end = log ( mm_ch_0->end );
        break;
    case STRING_pos_part_NUM:
        up_ch_0;
        min_max->beg = pos_part ( mm_ch_0->beg );
        min_max->end = pos_part ( mm_ch_0->end );
        break;
    case STRING_exp_NUM:
        up_ch_0;
        min_max->beg = exp ( mm_ch_0->beg );
        min_max->end = exp ( mm_ch_0->end );
        break;
    case STRING_floor_NUM:
        up_ch_0;
        min_max->beg = floor ( mm_ch_0->beg );
        min_max->end = floor ( mm_ch_0->end );
        break;
    case STRING_ceil_NUM:
        up_ch_0;
        min_max->beg = ceil ( mm_ch_0->beg );
        min_max->end = ceil ( mm_ch_0->end );
        break;
    case STRING_round_NUM:
        up_ch_0;
        min_max->beg = round ( mm_ch_0->beg );
        min_max->end = round ( mm_ch_0->end );
        break;
    case STRING_atan_NUM:
        up_ch_0;
        min_max->beg = atan ( mm_ch_0->beg );
        min_max->end = atan ( mm_ch_0->end );
        break;
        //
    case STRING_eqz_NUM:
        up_ch_0;
        min_max->beg = eqz ( mm_ch_0->beg ) * eqz ( mm_ch_0->end );
        min_max->end = heaviside ( - mm_ch_0->beg ) * heaviside ( mm_ch_0->end );
        break;
    case STRING_add_NUM:
        up_ch_0;
        up_ch_1;
        min_max->beg = mm_ch_0->beg + mm_ch_1->beg;
        min_max->end = mm_ch_0->end + mm_ch_1->end;
        break;
    case STRING_mul_NUM:
        //             Ex same_side = heaviside( mm_ch_0->beg ) * heaviside( mm_ch_1->beg ) + heaviside( - mm_ch_0->end ) * heaviside( - mm_ch_1->end );
        //             Ex oppo_side = heaviside( mm_ch_0->beg ) * heaviside( - mm_ch_1->end ) + heaviside( - mm_ch_0->end ) * heaviside( mm_ch_1->beg );
        //
        //             Ex cen_0_neg_1 = heaviside( - mm_ch_0->beg ) * heaviside( mm_ch_0->end ) * heaviside( - mm_ch_1->end );
        //             Ex cen_0_cen_1 = heaviside( - mm_ch_0->beg ) * heaviside( mm_ch_0->end ) * heaviside( - mm_ch_1->beg ) * heaviside( mm_ch_1->end );
        //             Ex cen_0_pos_1 = heaviside( - mm_ch_0->beg ) * heaviside( mm_ch_0->end ) * heaviside( mm_ch_1->beg );
        //
        //             min_max->beg =
        //                 mm_ch_0->beg * mm_ch_1->beg * same_side +
        //                 mm_ch_0->end * mm_ch_1->end * oppo_side +
        //                 mm_ch_0->end * mm_ch_1->beg * cen_0_neg_1
        //                 min( mm_ch_0->beg * mm_ch_1->beg );
        //             min_max->end =
        //                 mm_ch_0->end * mm_ch_1->end * same_side +
        //                 mm_ch_0->beg * mm_ch_1->beg * oppo_side +
        //                 mm_ch_0->beg * mm_ch_1->beg * cen_0_neg_1
        //                 ;
        up_ch_0;
        up_ch_1;
        min_max->beg = min ( min ( min ( mm_ch_0->beg * mm_ch_1->beg, mm_ch_0->beg * mm_ch_1->end ), mm_ch_0->end * mm_ch_1->beg ), mm_ch_0->end * mm_ch_1->end );
        min_max->end = max ( max ( max ( mm_ch_0->beg * mm_ch_1->beg, mm_ch_0->beg * mm_ch_1->end ), mm_ch_0->end * mm_ch_1->beg ), mm_ch_0->end * mm_ch_1->end );
        break;
    case STRING_abs_NUM:
        up_ch_0;
        min_max->beg = min ( abs ( mm_ch_0->beg ), abs ( mm_ch_0->end ) );
        min_max->end = max ( abs ( mm_ch_0->beg ), abs ( mm_ch_0->end ) );
        break;
        // case STRING_sin_NUM:        break;
        // case STRING_cos_NUM:        break;
        // case STRING_tan_NUM:        break;
        // case STRING_asin_NUM:       break;
        // case STRING_acos_NUM:       break;
    case STRING_pow_NUM: {
        up_ch_0;
        up_ch_1;
        if ( mm_ch_1->beg.op->type == Op::NUMBER and mm_ch_1->beg.op->type == Op::NUMBER and mm_ch_1->beg.op->number_data()->val == mm_ch_1->end.op->number_data()->val ) {
            Rationnal expo = mm_ch_1->end.op->number_data()->val;
            std::cout << expo << std::endl;
            if ( expo.is_even() ) {
                Ex opposite = heaviside ( - mm_ch_0->beg ) * heaviside ( mm_ch_0->end );
                min_max->beg = ( 1 - opposite ) * min ( pow ( mm_ch_0->beg, expo ), pow ( mm_ch_0->end, expo ) );
                min_max->end = max ( pow ( mm_ch_0->beg, expo ), pow ( mm_ch_0->end, expo ) );
                break;
            } else if ( expo.is_odd() ) {
                if ( expo.is_pos() ) {
                    min_max->beg = pow ( mm_ch_0->beg, expo );
                    min_max->end = pow ( mm_ch_0->end, expo );
                } else {
                    min_max->beg = pow ( mm_ch_0->end, expo );
                    min_max->end = pow ( mm_ch_0->beg, expo );
                }
                break;
            }
        }
        Ex tmp = exp ( log ( Ex ( op->func_data()->children[0] ) ) * Ex ( op->func_data()->children[1] ) );
        get_interval_rec ( th, tok, tmp.op, v );
        min_max->beg = reinterpret_cast<MinMax *> ( tmp.op->additional_info )->beg;
        min_max->end = reinterpret_cast<MinMax *> ( tmp.op->additional_info )->end;
        break;
    }
    default:
        th->add_error ( "for now, no rules to get interval from function of type '"+std::string ( Nstring ( op->type ) ) +"'.", tok );
    }
}

void Ex::interval ( Thread *th, const void *tok, const VarArgs &a, const VarArgs &beg, const VarArgs &end, Ex &res_beg, Ex &res_end ) const {
    ++Op::current_op;
    SplittedVec<MinMax,64> v;
for ( unsigned i=0;i<a.nb_uargs();++i ) {
const Op *a_op = reinterpret_cast<Ex *> ( a.variables[i].data )->op;
    a_op->op_id = Op::current_op;
    a_op->additional_info = reinterpret_cast<Op *> ( v.new_elem() );
    reinterpret_cast<MinMax *> ( a_op->additional_info )->beg = *reinterpret_cast<Ex *> ( beg.variables[i].data );
    reinterpret_cast<MinMax *> ( a_op->additional_info )->end = *reinterpret_cast<Ex *> ( end.variables[i].data );
}
//
get_interval_rec ( th, tok, op, v );
//
res_beg = reinterpret_cast<MinMax *> ( op->additional_info )->beg;
res_end = reinterpret_cast<MinMax *> ( op->additional_info )->end;
}



// ------------------------------------------------------------------------------------------------------------
struct IdEx {
    Ex operator() ( const Ex &ex ) const {
        return ex;
    }
};

template<class Func = IdEx>
struct SubsRec {
SubsRec ( Thread *th, const void *tok, const Ex &expr, const Func &func = Func() ) : th ( th ), tok ( tok ), expr ( expr ) {
        subs_rec ( expr.op, func );
    }
    //     ~SubsRec() {
    //         ++Op::current_op;
    //         destroy_rec( expr.op );
    //         if ( d.op->op_id != Op::current_op )
    //             dec_ref( d.op->additional_info );
    //     }


    void subs_rec ( Op *a, const Func &func ) {
        if ( a->op_id == Op::current_op ) // already done ?
            return;
        a->op_id = Op::current_op;
        //
        #define MAKE_S0( res ) \
        { \
            Op *c0 = a->func_data()->children[0]; subs_rec( c0, func ); \
            Ex e0 = c0->additional_info; \
            a->additional_info = to_inc_op( func( res ) ); \
        }
        #define MAKE_S0S1( res ) \
        { \
            Op *c0 = a->func_data()->children[0]; subs_rec( c0, func ); \
            Op *c1 = a->func_data()->children[1]; subs_rec( c1, func ); \
            Ex e0 = c0->additional_info; \
            Ex e1 = c1->additional_info; \
            a->additional_info = to_inc_op( func( res ) ); \
        }
        #define MAKE_S0S1_name( res ) \
        { \
        Op *c0 = a->func_data()->children[0]; subs_rec( c0, func ); \
        Op *c1 = a->func_data()->children[1]; subs_rec( c1, func ); \
        Op *c2 = a->func_data()->children[2]; subs_rec( c2, func );\
        Op *c3 = a->func_data()->children[3]; subs_rec( c3, func );\
        Op *c4 = a->func_data()->children[4]; subs_rec( c4, func );\
        Op *c5 = a->func_data()->children[5]; subs_rec( c5, func );\
        Ex e0 = c0->additional_info; \
        Ex e1 = c1->additional_info; \
        Ex e2 = c2->additional_info; \
        Ex e3 = c3->additional_info; \
        Ex e4 = c4->additional_info; \
        Ex e5 = c5->additional_info; \ 
        a->additional_info = to_inc_op( func( res ) ); \
        }
        #define MAKE_S0S1_name_( res ) \
        { \
        Op *c0 = a->func_data()->children[0]; subs_rec( c0, func ); \
        Op *c1 = a->func_data()->children[1]; subs_rec( c1, func ); \
        Op *c2 = a->func_data()->children[2]; subs_rec( c2, func );\
        Op *c3 = a->func_data()->children[3]; subs_rec( c3, func );\
        Op *c4 = a->func_data()->children[4]; subs_rec( c4, func );\
        Ex e0 = c0->additional_info; \
        Ex e1 = c1->additional_info; \
        Ex e2 = c2->additional_info; \
        Ex e3 = c3->additional_info; \
        Ex e4 = c4->additional_info; \
        a->additional_info = to_inc_op( func( res ) ); \
        }

        switch ( a->type ) {
        case Op::NUMBER:
            a->additional_info = to_inc_op ( func ( a ) );
            break;
        case Op::SYMBOL:
            a->additional_info = to_inc_op ( func ( a ) );
            break;
        case STRING_add_NUM:
            MAKE_S0S1 ( e0 + e1 );
            break;
        case STRING_mul_NUM:
            MAKE_S0S1 ( e0 * e1 );
            break;
        case STRING_pow_NUM:
            MAKE_S0S1 ( pow ( e0, e1 ) );
            break;
        case STRING_atan2_NUM:
            MAKE_S0S1 ( atan2 ( e0, e1 ) );
            break;
        case STRING_select_symbolic_NUM:
            MAKE_S0S1 ( select_symbolic ( e0, e1 ) );
            break;

        case STRING_floor_NUM:
            MAKE_S0 ( floor ( e0 ) );
            break;
        case STRING_ceil_NUM:
            MAKE_S0 ( ceil ( e0 ) );
            break;
        case STRING_round_NUM:
            MAKE_S0 ( round ( e0 ) );
            break;
        case STRING_heaviside_NUM:
            MAKE_S0 ( heaviside ( e0 ) );
            break;
        case STRING_pos_part_NUM:
            MAKE_S0 ( pos_part ( e0 ) );
            break;
        case STRING_eqz_NUM:
            MAKE_S0 ( eqz ( e0 ) );
            break;
        case STRING_log_NUM:
            MAKE_S0 ( log ( e0 ) );
            break;
        case STRING_abs_NUM:
            MAKE_S0 ( abs ( e0 ) );
            break;
        case STRING_exp_NUM:
            MAKE_S0 ( exp ( e0 ) );
            break;

        case STRING_sin_NUM:
            MAKE_S0 ( sin ( e0 ) );
            break;
        case STRING_cos_NUM:
            MAKE_S0 ( cos ( e0 ) );
            break;
        case STRING_tan_NUM:
            MAKE_S0 ( tan ( e0 ) );
            break;
        case STRING_asin_NUM:
            MAKE_S0 ( asin ( e0 ) );
            break;
        case STRING_acos_NUM:
            MAKE_S0 ( acos ( e0 ) );
            break;
        case STRING_atan_NUM:
            MAKE_S0 ( atan ( e0 ) );
            break;
        case STRING_Fun_enr_3D_NUM:
            MAKE_S0S1_name ( Fun_enr_3D( e0, e1, e2, e3, e4, e5 ) );
            break;
        case STRING_Fun_enr_3D_dx_NUM:
            MAKE_S0S1_name ( Fun_enr_3D_dx( e0, e1, e2, e3, e4, e5 ) );
            break;
        case STRING_Fun_enr_3D_dy_NUM:
            MAKE_S0S1_name ( Fun_enr_3D_dy( e0, e1, e2, e3, e4, e5 ) );
            break;
        case STRING_Fun_enr_3D_dz_NUM:
            MAKE_S0S1_name ( Fun_enr_3D_dz( e0, e1, e2, e3, e4, e5 ) );
            break;            
        case STRING_Young_enr_3D_NUM:
            MAKE_S0S1_name ( Young_enr_3D( e0, e1, e2, e3, e4, e5 ) );
            break;
        case STRING_Poisson_enr_3D_NUM:
            MAKE_S0S1_name ( Poisson_enr_3D( e0, e1, e2, e3, e4, e5 ) );
            break;
        case STRING_Fun_enr_NUM:
            MAKE_S0S1_name_ ( Fun_enr( e0, e1, e2, e3, e4 ) );
            break;
        case STRING_Fun_enr_dx_NUM:
            MAKE_S0S1_name_ ( Fun_enr_dx( e0, e1, e2, e3, e4 ) );
            break;
        case STRING_Fun_enr_dy_NUM:
            MAKE_S0S1_name_ ( Fun_enr_dy( e0, e1, e2, e3, e4 ) );
            break;
        case STRING_Young_enr_NUM:
            MAKE_S0S1_name_ ( Young_enr( e0, e1, e2, e3, e4 ) );
            break;
        case STRING_Poisson_enr_NUM:
            MAKE_S0S1_name_ ( Poisson_enr( e0, e1, e2, e3, e4 ) );
            break;
        default:
            th->add_error ( "for now, no rules to subs function of type '"+std::string ( Nstring ( a->type ) ) +"'.", tok );
        }
    }

    Thread *th;
    const void *tok;
    Ex expr;
};

Ex Ex::subs ( Thread *th, const void *tok, const VarArgs &a, const VarArgs &b ) const {
    ++Op::current_op;
for ( unsigned i=0;i<a.variables.size();++i ) {
reinterpret_cast<Ex *> ( a.variables[i].data )->op->op_id = Op::current_op;
    reinterpret_cast<Ex *> ( a.variables[i].data )->op->additional_info = reinterpret_cast<Ex *> ( b.variables[i].data )->op->inc_ref();
}
SubsRec<> sr ( th, tok, *this );
return op->additional_info;
}

Ex Ex::subs ( Thread *th, const void *tok, const SEX &a, const SEX &b ) const {
    ++Op::current_op;
for ( unsigned i=0;i<a.size();++i ) {
a[i].op->op_id = Op::current_op;
    a[i].op->additional_info = b[i].op->inc_ref();
}
SubsRec<> sr ( th, tok, *this );
return op->additional_info;
}

Ex Ex::subs ( Thread *th, const void *tok, const Ex &a, const Ex &b ) const {
    ++Op::current_op;
    a.op->op_id = Op::current_op;
    a.op->additional_info = b.op->inc_ref();
    SubsRec<> sr ( th, tok, *this );
    return op->additional_info;
}

// ------------------------------------------------------------------------------------------------------------
int Ex::poly_deg ( const Ex &var ) const {
    ++Op::current_op;
    var.op->op_id = Op::current_op;
    var.op->additional_int = 1;
    return op->poly_deg_rec();
}

// ------------------------------------------------------------------------------------------------------------
int Ex::poly_deg ( const VarArgs &var ) const {
    ++Op::current_op;
for ( unsigned i=0;i<var.nb_uargs();++i ) {
Op *op = reinterpret_cast<Ex *> ( var.uarg ( i )->data )->op;
    op->op_id = Op::current_op;
    op->additional_int = 1;
}
return op->poly_deg_rec();
}

// ------------------------------------------------------------------------------------------------------------
unsigned Ex::node_count() const {
    return op->nb_nodes();
}
unsigned Ex::ops_count() const {
    return op->nb_ops();
}
unsigned Ex::nb_sub_symbols() const {
    return op->nb_nodes_of_type ( Op::SYMBOL );
}

// ------------------------------------------------------------------------------------------------------------

struct ExpandOp {
    Ex make_mul ( const SplittedVec<const Op *,32> &res, unsigned beg ) const {
        if ( res.size() == beg )
        return 1;
        //
        const Op *a = res[ beg ];
        if ( a->type != STRING_add_NUM )
            return a;
            //
            SplittedVec<const Op *,32> add_children;
            get_child_not_of_type_add ( a, add_children );
            Ex sum, rem = make_mul ( res, beg + 1 );
            for ( unsigned i=0;i<add_children.size();++i )
                sum += add_children[i] * rem;
                return sum;
            }
            Ex operator() ( const Ex &ex ) const {
                    Op *a = ex.op;
                if ( a->type == STRING_mul_NUM ) {
                SplittedVec<const Op *,32> res;
                get_child_not_of_type_mul ( a, res );
                    return make_mul ( res, 0 );
                }
    return ex;
    }
};

Ex Ex::expand ( Thread *th, const void *tok ) const {
    ++Op::current_op;
    SubsRec<ExpandOp> sr ( th, tok, *this );
    return op->additional_info;
}

// ------------------------------------------------------------------------------------------------------------
struct LinearizeDiscOp {
    Ex operator() ( const Ex &ex ) const {
        Op *a = ex.op;
    if ( a->type == STRING_heaviside_NUM or a->type == STRING_abs_NUM or a->type == STRING_pos_part_NUM ) {
    Ex ch ( a->func_data()->children[0] );
        ch.op->inc_ref();
        ch.op->inc_ref();

        Ex res = ch.subs ( th, tok, *vars, *mids );
        std::cout << " -> " << ch  << std::endl;
        for ( unsigned i=0;i<nb_vars;++i ) {
            const Ex &var = *reinterpret_cast<const Ex *> ( vars->variables[i].data );
            const Ex &mid = *reinterpret_cast<const Ex *> ( mids->variables[i].data );
            std::cout << var << std::endl;
            std::cout << mid << std::endl;
            std::cout << ch.diff ( th, tok, var ) << std::endl;
            res += ch.diff ( th, tok, var ).subs ( th, tok, *vars, *mids ) * ( var - mid );
        }
        std::cout << res << std::endl;
        switch ( a->type ) {
        case STRING_heaviside_NUM:
            return ex; // heaviside( res + 0.1 );
        case STRING_abs_NUM:
            return abs ( res );
        case STRING_pos_part_NUM:
            return pos_part ( res );
        default:
            assert ( 0 );
        }
    }
    return ex;
    }
    const VarArgs *vars;
    const VarArgs *mids;
    unsigned nb_vars;
    Thread *th;
    const void *tok;
};


Ex Ex::linearize_discontinuity_children ( Thread *th, const void *tok, const VarArgs &a, const VarArgs &b ) const {
    ++Op::current_op;
    LinearizeDiscOp func;
    func.vars = &a;
    func.mids = &b;
    func.th   = th;
    func.tok  = tok;
    func.nb_vars = std::min ( a.variables.size(), b.variables.size() );
    SubsRec<LinearizeDiscOp> sr ( th, tok, *this, func );
    return op->additional_info;
}

// ------------------------------------------------------------------------------------------------------------
Rationnal Ex::subs_numerical ( Thread *th, const void *tok, const Rationnal &a ) const {
    SplittedVec<const Op *,32> symbols;
    get_sub_symbols ( op, symbols );
if ( symbols.size() > 1 ) {
std::ostringstream ss;
ss << "subs_numerical works only with expressions which contains at most 1 variable ( remaining ones are ";
for ( unsigned i=0;i<symbols.size();++i )
        ss << Ex ( symbols[i] ) << " ";
    ss << ")";
    th->add_error ( ss.str(),tok );
    return 0;
}
//
if ( symbols.size() == 0 )
return value();

// TODO : Optimize
Ex res = subs ( th, tok, Ex ( symbols[0] ), Ex ( a ) );
if ( symbols.size() > 1 ) {
    th->add_error ( "subs_numerical works on expr with only one symbol.",tok );
        return 0;
    }
return res.value();
}

// ------------------------------------------------------------------------------------------------------------
// a posteriori factorization
// ------------------------------------------------------------------------------------------------------------
bool same_abs_value ( const Op *a, const Op *b ) {
    if ( a->type == Op::NUMBER and b->type == Op::NUMBER )
        return abs ( a->number_data()->val ) == abs ( b->number_data()->val );
    return false;
}
bool same_op_or_same_abs_value ( const Op *a, const Op *b ) {
    if ( a == b )
        return true;
    return same_abs_value ( a, b );
}

struct CmpByOpPtr {
    bool operator() ( const Ex &a, const Ex &b ) const {
        return a.op < b.op;
    }
};

struct SumMulSeq {
    struct Item {
        const Op *m;
        Ex::T e;
        const Op *corr_op;
    };
    typedef SplittedVec<Item      ,4,4,true> MulList;
    typedef SplittedVec<MulList   ,4,4,true> AddList;
    typedef SplittedVec<const Op *,4,4,true> AddOLst;

    void add_to_quo_lst ( const Op *op, std::vector<Ex> &quo_lst ) {
        if ( op->type == STRING_add_NUM ) {
            add_to_quo_lst ( op->func_data()->children[0], quo_lst );
            add_to_quo_lst ( op->func_data()->children[1], quo_lst );
        } else
            quo_lst.push_back ( op );
    }
    Ex sep_by ( const Op *m, const Ex::T &e ) {
        // m^e * quo + rem
        std::vector<Ex> quo_lst, rem_lst;
        for ( unsigned i=0;i<items.size();++i ) {
            bool has = false;
            for ( unsigned j=0;j<items[i].size();++j ) {
                if ( same_op_or_same_abs_value ( items[i][j].m, m ) ) {
                    if ( items[i][j].e.is_pos() )
                        has |= ( e.is_pos() and items[i][j].e >= e );
                    else
                        has |= ( e.is_neg() and items[i][j].e <= e );
                }
            }
            if ( has ) {
                Ex p ( 1 );
                for ( unsigned j=0;j<items[i].size();++j ) {
                    if ( items[i][j].m == m ) {
                        if ( items[i][j].e.is_pos() ) {
                            if ( e.is_pos() and items[i][j].e >= e ) {
                                if ( items[i][j].e != e )
                                    p *= pow ( Ex ( items[i][j].m ), Ex ( items[i][j].e - e ) );
                            } else
                                p *= Ex ( items[i][j].corr_op );
                        } else {
                            if ( e.is_neg() and items[i][j].e <= e ) {
                                if ( items[i][j].e != e )
                                    p *= pow ( Ex ( items[i][j].m ), Ex ( items[i][j].e - e ) );
                            } else
                                p *= Ex ( items[i][j].corr_op );
                        }
                    } else if ( same_abs_value ( items[i][j].m, m ) )
                        p *= 1 - 2 * ( items[i][j].m->number_data()->val.is_pos() xor m->number_data()->val.is_pos() );
                    else
                        p *= items[i][j].corr_op;
                }
                add_to_quo_lst ( p.op, quo_lst );
            } else
                rem_lst.push_back ( corr_ops[i] );
        }
        //
        std::sort ( quo_lst.begin(), quo_lst.end(), CmpByOpPtr() );
        std::sort ( rem_lst.begin(), rem_lst.end(), CmpByOpPtr() );
        Ex quo, rem;
        for ( unsigned i=0;i<quo_lst.size();++i ) quo += quo_lst[i];
        for ( unsigned i=0;i<rem_lst.size();++i ) rem += rem_lst[i];
        return pow ( Ex ( m ), Ex ( e ) ) * quo + rem;
    }

    AddList items;
    AddOLst corr_ops;
};
void get_SumMulSeq_mul_seq ( const Op *op, SumMulSeq::MulList &ml ) {
    if ( op->type == STRING_mul_NUM ) {
        get_SumMulSeq_mul_seq ( op->func_data()->children[0], ml );
        get_SumMulSeq_mul_seq ( op->func_data()->children[1], ml );
    } else {
        SumMulSeq::Item *item = ml.new_elem();
        if ( op->type == STRING_pow_NUM and op->func_data()->children[1]->type == Op::NUMBER ) {
            item->m       = op->func_data()->children[0];
            item->e       = op->func_data()->children[1]->number_data()->val;
            item->corr_op = op;
        } else {
            item->m       = op;
            item->e       = 1;
            item->corr_op = op;
        }
    }
}
void get_SumMulSeq ( const Op *op, SumMulSeq &sq ) {
    if ( op->type == STRING_add_NUM ) {
        get_SumMulSeq ( op->func_data()->children[0], sq );
        get_SumMulSeq ( op->func_data()->children[1], sq );
    } else {
        sq.corr_ops.push_back ( op );
        get_SumMulSeq_mul_seq ( op, *sq.items.new_elem() );
    }
}
std::ostream &operator<< ( std::ostream &os, const SumMulSeq &sq ) {
    for ( unsigned i=0;i<sq.items.size();++i )
        for ( unsigned j=0;j<sq.items[i].size();++j )
            os << ( j ? "*" : ( i ? "+" : "" ) ) << Ex ( sq.items[i][j].m ) << "^" << sq.items[i][j].e;
    return os;
}

// ------------------------------------------------------------------------------------------------------------
struct ItemAndFreqSet {
    struct Item {
Item ( const Op *m, Ex::T e ) : m ( m ), e ( e ), freq ( 0 ) {}
        const Op *m;
        Ex::T e;
        int freq;
    };
    void insert ( const Op *m, Ex::T e ) {
        for ( unsigned i=0;i<items.size();++i )
            if ( same_op_or_same_abs_value ( items[i].m, m ) and items[i].e == e )
                return;
        if ( m->is_minus_one() ==false and m->is_one() ==false ) // -1 or 1
            items.push_back ( Item ( m, e ) );
    }
    void add_freq ( const Op *m, Ex::T e ) {
        for ( unsigned i=0;i<items.size();++i ) {
            if ( same_op_or_same_abs_value ( items[i].m, m ) ) {
                if ( items[i].e.is_pos() )
                    items[i].freq += ( e.is_pos() and items[i].e <= e );
                else
                    items[i].freq += ( e.is_neg() and items[i].e >= e );
            }
        }
    }
    Item *best_item() {
        int best_ind = 0;
        for ( unsigned i=0;i<items.size();++i )
            if ( abs ( items[best_ind].freq ) < abs ( items[i].freq ) )
                best_ind = i;
        return &items[ best_ind ];
    }
    std::vector<Item> items;
};
std::ostream &operator<< ( std::ostream &os, const ItemAndFreqSet::Item &item ) {
    os << Ex ( item.m ) << "^" << item.e << " (" << item.freq << ")";
    return os;
}

Ex add_a_posteriori_simplification ( const Ex &a ) {
    if ( a.op->simplified )
        return a;
    //
    if ( a.op->type == STRING_add_NUM ) {
        SumMulSeq sq;
        get_SumMulSeq ( a.op, sq );
        // get frequency of sub items
        ItemAndFreqSet items_and_freq;
        for ( unsigned i=0;i<sq.items.size();++i )
            for ( unsigned j=0;j<sq.items[i].size();++j )
                items_and_freq.insert ( sq.items[i][j].m, sq.items[i][j].e );
        for ( unsigned i=0;i<sq.items.size();++i )
            for ( unsigned j=0;j<sq.items[i].size();++j )
                items_and_freq.add_freq ( sq.items[i][j].m, sq.items[i][j].e );
        ItemAndFreqSet::Item *best_item = items_and_freq.best_item();
        //
        if ( best_item->freq > 1 )
            return a_posteriori_simplification ( sq.sep_by ( best_item->m, best_item->e ) );
    }
    //
    a.op->simplified = true;
    return a;
}
Ex a_posteriori_simplification ( const Ex &a ) {
    #ifdef WITHOUT_SIMP
    return a;
    #endif
    return add_a_posteriori_simplification ( a );
}








// ------------------------------------------------------------------------------------------------------------
void get_taylor_expansion ( Thread *th, const void *tok, Ex expr, const Ex &center, const Ex &var, Int32 deg_poly_max, SEX &res ) {
    #ifdef USE_SERIES_FOR_TAYLOR
    Ex tmp ( "tmp",3,"tmp",3 );
    Ex e = expr.subs ( th, tok, var, center + tmp );
    SEX expressions;
    expressions.push_back ( e );
    res.get_room_for ( deg_poly_max + 1 );
    polynomial_expansion ( th, tok, expressions, tmp, deg_poly_max, res );
    for ( unsigned i=0;i<res.size();++i )
        res[i] = res[i].subs ( th, tok, tmp, var - center );
    #else
    Rationnal r ( 1 );
    for ( Int32 i=0;i<=deg_poly_max;++i ) {
        Ex t = r * expr.subs ( th, tok, var, center );
        res.push_back ( t );
        if ( i < deg_poly_max ) {
            expr = expr.diff ( th, tok, var );
            r /= i + 1;
        }
    }
    #endif
}

void get_taylor_expansion_manual ( Thread *th, const void *tok, Ex expr, const Ex &center, const Ex &var, Int32 deg_poly_max, SEX &res ) {
    Rationnal r ( 1 );
    for ( Int32 i=0;i<=deg_poly_max;++i ) {
        Ex t = r * expr.subs ( th, tok, var, center );
        res.push_back ( t );
        if ( i < deg_poly_max ) {
            expr = expr.diff ( th, tok, var );
            r /= i + 1;
        }
    }
}

#include "fit.h"

template<unsigned n>
struct Poly_fit_01_rec {
    struct Coeffs {
        Ex c[ n ];
    };

    void get_poly ( const Op *a ) {
        if ( a->op_id == Op::current_op )
            return;
        a->op_id = Op::current_op;
        Coeffs *pol = coeffs.new_elem();
        a->additional_info = reinterpret_cast<Op *> ( pol );
        //
        switch ( a->type ) {
        case Op::NUMBER:
        case Op::SYMBOL:
            pol->c[0] = a;
            break;
        case STRING_add_NUM:
            get_poly ( a->func_data()->children[0] );
            get_poly ( a->func_data()->children[1] );
            for ( unsigned i=0;i<n;++i )
                pol->c[i] = reinterpret_cast<const Coeffs *> ( a->func_data()->children[0]->additional_info )->c[i] +
                            reinterpret_cast<const Coeffs *> ( a->func_data()->children[1]->additional_info )->c[i];
            break;
        case STRING_mul_NUM:
            get_poly ( a->func_data()->children[0] );
            get_poly ( a->func_data()->children[1] );
            get_fit_from_mul (
                reinterpret_cast<const Coeffs *> ( a->func_data()->children[0]->additional_info )->c,
                reinterpret_cast<const Coeffs *> ( a->func_data()->children[1]->additional_info )->c,
                pol->c, N<n>()
            );
            break;
        default:
            th->add_error ( "Unable to manage function of type "+std::string ( Nstring ( a->type ) ) +" during poly fit propagation", tok );
        }
    }

    SplittedVec<Coeffs,64,64,true> coeffs;
    Thread *th;
    const void *tok;
};


template<int n>
void get_poly_fit_01 ( Thread *th, const void *tok, const Ex &expr, const Ex &var, SEX &res, N<n> ) {
    typedef Poly_fit_01_rec<n> PF;
    PF p;
    //
    typename PF::Coeffs *pol = p.coeffs.new_elem();
    pol->c[ 1 ] = 1;
    //
    ++Op::current_op;
    var.op->op_id = Op::current_op;
    var.op->additional_info = reinterpret_cast<Op *> ( pol );
    //
    p.th = th;
    p.tok = tok;
    p.get_poly ( expr.op );
    for ( unsigned i=0;i<n;++i )
        res.push_back ( reinterpret_cast<typename PF::Coeffs *> ( expr.op->additional_info )->c[ i ] );
}

// get coeffs of P( x - beg )
template<int n>
void get_poly_fit ( Thread *th, const void *tok, const Ex &expr, const Ex &var, const Ex &beg, const Ex &end, SEX &res, N<n> ) {
    Ex new_var ( "v",1,"v",2 );
    Ex new_expr = expr.subs ( th, tok, var, beg + new_var * ( end - beg ) );
    get_poly_fit_01 ( th, tok, new_expr, new_var, res, N<n>() );
    Ex d = end - beg;
    for ( unsigned i=0;i<n;++i )
        res[ i ] = res[ i ] / pow ( d, i );
}

void get_poly_fit ( Thread *th, const void *tok, const Ex &expr, const Ex &var, const Ex &beg, const Ex &end, Int32 deg_poly, SEX &res ) {
    switch ( deg_poly ) {
    case 0 :
        res.push_back ( expr );
        break;
    case 1 :
        get_poly_fit ( th, tok, expr, var, beg, end, res, N<2 >() );
        break;
    case 2 :
        get_poly_fit ( th, tok, expr, var, beg, end, res, N<3 >() );
        break;
    case 3 :
        get_poly_fit ( th, tok, expr, var, beg, end, res, N<4 >() );
        break;
//         case 4 : get_poly_fit( th, tok, expr, var, beg, end, res, N<5 >() ); break;
//         case 5 : get_poly_fit( th, tok, expr, var, beg, end, res, N<6 >() ); break;
//         case 6 : get_poly_fit( th, tok, expr, var, beg, end, res, N<7 >() ); break;
//         case 7 : get_poly_fit( th, tok, expr, var, beg, end, res, N<8 >() ); break;
//         case 8 : get_poly_fit( th, tok, expr, var, beg, end, res, N<9 >() ); break;
//         case 9 : get_poly_fit( th, tok, expr, var, beg, end, res, N<10>() ); break;
//         case 10: get_poly_fit( th, tok, expr, var, beg, end, res, N<11>() ); break;
    }
}

Ex make_poly_fit ( Thread *th, const void *tok, const Ex &expr, const Ex &var, const Ex &beg, const Ex &end, Int32 deg_poly ) {
    SEX pol;
    get_poly_fit ( th, tok, expr, beg, end, var, deg_poly, pol );
    //
    Ex res;
    for ( unsigned i=0;i<pol.size();++i )
        res += pol[i] * pow ( var - beg, i );
    return res;
}

Ex integration_with_taylor_expansion ( Thread *th, const void *tok, const Ex &expr, const Ex &var, const Ex &beg, const Ex &end, Int32 deg_poly_max ) {
    //
    SEX taylor_expansion;
    get_taylor_expansion ( th, tok, expr, ( beg + end ) / 2, var, deg_poly_max, taylor_expansion );
    //
    Ex res ( 0 ), d = ( end - beg ) / 2;
    for ( Int32 i=0; i< ( Int32 ) taylor_expansion.size(); i += 2 )
        res = res + 2 * taylor_expansion[i] * pow ( d, Ex ( Rationnal ( i + 1 ) ) ) * Rationnal ( 1, i + 1 );
    return res;
    //     SEX taylor_expansion;
    //     get_taylor_expansion( th, tok, expr, beg, var, deg_poly_max, taylor_expansion );
    //     //
    //     Ex res( 0 ), d = end - beg;
    //     for( Int32 i=0; i<(Int32)taylor_expansion.size(); ++i )
    //         res = res + taylor_expansion[i] * pow( d, Ex( Rationnal( i + 1 ) ) ) * Rationnal( 1, i + 1 );
    //     return res;
}


Ex get_roots_with_validity ( const SEX &taylor_expansion, SEX &roots, SEX &validity ) {
    for ( unsigned i=0;i<std::min ( 3u,taylor_expansion.size() );++i ) {
        roots   .push_back ( 0 );
        validity.push_back ( 0 );
    }

    //
    Ex z = taylor_expansion[3], ez = eqz ( z );
    Ex a = taylor_expansion[2] / ( z + ez ), b = taylor_expansion[1] / ( z + ez ), c = taylor_expansion[0] / ( z + ez );
    Ex delta = pow ( b, 2 ) - 4 * a * c;
    Ex ea = eqz ( a ), eb = eqz ( b ), hd = heaviside ( delta );
    Ex sq_delta = pow ( delta * hd, Rationnal ( 1, 2 ) );
    roots   [ 0 ] = - ( ( 1 - ea ) * ( b + sq_delta ) + ea * c ) / ( 2 * a + ea * ( b + eb ) ); // first root
    roots   [ 1 ] = - ( b - sq_delta ) / ( 2 * a + ea );                                        // second one
    validity[ 0 ] = 1 - ea * eb;
    validity[ 1 ] = ( 1 - ea ) * hd;

    // order 3
    if ( not z.op->is_zero() ) {
        Ex p = b - pow ( a,2 ) / 3;
        Ex q = pow ( a,3 ) * Rationnal ( 2,27 ) - a * b / 3 + c;
        Ex delta = 4 * pow ( p,3 ) + 27 * pow ( q,2 );
        //delta >= 0
        //         Ex u = (-27*q+sqrt(27*delta))/2;
        //         Ex v = (-27*q-sqrt(27*delta))/2;
        //         res += heaviside( delta ) * ExVector(
        //             sgn(u)*pow(abs(u),1.0/3.0)+sgn(v)*pow(abs(v),1.0/3.0)-a)/3.0
        //         );
        //delta < 0
        std::complex<Ex> j ( -Rationnal ( 1,2 ), sqrt_96 ( Rationnal ( 3,4 ) ) );
        std::complex<Ex> v ( -27 * q / 2, 0 );
        v += sqrtc ( - Rationnal ( 27,4 ) * delta ) * std::complex<Ex> ( 0, 1 );
        std::complex<Ex> u ( powc ( v, Rationnal ( 1,3 ) ) );
        Ex x0 = ( 2 * std::real ( u ) - a ) / 3;
        Ex x1 = ( 2 * std::real ( j*u ) - a ) / 3;
        Ex x2 = ( 2 * std::real ( j*j*u ) - a ) / 3;

        Ex va = 1 - ez;
        roots   [ 0 ] = ( 1 - va ) * roots   [ 0 ] + va * x0;
        validity[ 0 ] = ( 1 - va ) * validity[ 0 ] + va;
        va *= 1 - heaviside ( delta );
        roots   [ 1 ] = ( 1 - va ) * roots   [ 1 ] + va * x1;
        roots   [ 2 ] = ( 1 - va ) * roots   [ 2 ] + va * x2;
        validity[ 1 ] = ( 1 - va ) * validity[ 1 ] + va;
        validity[ 2 ] = ( 1 - va ) * validity[ 2 ] + va;
    }

    return ( a * ( 1 - ea ) + b * ( 1 - eb ) * ea ) * ez + z * ( 1 - ez );
}

// Ex integration_with_discontinuities_rec( Thread *th, const void *tok, const Ex &expr, const Ex &var, const Ex &beg, const Ex &end, Int32 deg_poly_max ) {
//     const Op *disc = expr.op->find_discontinuities( var.op );
//     if ( disc ) {
//         const Op *ch = disc->func_data()->children[0];
//         Ex ex_disc( disc );
//         Ex ex_ch( ch );
//
//         // ch = f * g
//         //   -> H( f * g ) = H( f ) * H( g ) + ( 1 - H( f ) ) * ( 1 - H( g ) )
//         if ( ch->type == STRING_mul_NUM ) {
//             Ex new_expr;
//             if ( disc->type == STRING_heaviside_NUM ) {
//                 Ex h_0 = heaviside( Ex( ch->func_data()->children[0] ) );
//                 Ex h_1 = heaviside( Ex( ch->func_data()->children[1] ) );
//                 new_expr = expr.subs( th, tok, ex_disc, h_0 * h_1 + ( 1 - h_0 ) * ( 1 - h_1 ) );
//             }
//             else if ( disc->type == STRING_abs_NUM ) {
//                 Ex a_0 = abs( Ex( ch->func_data()->children[0] ) );
//                 Ex a_1 = abs( Ex( ch->func_data()->children[1] ) );
//                 new_expr = expr.subs( th, tok, ex_disc, a_0 * a_1 );
//             }
//             else if ( disc->type == STRING_pos_part_NUM ) {
//                 Ex h_0 = heaviside( Ex( ch->func_data()->children[0] ) );
//                 Ex h_1 = heaviside( Ex( ch->func_data()->children[1] ) );
//                 new_expr = expr.subs( th, tok, ex_disc, ch * ( h_0 * h_1 + ( 1 - h_0 ) * ( 1 - h_1 ) ) );
//             }
//             else
//                 assert( 0 /* TODO */ );
//
//             return integration( th, tok, new_expr, var, beg, end, deg_poly_max );
//         }
//
//         // taylor_expansion
//         #ifdef WANT_ORDER_2_FOR_DISC_INTEGRATION
//         SEX taylor_expansion;
//         Ex mid = Rationnal( 1, 2 ) * ( beg + end );
//         Ex del = Rationnal( 1, 2 ) * ( end - beg );
//         get_taylor_expansion( th, tok, ex_ch, mid, var, 7, taylor_expansion );
//         SEX poly_coeff; // best L2 fitting of order 7 -> order 3
//         poly_coeff.push_back( taylor_expansion[0] - Rationnal( 3,35) * pow(del,4) * taylor_expansion[4] - Rationnal( 2,21) * pow(del,6) * taylor_expansion[6] );
//         poly_coeff.push_back( taylor_expansion[1] - Rationnal( 5,21) * pow(del,4) * taylor_expansion[5] - Rationnal(10,33) * pow(del,6) * taylor_expansion[7] );
//         poly_coeff.push_back( taylor_expansion[2] + Rationnal( 6, 7) * pow(del,2) * taylor_expansion[4] + Rationnal( 5, 7) * pow(del,4) * taylor_expansion[6] );
//         poly_coeff.push_back( taylor_expansion[3] + Rationnal(10, 9) * pow(del,2) * taylor_expansion[5] + Rationnal(35,33) * pow(del,4) * taylor_expansion[7] );
//
//         SEX roots, root_validity;
//         Ex leading_coefficient = get_roots_with_validity( poly_coeff, roots, root_validity );
//
//         // ch = f * g (mandatory : in case of several roots, ch is factorized and then we use this optimization)
//         //   -> H( f * g ) = H( f ) * H( g ) + ( 1 - H( f ) ) * ( 1 - H( g ) )
//         if ( ch->type == STRING_mul_NUM ) {
//             Ex new_expr;
//             if ( disc->type == STRING_heaviside_NUM ) {
//                 Ex h_0 = heaviside( Ex( ch->func_data()->children[0] ) );
//                 Ex h_1 = heaviside( Ex( ch->func_data()->children[1] ) );
//                 new_expr = expr.subs( th, tok, ex_disc, h_0 * h_1 + ( 1 - h_0 ) * ( 1 - h_1 ) );
//             }
//             else if ( disc->type == STRING_abs_NUM ) {
//                 Ex a_0 = abs( Ex( ch->func_data()->children[0] ) );
//                 Ex a_1 = abs( Ex( ch->func_data()->children[1] ) );
//                 new_expr = expr.subs( th, tok, ex_disc, a_0 * a_1 );
//             }
//             else if ( disc->type == STRING_pos_part_NUM ) {
//                 Ex h_0 = heaviside( Ex( ch->func_data()->children[0] ) );
//                 Ex h_1 = heaviside( Ex( ch->func_data()->children[1] ) );
//                 new_expr = expr.subs( th, tok, ex_disc, ch * ( h_0 * h_1 + ( 1 - h_0 ) * ( 1 - h_1 ) ) );
//             }
//             else
//                 assert( 0 /* TODO */ );
//
//             return integration( th, tok, new_expr, var, beg, end, deg_poly_max );
//         }
//
//         // several roots ?
//         if ( root_validity[1].op->is_zero()==false or root_validity[2].op->is_zero()==false ) {
//             Ex su = leading_coefficient;
//             for(unsigned i=0;i<roots.size();++i)
//                 if ( not root_validity[i].op->is_zero() )
//                     su *= root_validity[i] * ( var - ( mid + roots[ i ] ) - 1 ) + 1;
//             Ex new_expr = expr.subs( th, tok, ex_ch, su );
//             return integration( th, tok, new_expr, var, beg, end, deg_poly_max );
//         }
//         #else // order 1
//         SEX taylor_expansion;
//         Ex mid = Rationnal( 1, 2 ) * ( beg + end );
//         Ex del = Rationnal( 1, 2 ) * ( end - beg );
//         get_taylor_expansion( th, tok, ex_ch, mid, var, 7, taylor_expansion );
//         SEX poly_coeff; // best L2 fitting of order 7 -> order 1
//         /*
//             a := symbol("a")
//             c := Vec[Op,7]( function = x => symbol("c[$x]") )
//             v := Vec[Op,2]( function = x => symbol("v[$x]") )
//             p := dot( c, a ^ (0...) )
//             q := dot( v, a ^ (0...) )
//             del := symbol("del")
//             r := newton_raphson_minimize_iteration( integration( ( p - q ) ^ 2, a, -del, del, deg_poly_max = 7 ), v )
//             for i in c
//                 info i, r[0].diff(i)
//             for i in c
//                 info i, r[1].diff(i)
//         */
//         Ex p0 = taylor_expansion[0] + Rationnal(1,3) * pow(del,2) * taylor_expansion[2] + Rationnal(1,5) * pow(del,4) * taylor_expansion[4] + Rationnal(1,7) * pow(del,6) * taylor_expansion[6];
//         Ex p1 = taylor_expansion[1] + Rationnal(3,5) * pow(del,2) * taylor_expansion[3] + Rationnal(3,7) * pow(del,4) * taylor_expansion[5];
//         Ex roots[] = { -p1 / p0 };
//         #endif
//
//         // special case ( the last one ) : only one root
//         Ex cut = mid + roots[0];
//         Ex subs_p, subs_n;
//         if ( disc->type == STRING_heaviside_NUM ) {
//             subs_n = expr.subs( th, tok, ex_disc, Ex( 0 ) );
//             subs_p = expr.subs( th, tok, ex_disc, Ex( 1 ) );
//         }
//         else if ( disc->type == STRING_abs_NUM ) {
//             subs_n = expr.subs( th, tok, ex_disc, - ex_ch );
//             subs_p = expr.subs( th, tok, ex_disc,   ex_ch );
//         }
//         else if ( disc->type == STRING_pos_part_NUM ) {
//             subs_n = expr.subs( th, tok, ex_disc, Ex( 0 ) );
//             subs_p = expr.subs( th, tok, ex_disc,   ex_ch );
//         }
//         else
//             assert( 0 ); //
//
//         //
//         Ex p_beg = heaviside( ex_ch.subs( th, tok, var, beg ) );
//         Ex p_end = heaviside( ex_ch.subs( th, tok, var, end ) );
//         Ex n_beg = 1 - p_beg;
//         Ex n_end = 1 - p_end;
//
//         //
//         Ex nb = beg + ( cut - beg ) * p_beg * n_end;
//         Ex ne = end + ( beg - end + ( cut - beg ) * n_beg ) * p_end;
//         Ex pb = beg + ( end - beg + ( cut - end ) * p_end ) * n_beg;
//         Ex pe = end + ( cut - end ) * p_beg * n_end;
//
//         Ex int_n = integration( th, tok, subs_n, var, nb, ne, deg_poly_max );
//         Ex int_p = integration( th, tok, subs_p, var, pb, pe, deg_poly_max );
//         return int_n + int_p;
//     }
//     return integration_with_taylor_expansion( th, tok, expr, var, beg, end, deg_poly_max );
// }

Ex integ_discontinuities_rec ( Thread *th, const void *tok, const SEX &taylor_expansion, const Ex &var, const Ex &cen, const Ex &beg, const Ex &end, int level = 0 ) {
//     // look up for a discontinuity
//     SimpleVector<Op *> discontinuities;
//     ++Op::current_op;
//     for(unsigned i=0;i<taylor_expansion.size();++i)
//         taylor_expansion[i].op->find_discontinuity_rec( var.op, discontinuities );

    // found one
//     if ( disc ) {
//         const Op *ch = disc->func_data()->children[0];
//         Ex ex_disc( disc ), ex_ch( ch );
//         std::cout << level << " " << ex_disc << std::endl;
    //
//         //         // taylor_expansion of child (to find the roots)
//         //         SEX taylor_expansion;
//         //         Ex mid = Rationnal( 1, 2 ) * ( beg + end );
//         //         Ex del = Rationnal( 1, 2 ) * ( end - beg );
//         //         get_taylor_expansion( th, tok, ex_ch, mid, var, 7, taylor_expansion );
//         //         SEX poly_coeff; // best L2 fitting of order 7 -> order 3
//         //         poly_coeff.push_back( taylor_expansion[0] - Rationnal( 3,35) * pow(del,4) * taylor_expansion[4] - Rationnal( 2,21) * pow(del,6) * taylor_expansion[6] );
//         //         poly_coeff.push_back( taylor_expansion[1] - Rationnal( 5,21) * pow(del,4) * taylor_expansion[5] - Rationnal(10,33) * pow(del,6) * taylor_expansion[7] );
//         //         poly_coeff.push_back( taylor_expansion[2] + Rationnal( 6, 7) * pow(del,2) * taylor_expansion[4] + Rationnal( 5, 7) * pow(del,4) * taylor_expansion[6] );
//         //         poly_coeff.push_back( taylor_expansion[3] + Rationnal(10, 9) * pow(del,2) * taylor_expansion[5] + Rationnal(35,33) * pow(del,4) * taylor_expansion[7] );
    //         //
//         //         SEX roots, root_validity;
//         //         Ex leading_coefficient = get_roots_with_validity( poly_coeff, roots, root_validity );
    //         //
//         //         // several roots ?
//         //         if ( root_validity[1].op->is_zero()==false or root_validity[2].op->is_zero()==false ) {
//         //             Ex su = leading_coefficient;
//         //             for(unsigned i=0;i<roots.size();++i)
//         //                 if ( not root_validity[i].op->is_zero() )
//         //                     su *= root_validity[i] * ( var - ( mid + roots[ i ] ) - 1 ) + 1;
//         //             Ex new_expr = expr.subs( th, tok, ex_ch, su );
//         //             return integration( th, tok, new_expr, var, beg, end, deg_poly_max );
//         //         }
//         SEX ch_taylor_expansion;
//         Ex mid = Rationnal( 1, 2 ) * ( beg + end );
//         Ex del = Rationnal( 1, 2 ) * ( end - beg );
//         get_taylor_expansion( th, tok, ex_ch, mid, var, 7, ch_taylor_expansion );
//         /*
//             a := symbol("a")
//             c := Vec[Op,7]( function = x => symbol("c[$x]") )
//             v := Vec[Op,2]( function = x => symbol("v[$x]") )
//             p := dot( c, a ^ (0...) )
//             q := dot( v, a ^ (0...) )
//             del := symbol("del")
//             r := newton_raphson_minimize_iteration( integration( ( p - q ) ^ 2, a, -del, del, deg_poly_max = 7 ), v )
//             for i in c
//                 info i, r[0].diff(i)
//             for i in c
//                 info i, r[1].diff(i)
    //         */
//         // best L2 fitting of order 7 -> order 1
//         Ex p0 = ch_taylor_expansion[0] + Rationnal(1,3) * pow(del,2) * ch_taylor_expansion[2] + Rationnal(1,5) * pow(del,4) * ch_taylor_expansion[4] + Rationnal(1,7) * pow(del,6) * ch_taylor_expansion[6];
//         Ex p1 = ch_taylor_expansion[1] + Rationnal(3,5) * pow(del,2) * ch_taylor_expansion[3] + Rationnal(3,7) * pow(del,4) * ch_taylor_expansion[5];
    //
//         // end case : only one root
//         SEX taylor_expansion_p, taylor_expansion_n;
//         if ( disc->type == STRING_heaviside_NUM ) {
//             for(unsigned i=0;i<taylor_expansion.size();++i) {
//                 taylor_expansion_n.push_back( taylor_expansion[i].subs( th, tok, ex_disc, Ex( 0 ) ) );
//                 taylor_expansion_p.push_back( taylor_expansion[i].subs( th, tok, ex_disc, Ex( 1 ) ) );
//             }
//         }
//         else if ( disc->type == STRING_abs_NUM ) {
//             for(unsigned i=0;i<taylor_expansion.size();++i) {
//                 taylor_expansion_n.push_back( taylor_expansion[i].subs( th, tok, ex_disc, - ex_ch ) );
//                 taylor_expansion_p.push_back( taylor_expansion[i].subs( th, tok, ex_disc,   ex_ch ) );
//             }
//         }
//         else if ( disc->type == STRING_pos_part_NUM ) {
//             for(unsigned i=0;i<taylor_expansion.size();++i) {
//                 taylor_expansion_n.push_back( taylor_expansion[i].subs( th, tok, ex_disc, Ex( 0 ) ) );
//                 taylor_expansion_p.push_back( taylor_expansion[i].subs( th, tok, ex_disc,   ex_ch ) );
//             }
//         }
//         else
    //             assert( 0 ); //
    //
    //         //
//         Ex p_beg = heaviside( ex_ch.subs( th, tok, var, beg ) );
//         Ex p_end = heaviside( ex_ch.subs( th, tok, var, end ) );
//         Ex n_beg = 1 - p_beg;
//         Ex n_end = 1 - p_end;
    //
    //         //
//         Ex cut = mid - p0 / ( p1 + eqz( p1 ) );
//         Ex nb = beg + ( cut - beg ) * p_beg * n_end;
//         Ex ne = end + ( beg - end + ( cut - beg ) * n_beg ) * p_end;
//         Ex pb = beg + ( end - beg + ( cut - end ) * p_end ) * n_beg;
//         Ex pe = end + ( cut - end ) * p_beg * n_end;
    //
//         Ex int_n = integ_discontinuities_rec( th, tok, taylor_expansion_n, var, cen, nb , ne , level + 1 );
//         Ex int_p = integ_discontinuities_rec( th, tok, taylor_expansion_p, var, cen, pb , pe , level + 1 );
//         Ex int_z = integ_discontinuities_rec( th, tok, taylor_expansion_p, var, cen, beg, end, level + 1 );
//         return ( int_n + int_p ) * ( 1 - eqz( p1 ) ) + int_z * eqz( p1 );
//     }


    // else -> no discontinuities
    Ex res ( 0 );
    for ( Int32 i=0;i< ( Int32 ) taylor_expansion.size();++i )
        res = res + taylor_expansion[i].subs ( th, tok, var, cen ) * (
                  pow ( end - cen, Ex ( Rationnal ( i + 1 ) ) ) -
                  pow ( beg - cen, Ex ( Rationnal ( i + 1 ) ) )
              ) * Rationnal ( 1, i + 1 );
    return res;
}

Ex integration_disc_rec ( Thread *th, const void *tok, const SEX &taylor_expansion, const SEX &discontinuities, unsigned offset_in_discontinuities, const SEX &p0, const SEX &p1, const Ex &beg, const Ex &end ) {
    if ( offset_in_discontinuities == discontinuities.size() ) {
        // [ -deb; deb ]
        Ex res ( 0 );
        for ( Int32 i=0;i< ( Int32 ) taylor_expansion.size();++i ) {
            res = res + taylor_expansion[i] * (
                      pow ( end, Ex ( Rationnal ( i + 1 ) ) ) - pow ( beg, Ex ( Rationnal ( i + 1 ) ) )
                  ) * Rationnal ( 1, i + 1 );
        }
        return res;
    }

    //
    Ex cut = - p0[offset_in_discontinuities] / ( p1[offset_in_discontinuities] + eqz ( p1[offset_in_discontinuities] ) );
    Ex end_sup_beg = heaviside ( end - beg );
    Ex beg_ = min ( beg, end );
    Ex end_ = max ( beg, end );

    //
    Ex mon ( -1 );
    SEX neg_taylor_expansion;
    ++Op::current_op;
    discontinuities[offset_in_discontinuities].op->op_id = Op::current_op;
    discontinuities[offset_in_discontinuities].op->additional_info = mon.op;
    for ( unsigned i=0;i<taylor_expansion.size();++i ) {
        SubsRec<> sr ( th, tok, taylor_expansion[i] );
        neg_taylor_expansion.push_back ( taylor_expansion[i].op->additional_info );
    }
    Ex res = integration_disc_rec ( th, tok, neg_taylor_expansion, discontinuities, offset_in_discontinuities + 1, p0, p1, beg_, max ( beg_,min ( end_,cut ) ) );

    //
    Ex one ( 1 );
    SEX sup_taylor_expansion;
    ++Op::current_op;
    discontinuities[offset_in_discontinuities].op->op_id = Op::current_op;
    discontinuities[offset_in_discontinuities].op->additional_info = one.op;
    for ( unsigned i=0;i<taylor_expansion.size();++i ) {
        SubsRec<> sr ( th, tok, taylor_expansion[i] );
        sup_taylor_expansion.push_back ( taylor_expansion[i].op->additional_info );
    }
    res = res + integration_disc_rec ( th, tok, sup_taylor_expansion, discontinuities, offset_in_discontinuities + 1, p0, p1, max ( beg_,min ( end_,cut ) ), end_ );

    return res;
}

Ex integration_ ( Thread *th, const void *tok, Ex expr, Ex var, const Ex &beg, const Ex &end, Int32 deg_poly_max ) {
    if ( same_op ( beg.op, end.op ) )
        return Ex ( 0 );

    // add interval assumptions
    if ( beg.known_at_compile_time() or end.known_at_compile_time() ) {
        Ex old_var = var;
        var = Ex ( "tmp_end_beg_known", 17, "tmp_end_beg_known", 17 );
        if ( beg.known_at_compile_time() )
            var.set_beg_value ( th, tok, beg.value(), true );
        if ( end.known_at_compile_time() )
            var.set_end_value ( th, tok, end.value(), true );
        expr = expr.subs ( th, tok, old_var, var );
    }

    // degree of polynomial_expansion
    Int32 pd = expr.poly_deg ( var );
    Int32 deg_poly = ( pd < 0 ? deg_poly_max : std::min ( pd, deg_poly_max ) );

    // polynomial_expansion
    Ex mid = ( beg + end ) / 2;
    Ex deb = ( end - beg ) / 2;
    Ex exc ( "tmp",3,"tmp",3 );
    expr = expr.subs ( th, tok, var, mid + exc );
    SEX expressions, taylor_expansion;
    expressions.push_back ( expr );
    taylor_expansion.get_room_for ( deg_poly + 1 );
    polynomial_expansion ( th, tok, expressions, exc, deg_poly, taylor_expansion );

    // look up for discontinuities
    SimpleVector<Op *> discontinuities_;
    expr.op->find_discontinuities ( exc.op, discontinuities_ );
    if ( discontinuities_.size() ) {
        SEX discontinuities;
        for ( unsigned i=0;i<discontinuities_.size();++i ) {
            Ex d ( discontinuities_[i] );
            for ( unsigned j=0;;++j ) {
                if ( j == discontinuities.size() ) {
                    discontinuities.push_back ( d );
                    break;
                }
                if ( same_ex ( - d, discontinuities[ j ] ) )
                    break;
            }
        }

        // cuts -> linearization of all f(a) in H( f( a ) )
        const int nb_terms_taylor_ch = 7;
        SEX ch_taylor_expansion;
        ch_taylor_expansion.get_room_for ( nb_terms_taylor_ch * discontinuities.size() );
        polynomial_expansion ( th, tok, discontinuities, exc, nb_terms_taylor_ch - 1, ch_taylor_expansion );

        SEX p0, p1;
        for ( unsigned i=0,j=0;i<discontinuities.size();++i,j+=nb_terms_taylor_ch ) {
            /*
                                             a := symbol("a")
                                             c := Vec[Op,7]( function = x => symbol("c[$x]") )
                                             v := Vec[Op,2]( function = x => symbol("v[$x]") )
                                             p := dot( c, a ^ (0...) )
                                             q := dot( v, a ^ (0...) )
                                             del := symbol("del")
                                             r := newton_raphson_minimize_iteration( integration( ( p - q ) ^ 2, a, -del, del, deg_poly_max = 7 ), v )
                                             for i in c
                                             info i, r[0].diff(i)
                                             for i in c
                                             info i, r[1].diff(i)
            */
            // best L2 fitting of order 7 -> order 1
            p0.push_back ( ch_taylor_expansion[j+0] + Rationnal ( 1,3 ) * pow ( deb,2 ) * ch_taylor_expansion[j+2] + Rationnal ( 1,5 ) * pow ( deb,4 ) * ch_taylor_expansion[j+4] + Rationnal ( 1,7 ) * pow ( deb,6 ) * ch_taylor_expansion[j+6] );
            p1.push_back ( ch_taylor_expansion[j+1] + Rationnal ( 3,5 ) * pow ( deb,2 ) * ch_taylor_expansion[j+3] + Rationnal ( 3,7 ) * pow ( deb,4 ) * ch_taylor_expansion[j+5] );
        }

        //
        return integration_disc_rec ( th, tok, taylor_expansion, discontinuities, 0, p0, p1, -deb, deb );
    }

    //
    Ex res ( 0 );
    for ( Int32 i=0;i< ( Int32 ) taylor_expansion.size();i+=2 )
        res = res + 2 * taylor_expansion[i] * (
                  pow ( deb, Ex ( Rationnal ( i + 1 ) ) )
              ) * Rationnal ( 1, i + 1 );
    return res;
}

SEX subs ( Thread *th, const void *tok, const SEX &v, const Ex &from, const Ex &to, Int32 inc = 1 ) {
    SEX res;
    //
    ++Op::current_op;
    from.op->op_id = Op::current_op;
    from.op->additional_info = to.op;
    for ( unsigned i=0;i<v.size(); i += inc ) {
        SubsRec<> sr ( th, tok, v[i] );
        res.push_back ( v[i].op->additional_info );
    }
    return res;
}

Ex integration ( Thread *th, const void *tok, Ex expr, Ex var, const Ex &beg, const Ex &end, Int32 deg_poly_max ) {
    if ( same_op ( beg.op, end.op ) )
        return Ex ( 0 );

    // add interval assumptions
    // PRINT ( "add interval assumptions" );
    if ( beg.known_at_compile_time() and end.known_at_compile_time() ) {
        Rationnal b = std::min ( beg.value(), end.value() );
        Rationnal e = std::max ( beg.value(), end.value() );
        //
        if ( var.beg_value_valid() ==false or var.beg_value() < b or var.end_value_valid() ==false or var.end_value() > e ) {
            Ex old_var = var;
            var = Ex ( "tmp_end_beg_known", 17, "tmp_end_beg_known", 17 );
            if ( var.beg_value_valid() )
                var.set_beg_value ( th, tok, std::max ( b, var.beg_value() ), false );
            else
                var.set_beg_value ( th, tok, b, false );
            //
            if ( var.end_value_valid() )
                var.set_end_value ( th, tok, std::min ( e, var.end_value() ), false );
            else
                var.set_end_value ( th, tok, e, false );
            expr = expr.subs ( th, tok, old_var, var );
        }
    }

    //
    Ex mid = ( beg + end ) / 2;
    Ex off = ( end - beg ) / 2;

    // degree of polynomial_expansion
    // PRINT ( "degree of polynomial_expansion" );
    Int32 pd = expr.poly_deg ( var );
    Int32 deg_poly = ( pd < 0 ? deg_poly_max : std::min ( pd, deg_poly_max ) );
    SEX expressions, taylor_expansion;
    expressions.push_back ( expr );
    taylor_expansion.get_room_for ( deg_poly + 1 );

    // look up for discontinuities
    // PRINT ( "look up for discontinuities" );
    SimpleVector<Op *> discontinuities_;
    expr.op->find_discontinuities ( var.op, discontinuities_ );
    if ( discontinuities_.size() ) {
        SEX discontinuities;
        for ( unsigned i=0;i<discontinuities_.size();++i ) {
            Ex d ( discontinuities_[i] );
            for ( unsigned j=0;;++j ) {
                if ( j == discontinuities.size() ) {
                    discontinuities.push_back ( d );
                    break;
                }
                if ( same_ex ( - d, discontinuities[ j ] ) )
                    break;
            }
        }

        // cuts -> linearization of all f(a) in H( f( a ) )
        // PRINT ( "cuts -> linearization of all f(a) in H( f( a ) )" );
        const int nb_terms_taylor_ch = 7;
        SEX ch_taylor_expansion;
        ch_taylor_expansion.get_room_for ( nb_terms_taylor_ch * discontinuities.size() );
        polynomial_expansion ( th, tok, discontinuities, var, nb_terms_taylor_ch - 1, ch_taylor_expansion, mid );

        Ex end_supeq_beg = ( end >= beg );

        SEX cut_pos, cut_val;
        cut_pos.push_back ( beg );
        cut_val.push_back ( 1 );
        for ( unsigned i=0,j=0;i<discontinuities.size();++i,j+=nb_terms_taylor_ch ) {
            /*
                a := symbol("a")
                c := Vec[Op,7]( function = x => symbol("c[$x]") )
                v := Vec[Op,2]( function = x => symbol("v[$x]") )
                p := dot( c, a ^ (0...) )
                q := dot( v, a ^ (0...) )
                del := symbol("del")
                r := newton_raphson_minimize_iteration( integration( ( p - q ) ^ 2, a, -del, del, deg_poly_max = 7 ), v )
                for i in c
                info i, r[0].diff(i)
                for i in c
                info i, r[1].diff(i)
            */
            // best L2 fitting of order 7 -> order 1 // a + Rationnal( 1,7 ) * pow( off,6 ) * ch_taylor_expansion[j+6]
            Ex a = ch_taylor_expansion[j+0] + Rationnal( 1,3 ) * pow( off,2 ) * ch_taylor_expansion[j+2] + Rationnal( 1,5 ) * pow( off,4 ) * ch_taylor_expansion[j+4];
            Ex b = ch_taylor_expansion[j+1] + Rationnal( 3,5 ) * pow( off,2 ) * ch_taylor_expansion[j+3] + Rationnal( 3,7 ) * pow( off,4 ) * ch_taylor_expansion[j+5];
            Ex cp = mid - a / ( b + eqz ( b ) );
            Ex va = ( 1 - eqz ( b ) ) * ( 1 - ( end > cp ) * ( beg >= cp ) - ( cp >= beg ) * ( cp > end ) );
            for ( unsigned c=0;;++c ) {
                if ( c == cut_pos.size() ) {
                    cut_pos.push_back ( cp );
                    cut_val.push_back ( va );
                    break;
                }
                if ( assumed ( cut_pos[ c ] == cp ) ) {
                    cut_val[ c ] += ( 1 - cut_val[ c ] ) * va;
                    break;
                }
            }
        }
        cut_pos.push_back ( end );
        cut_val.push_back ( 1 );

        // polynomial_expansion
        // PRINT ( "polynomial_expansion" );
        if ( deg_poly < 7 ) {
            polynomial_expansion ( th, tok, expressions, var, deg_poly, taylor_expansion, var );
        } else {
            taylor_expansion.clear();
            get_taylor_expansion_manual ( th, tok, expr, var, var, deg_poly, taylor_expansion );
        }

        //
        Ex res;
        for ( unsigned num_cut_0=0;num_cut_0<cut_pos.size();++num_cut_0 ) {
            for ( unsigned num_cut_1=num_cut_0+1;num_cut_1<cut_pos.size();++num_cut_1 ) {
                Ex mid_cut = ( cut_pos[ num_cut_1 ] + cut_pos[ num_cut_0 ] ) / 2;
                Ex off_cut = ( cut_pos[ num_cut_1 ] - cut_pos[ num_cut_0 ] ) / 2;
                //
                Ex valid = cut_val[ num_cut_0 ] * cut_val[ num_cut_1 ];
                for ( unsigned b=1;b+1<cut_pos.size();++b ) {
                    if ( b != num_cut_0 and b != num_cut_1 ) {
                        valid = valid * (
                            cut_val[ b ] * (
                                ( b < num_cut_0 ? cut_pos[ b ] > cut_pos[ num_cut_0 ] : cut_pos[ b ] >= cut_pos[ num_cut_0 ] ) * ( b < num_cut_1 ? cut_pos[ b ] > cut_pos[ num_cut_1 ] : cut_pos[ b ] >= cut_pos[ num_cut_1 ] ) +
                                ( b < num_cut_0 ? cut_pos[ b ] < cut_pos[ num_cut_0 ] : cut_pos[ b ] <= cut_pos[ num_cut_0 ] ) * ( b < num_cut_1 ? cut_pos[ b ] < cut_pos[ num_cut_1 ] : cut_pos[ b ] <= cut_pos[ num_cut_1 ] )
                            ) +
                            1 - cut_val[ b ]
                        );
                    }
                }
                //
                SEX taylor_expansion_subs = subs ( th, tok, taylor_expansion, var, mid_cut, 2 );
                //
                Ex tmp;
                for ( Int32 i=0;i< ( Int32 ) taylor_expansion.size();i+=2 )
                    tmp = tmp + 2 * taylor_expansion_subs[ i / 2 ] * (
                              pow ( off_cut, Ex ( Rationnal ( i + 1 ) ) )
                          ) * Rationnal ( 1, i + 1 );
                // std::cout << "valid=" << valid << " cp0=" << cut_pos[num_cut_0] << " cp1=" << cut_pos[num_cut_1] << " tmp=" << tmp << " num_cut_0=" << num_cut_0 << " num_cut_1=" << num_cut_1 << std::endl;
                //
                if ( num_cut_0 != 0 and num_cut_1 + 1 != cut_pos.size() )
                    valid *= sgn ( cut_pos[ num_cut_1 ] - cut_pos[ num_cut_0 ] ) * sgn ( end - beg );
                //
                res += tmp * valid;
            }
        }
        return res;
    }

    // else -> simple polynomial_expansion
    // PRINT ( "else -> simple polynomial_expansion" );
    if ( deg_poly < 7 ) {
        polynomial_expansion ( th, tok, expressions, var, deg_poly, taylor_expansion, mid );
    } else {
        taylor_expansion.clear();
        get_taylor_expansion_manual ( th, tok, expr, mid, var, deg_poly, taylor_expansion );
    }

    //
    Ex res ( 0 );
    for ( Int32 i=0;i< ( Int32 ) taylor_expansion.size();i+=2 )
        res = res + 2 * taylor_expansion[i] * (
                  pow ( off, Ex ( Rationnal ( i + 1 ) ) )
              ) * Rationnal ( 1, i + 1 );
    return res;
}

