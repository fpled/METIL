#include "op.h"
#include "basic_string_manip.h"
#include "nstring.h"
#include "thread.h"
#include "varargs.h"
#include "globaldata.h"

Op &Op::new_number( const Rationnal &val ) {
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(NumberData) ); res->cpt_use = 0; res->type = NUMBER; res->parents.init(); res->cleared_id = 0; res->integer_type = false;
    
    init_arithmetic( res->number_data()->val, val );
    return *res;
}

Op &Op::new_symbol( const char *cpp_name_str, unsigned cpp_name_si, const char *tex_name_str, unsigned tex_name_si ) {
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(SymbolData) ); res->cpt_use = 0; res->type = SYMBOL; res->parents.init(); res->cleared_id = 0; res->integer_type = false;
    
    SymbolData *ds = res->symbol_data();
    ds->cpp_name_str = strdupp0( cpp_name_str, cpp_name_si );
    ds->tex_name_str = strdupp0( tex_name_str, tex_name_si );
    return *res;
}

inline bool are_numbers( const Op &a, const Op &b ) { return a.type == Op::NUMBER and b.type == Op::NUMBER; }
inline bool same_op( Op *a, Op *b ) {
    if ( are_numbers(*a,*b) )
        return a->number_data()->val == b->number_data()->val;
    return a == b;
}

Op &Op::new_function( int type, Op &a ) {
    assert( a.type != Op::NUMBER );
    
    // look in parents of a or b if function already created somewhere
    for(unsigned i=0;i<a.parents.size();++i) {
        Op *p = a.parents[ i ];
        if ( p->type == type and same_op(&a,p->func_data()->children[0]) ) {
            p->inc_ref();
            return *p;
        }
    }
    //
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(FuncData) ); res->cpt_use = 0; res->type = type; res->parents.init(); res->cleared_id = 0; res->integer_type = false;
    
    FuncData *ds = res->func_data();
    ds->children[0] = &a.inc_ref();
    ds->children[1] = NULL;
    
    if ( type == STRING_sub_NUM )
        res->integer_type = a->integer_type;
    
    if ( a.type != Op::NUMBER ) a.parents.push_back( res );
    return *res;
}

Op &Op::new_function( int type, Op &a, Op &b ) {
    assert( a.type != Op::NUMBER or b.type != Op::NUMBER );
    if ( type == STRING_pow_NUM )
        assert( not b.is_one() );
    
    // look in parents of a or b if function already created somewhere
    Op *c = ( a.type != Op::NUMBER ? &a : &b );
    for(unsigned i=0;i<c->parents.size();++i) {
        Op *p = c->parents[ i ];
        if ( p->type == type and same_op(&a,p->func_data()->children[0]) and same_op(&b,p->func_data()->children[1]) ) {
            p->inc_ref();
            return *p;
        }
    }
    //
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(FuncData) ); res->cpt_use = 0; res->type = type; res->parents.init(); res->cleared_id = 0; res->integer_type = false;
    
    FuncData *ds = res->func_data();
    ds->children[0] = &a.inc_ref();
    ds->children[1] = &b.inc_ref();
    
    if ( a.type != Op::NUMBER ) a.parents.push_back( res );
    if ( b.type != Op::NUMBER ) b.parents.push_back( res );
    return *res;
}

void Op::destroy() {
    assert( parents.size() == 0 );
    parents.destroy();
    if ( type == SYMBOL ) {
        free( symbol_data()->cpp_name_str );
        free( symbol_data()->tex_name_str );
    } else if ( type == NUMBER ) {
        number_data()->val.destroy();
    } else { // function
        for(unsigned i=0;i<Op::FuncData::max_nb_children and func_data()->children[i];++i) {
            if ( func_data()->children[i]->type != Op::NUMBER )
                func_data()->children[i]->parents.erase_elem_in_unordered_list( this );
            dec_ref( func_data()->children[i] );
        }
    }
}
    
bool Op::is_zero     () { return type==NUMBER and number_data()->val.is_zero(); }
bool Op::is_one      () { return type==NUMBER and number_data()->val.is_one(); }
bool Op::is_minus_one() { return type==NUMBER and number_data()->val.is_minus_one(); }

static unsigned clear_id_clear_additional_info_rec = 0;
void Op::clear_additional_info_rec() {
    clear_additional_info_rec( ++clear_id_clear_additional_info_rec );
}

void Op::clear_additional_info_rec( unsigned clear_id ) {
    if ( cleared_id == clear_id )
        return;
        
    cleared_id = clear_id;
    additional_info = NULL;
    if ( type > 0 )
        for(unsigned i=0;i<FuncData::max_nb_children and func_data()->children[i];++i)
            func_data()->children[i]->clear_additional_info_rec( clear_id );
}

void graphviz_repr_rec( std::ostream &os, Op *op ) {
    if ( op->additional_info ) return; // already outputted ?
    op->additional_info = op;
    
    if ( op->type == Op::NUMBER )
        os << "    node" << op << " [label=\"" << op->number_data()->val << "\"];\n";
    else if ( op->type == Op::SYMBOL ) {
        os << "    node" << op << " [label=\"" << op->symbol_data()->cpp_name_str << "\"];\n";
    } else {
        os << "    node" << op << " [label=\"" << Nstring( op->type ) << op-> << "\"];\n";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            os << "    node" << op << " -> node" << op->func_data()->children[i] << " [color=black];\n";
            graphviz_repr_rec( os, op->func_data()->children[i] );
        }
    }
}
void graphviz_repr( std::ostream &os, Op *op ) {
    op->clear_additional_info_rec();
    graphviz_repr_rec( os, op );
}

// std::string tex_repr( const Rationnal &val, bool want_par = false, bool assume_par_not_needed = false ) {
//     std::ostringstream os;
//     //     want_par &= ( val.den != Rationnal::BI(1) );
//     if ( want_par and assume_par_not_needed==false ) os << "(";
//     if ( val.den == Rationnal::BI(1) ) os << val.num;
//     else os << "\\frac{" << val.num << "}{" << val.den << "}";
//     if ( want_par and assume_par_not_needed==false ) os << ")";
//     return os.str();
// }
// 
// void tex_repr_rec( std::ostream &os, Op *op, int type_parent ) {
//     if ( op->type == Op::NUMBER ) {
//         os << tex_repr( op->number_data()->val );
//     } else if ( op->type == Op::SYMBOL ) {
//         os << op->symbol_data()->tex_name_str;
//     } else {
//         os << Nstring(op->type) << "(";
//         for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
//             if ( i ) os << ",";
//             tex_repr_rec( os, op->func_data()->children[i], 0 );
//         }
//         os << ")";
//     }
// }


std::string cpp_repr( const Rationnal &val, int type_parent ) {
    std::ostringstream os;
    int local_type = 10000;
    if ( val.num.is_negative() )
        local_type = STRING_sub_NUM;
    else if ( val.den != Rationnal::BI(1) )
        local_type = STRING_div_NUM;
    bool want_par = type_parent > local_type;
    
    if ( want_par ) os << "(";
    if ( val.den == Rationnal::BI(1) ) os << val.num;
    else os << val.num << ".0/" << val.den << ".0";
    if ( want_par ) os << ")";
    return os.str();
}

// 
void find_p_and_s_parts_rec( Op *op, std::vector<Op *> &p_part, std::vector<Op *> &s_part ) {
    if ( op->type == Op::NUMBER and op->number_data()->val.is_neg() )
        s_part.push_back( op );
    else if ( op->type != STRING_add_NUM )
        p_part.push_back( op );
    else {
        if ( is_a_mul_by_neg_number( *op->func_data()->children[0] ) ) // ( -2 ) * a
            s_part.push_back( op->func_data()->children[0] );
        else
            find_p_and_s_parts_rec( op->func_data()->children[0], p_part, s_part );
        //
        if ( is_a_mul_by_neg_number( *op->func_data()->children[1] ) ) // ( -2 ) * a
            s_part.push_back( op->func_data()->children[1] );
        else
            find_p_and_s_parts_rec( op->func_data()->children[1], p_part, s_part );
    }
}

void cpp_repr_rec( std::ostream &os, Op *op, int type_parent ) {
    if ( op->type == Op::NUMBER ) {
        os << cpp_repr( op->number_data()->val, type_parent );
    } else if ( op->type == Op::SYMBOL ) {
        os << op->symbol_data()->cpp_name_str;
    } else if ( op->type == STRING_add_NUM ) { // (...) + (...)
        bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
        
        std::vector<Op *> p_part;
        std::vector<Op *> s_part;
        find_p_and_s_parts_rec( op, p_part, s_part );
    
        for(unsigned i=0;i<p_part.size();++i) {
            if ( i ) os << "+";
            cpp_repr_rec( os, p_part[i], STRING_add_NUM );
        }
        for(unsigned i=0;i<s_part.size();++i) {
            os << "-";
            if ( s_part[i]->type == Op::NUMBER ) // + (-2)
                os << cpp_repr( - s_part[i]->number_data()->val, STRING_add_NUM );
            else { // ( -2 ) * a
                Op *ch_0 = s_part[i]->func_data()->children[0];
                Op *ch_1 = s_part[i]->func_data()->children[1];
                if ( ch_0->number_data()->val.is_minus_one() )
                    cpp_repr_rec( os, ch_1, STRING_sub_NUM );
                else {
                    os << cpp_repr( - ch_0->number_data()->val, STRING_mul_NUM );
                    os << "*";
                    cpp_repr_rec( os, ch_1, STRING_mul_NUM );
                }
            }
        }
        
        if ( np ) os << ")";
        /*if ( is_a_sub( *op->func_data()->children[1] ) ) { // a + ( -b )
            bool np = type_parent > STRING_sub_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[0], STRING_add_NUM );
            os << "-";
            cpp_repr_rec( os, op->func_data()->children[1]->func_data()->children[1], STRING_sub_NUM );
            if ( np ) os << ")";
        } else if ( is_a_sub( *op->func_data()->children[0] ) ) { // (-a) + b
            bool np = type_parent > STRING_sub_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[1], STRING_add_NUM );
            os << "-";
            cpp_repr_rec( os, op->func_data()->children[0]->func_data()->children[1], STRING_sub_NUM );
            if ( np ) os << ")";
        } else if ( op->func_data()->children[0]->type == Op::NUMBER and op->func_data()->children[0]->number_data()->val.num.is_negative() ) { // (-2) + b
            bool np = type_parent > STRING_sub_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[1], STRING_add_NUM ); 
            os << "-" << cpp_repr( - op->func_data()->children[0]->number_data()->val, STRING_sub_NUM );
            if ( np ) os << ")";
        } else { // a + b
            bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[0], STRING_add_NUM );
            os << "+";
            cpp_repr_rec( os, op->func_data()->children[1], STRING_add_NUM );
            if ( np ) os << ")";
        }
        */
    } else if ( op->type == STRING_mul_NUM ) { // (...) * (...)
        if ( is_a_inv( *op->func_data()->children[1] ) ) { // a * ( 1/b )
            bool np = type_parent > STRING_div_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[0], STRING_mul_NUM );
            os << "/";
            cpp_repr_rec( os, op->func_data()->children[1]->func_data()->children[0], STRING_div_NUM );
            if ( np ) os << ")";
        } else if ( is_a_inv( *op->func_data()->children[0] ) ) { // (1/a) * b
            bool np = type_parent > STRING_div_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[1], STRING_mul_NUM );
            os << "/";
            cpp_repr_rec( os, op->func_data()->children[0]->func_data()->children[0], STRING_div_NUM );
            if ( np ) os << ")";
        } else { // a * b
            bool np = type_parent > STRING_mul_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[0], STRING_mul_NUM );
            os << "*";
            cpp_repr_rec( os, op->func_data()->children[1], STRING_mul_NUM );
            if ( np ) os << ")";
        }
    } else if ( is_a_sub( *op ) ) { // -a
        assert( op->func_data()->children[1] == NULL );
        bool np = type_parent > op->type;
        if ( np ) os << "(";
        os << "-";
        cpp_repr_rec( os, op->func_data()->children[1], op->type );
        if ( np ) os << ")";
    } else {
        os << Nstring(op->type) << "(";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            if ( i ) os << ",";
            cpp_repr_rec( os, op->func_data()->children[i], 0 );
        }
        os << ")";
    }
}

void cpp_repr( std::ostream &os, Op *op ) {
    cpp_repr_rec( os, op, 0 );
}

bool Op::necessary_positive_or_null() {
    return ( type == NUMBER and number_data()->val.positive_or_null() ) or
           type == STRING_abs_NUM or
           type == STRING_eqz_NUM or
           type == STRING_exp_NUM or
           type == STRING_cosh_NUM or
           type == STRING_heaviside_NUM;
}

Op &add_number_and_expr( Op &a, Op &b ) { // a is a number
    if ( a.number_data()->val.is_zero() )
        return b.inc_ref();
    // 10 + ( 5 + a )
    if ( b.type == STRING_add_NUM and b.func_data()->children[0]->type == Op::NUMBER ) {
        Op &new_num = Op::new_number( a.number_data()->val + b.func_data()->children[0]->number_data()->val );
        return new_num + *b.func_data()->children[1];
        //         return Op::new_function( STRING_add_NUM, new_num, *b.func_data()->children[1] );
    }
    return Op::new_function( STRING_add_NUM, a, b );
}

struct SumSeq {
    Rationnal c;
    Op *op;
    Op &to_op() {
        if ( c.is_one() )
            return *op;
        return Op::new_number(c) * *op;
    }
};

Op &make_add_seq( SplittedVec<SumSeq,4,16,true> &items_c ) {
    if ( not items_c.size() )
        return Op::new_number( 0 );
    //
    Op *res = &items_c[0].to_op(); res->inc_ref();
    for(unsigned i=1;i<items_c.size();++i) {
        res = &( *res + items_c[i].to_op() );
        //         if ( res < items_c[i].op )
        //             res = &Op::new_function( STRING_add_NUM, *res, items_c[i].to_op() );
        //         else
        //             res = &Op::new_function( STRING_add_NUM, items_c[i].to_op(), *res );
    }
    return *res;
}

/// @see operator+
void find_add_items_and_coeff_rec( Op *a, SplittedVec<SumSeq,4,16,true> &items ) {
    if ( a->type == STRING_add_NUM ) { // anything_but_a_number + anything_but_a_number
        find_add_items_and_coeff_rec( a->func_data()->children[0], items );
        find_add_items_and_coeff_rec( a->func_data()->children[1], items );
    } else if ( a->type == STRING_mul_NUM and a->func_data()->children[0]->type == Op::NUMBER ) { // 10 * a
        SumSeq *s = items.new_elem();
        s->c.init( a->func_data()->children[0]->number_data()->val );
        s->op = a->func_data()->children[1];
    } else { // a
        SumSeq *s = items.new_elem();
        s->c.init( 1 );
        s->op = a;
    }
}
/// @see operator+
void add_items_and_coeffs( const SplittedVec<SumSeq,4,16,true> &items_a, SplittedVec<SumSeq,4,16,true> &items_b ) {
    for(unsigned i=0;i<items_a.size();++i) {
        for(unsigned j=0;;++j) {
            if ( j == items_b.size() ) {
                items_b.push_back( items_a[i] );
                break;
            } 
            if ( items_a[ i ].op == items_b[ j ].op ) {
                items_b[ j ].c += items_a[ i ].c;
                if ( not items_b[ j ].c )
                    items_b.erase_elem_nb_in_unordered_list( j );
                break;
            }
        }
    }
}

Op &operator+( Op &a, Op &b ) {
    // 10 + ...
    if ( a.type == Op::NUMBER ) {
        // 10 + 20
        if ( b.type == Op::NUMBER )
            return Op::new_number( a.number_data()->val + b.number_data()->val );
        // 10 + ( ... )
        return add_number_and_expr( a, b );
    }
    // ( not a number ) + 10 -> 10 + ( not a number )
    if ( b.type == Op::NUMBER )
        return add_number_and_expr( b, a );
        
    // ( 10 + a ) + ( not a number ) -> 10 + ( a + ( not a number ) )
    if ( a.type == STRING_add_NUM and a.func_data()->children[0]->type == Op::NUMBER ) {
        // ( 10 + a ) + ( 20 + b ) -> 30 + ( a + b )
        if ( b.type == STRING_add_NUM and b.func_data()->children[0]->type == Op::NUMBER ) {
            Op &new_num = Op::new_number( a.func_data()->children[0]->number_data()->val + b.func_data()->children[0]->number_data()->val );
            Op &new_sum = *a.func_data()->children[1] + *b.func_data()->children[1];
            return new_num + new_sum;
        }
        // ( 10 + a ) + b -> 10 + ( a + b )
        Op &new_sum = *a.func_data()->children[1] + b;
        return *a.func_data()->children[0] + new_sum; // add_number_and_expr( *a.func_data()->children[0], new_sum );
    }
    // a + ( 10 + b ) -> 10 + ( a + b )
    if ( b.type == STRING_add_NUM and b.func_data()->children[0]->type == Op::NUMBER ) {
        Op &new_sum = a + *b.func_data()->children[1];
        return *b.func_data()->children[0] + new_sum;
    }

    // ( 10 * a + b + anything_but_a_number ) + ( 2 * b + a + anything_but_a_number )
    SplittedVec<SumSeq,4,16,true> items_a, items_b;
    find_add_items_and_coeff_rec( &a, items_a );
    find_add_items_and_coeff_rec( &b, items_b );
    unsigned old_size = items_a.size() + items_b.size();
    add_items_and_coeffs( items_b, items_a );
    if ( old_size != items_a.size() )
        return make_add_seq( items_a );
    
    //
    if ( &a < &b )
        return Op::new_function( STRING_add_NUM, a, b );
    return Op::new_function( STRING_add_NUM, b, a );
}

Op &operator-( Op &a ) {
    if ( a.type == Op::NUMBER )
        return Op::new_number( - a.number_data()->val );
    if ( a.type == STRING_add_NUM and a.func_data()->children[0]->type == Op::NUMBER ) { // - ( 1 + a ) -> -1 + (-a)
        Op &new_num = Op::new_number( - a.func_data()->children[0]->number_data()->val );
        return new_num + ( - *a.func_data()->children[1] );
    }
    if ( a.type == STRING_mul_NUM and a.func_data()->children[0]->type == Op::NUMBER ) { // - 10 * a -> (-10) * a
        Op &new_num = Op::new_number( - a.func_data()->children[0]->number_data()->val );
        return new_num * *a.func_data()->children[1];
    }
    return Op::new_number(-1) * a; // (-1) * a
}

Op &operator-( Op &a, Op &b ) {
    return a + (-b);
}

Op &mul_number_and_expr( Op &a, Op &b ) { // a is a number
    if ( a.number_data()->val.is_one() )
        return b.inc_ref();
    if ( a.number_data()->val.is_zero() )
        return a.inc_ref();
    // 10 * ( 5 * a )
    if ( b.type == STRING_mul_NUM and b.func_data()->children[0]->type == Op::NUMBER ) {
        Op &new_num = Op::new_number( a.number_data()->val * b.func_data()->children[0]->number_data()->val );
        return mul_number_and_expr( new_num, *b.func_data()->children[1] );
        //         return new_num * *b.func_data()->children[1];
        //         return Op::new_function( STRING_mul_NUM, new_num, *b.func_data()->children[1] );
    }
    // 10 * ( a + 5 * b ) -> 10 * a + 50 * b
    if ( b.type == STRING_add_NUM ) {
        SplittedVec<SumSeq,4,16,true> items;
        find_add_items_and_coeff_rec( b.func_data()->children[0], items );
        find_add_items_and_coeff_rec( b.func_data()->children[1], items );
        for(unsigned i=0;i<items.size();++i)
            items[i].c *= a.number_data()->val;
        return make_add_seq( items );
    }
    
    //
    return Op::new_function( STRING_mul_NUM, a, b );
}

struct MulSeq {
    Rationnal e;
    Op *op;
    //
    Op &to_op() {
        if ( e.is_one() )
            return *op;
        return pow( *op, Op::new_number(e) );
    }
    void tex_repr( std::ostream &os ) {
        bool np = ( STRING_mul_NUM > op->type ) and op->type >= 0;
        if (np) os << "(";
        if ( e.is_one() ) {
            if ( op->type == Op::NUMBER and op->number_data()->val.is_minus_one() )
                os << "-";
            else
                ::tex_repr( os, op );
        }
        else if ( e.num.is_one() ) {
            if ( e.den.is_two() )
                os << "\\sqrt{";
            else    
                os << "\\sqrt[" << e.den << "]{";
            ::tex_repr( os, op );
            os << "}";
        } else {
            os << "{";
            ::tex_repr( os, op );
            os << "}";
            if ( e.is_integer() )
                os << "^{" << e.num << "} ";
            else
                os << "^{ \\frac{" << e.num << "}{" << e.den << "} } ";
        }
        if (np) os << ")";
        os << " ";
    }
};

Op &make_mul_seq( SplittedVec<MulSeq,4,16,true> &items_c ) {
    if ( not items_c.size() )
        return Op::new_number( 1 );
    //
    Op *res = &items_c[0].to_op(); res->inc_ref();
    Rationnal n = 1;
    for(unsigned i=1;i<items_c.size();++i) {
        if ( items_c[i].op->type == Op::NUMBER )
            n *= items_c[i].to_op().number_data()->val;
        else
            res = &( *res * items_c[i].to_op() );
        // else if ( res < items_c[i].op )
        //     res = &Op::new_function( STRING_mul_NUM, *res, items_c[i].to_op() ); // &( *res * items_c[i].to_op() ); // 
        // else
        //     res = &Op::new_function( STRING_mul_NUM, items_c[i].to_op(), *res ); // &( items_c[i].to_op() * *res ); // 
    }
    if ( n.is_one() )
        return *res;
    return Op::new_number( n ) * *res;
}

/// @see operator*
void find_mul_items_and_coeff_rec( Op *a, SplittedVec<MulSeq,4,16,true> &items ) {
    if ( a->type == STRING_mul_NUM ) { // anything_but_a_number * anything_but_a_number
        find_mul_items_and_coeff_rec( a->func_data()->children[0], items );
        find_mul_items_and_coeff_rec( a->func_data()->children[1], items );
    } else if ( a->type == STRING_pow_NUM and a->func_data()->children[1]->type == Op::NUMBER ) { // a ^ 10
        MulSeq *s = items.new_elem();
        s->e.init( a->func_data()->children[1]->number_data()->val );
        s->op = a->func_data()->children[0];
    } else { // a
        MulSeq *s = items.new_elem();
        s->e.init( 1 );
        s->op = a;
    }
}
/// @see operator*
void mul_items_and_coeffs( const SplittedVec<MulSeq,4,16,true> &items_a, SplittedVec<MulSeq,4,16,true> &items_b ) {
    for(unsigned i=0;i<items_a.size();++i) {
        for(unsigned j=0;;++j) {
            if ( j == items_b.size() ) {
                items_b.push_back( items_a[i] );
                break;
            } 
            if ( items_a[ i ].op == items_b[ j ].op ) {
                items_b[ j ].e += items_a[ i ].e;
                if ( not items_b[ j ].e )
                    items_b.erase_elem_nb_in_unordered_list( j );
                break;
            }
        }
    }
}

Op &operator*( Op &a, Op &b ) {
    // 10 * ...
    if ( a.type == Op::NUMBER ) {
        // 10 * 20
        if ( b.type == Op::NUMBER )
            return Op::new_number( a.number_data()->val * b.number_data()->val );
        // 10 * ( ... )
        return mul_number_and_expr( a, b );
    }
    // ( not a number ) * 10 -> 10 * ( not a number )
    if ( b.type == Op::NUMBER )
        return mul_number_and_expr( b, a );
        
    // ( 10 * a ) * ( not a number ) -> 10 * ( a * ( not a number ) )
    if ( a.type == STRING_mul_NUM and a.func_data()->children[0]->type == Op::NUMBER ) {
        // ( 10 * a ) * ( 20 * b ) -> 30 * ( a * b )
        if ( b.type == STRING_mul_NUM and b.func_data()->children[0]->type == Op::NUMBER ) {
            Op &new_num = Op::new_number( a.func_data()->children[0]->number_data()->val * b.func_data()->children[0]->number_data()->val );
            Op &new_mul = *a.func_data()->children[1] * *b.func_data()->children[1];
            return new_num * new_mul;
        }
        // ( 10 * a ) * b -> 10 * ( a * b )
        Op &new_mul = *a.func_data()->children[1] * b;
        return *a.func_data()->children[0] * new_mul;
        //         return mul_number_and_expr( *a.func_data()->children[0], new_mul );
    }
    // a * ( 10 * b ) -> 10 * ( a * b )
    if ( b.type == STRING_mul_NUM and b.func_data()->children[0]->type == Op::NUMBER ) {
        Op &new_mul = a * *b.func_data()->children[1];
        return *b.func_data()->children[0] * new_mul;
    }
    
    // ( a^10 * b * anything_but_a_number ) * ( a * anything_but_a_number )
    SplittedVec<MulSeq,4,16,true> items_a, items_b;
    find_mul_items_and_coeff_rec( &a, items_a );
    find_mul_items_and_coeff_rec( &b, items_b );
    unsigned old_size = items_a.size() + items_b.size();
    mul_items_and_coeffs( items_b, items_a );
    if ( old_size != items_a.size() )
      return make_mul_seq( items_a );
    
    //
    if ( &a < &b )
        return Op::new_function( STRING_mul_NUM, a, b );
    return Op::new_function( STRING_mul_NUM, b, a );
}
Op &inv( Op &a ) {
    return pow( a, Op::new_number(-1) );
}
Op &operator/( Op &a, Op &b ) {
    return a * inv( b );
}

bool need_abs_for_pow( Op *op, const Rationnal &e0, const Rationnal &nn ) {
    if ( op->necessary_positive_or_null() )
        return false;
    if ( not e0.is_even() ) // (a^3)^... or (a^0.5)^... -> non need
        return false;
    if ( nn.is_even() ) // a^(2) -> abs is not useful
        return false;
    // (a^6)^0.5 -> a^3
    return true;
}

Op &pow( Op &a, Op &b ) {
    if ( b.type == Op::NUMBER ) { // a ^ 10
        if ( a.type == Op::NUMBER ) // 10 ^ 32
            return Op::new_number( pow_96( a.number_data()->val, b.number_data()->val ) );
        if ( b.is_zero() )
            return Op::new_number( 1 );
        if ( b.is_one() )
            return a.inc_ref();
        if ( a.type == STRING_abs_NUM ) { // abs(a) ^ n
            if ( b.number_data()->val.is_even() ) // abs(a) ^ 4
                return pow( *a.func_data()->children[0], b );
        }
        // ( a * b^2 ) ^ 3 -> a^3 * b^6
        if ( a.type == STRING_mul_NUM and b.number_data()->val.is_integer() ) {
            SplittedVec<MulSeq,4,16,true> items;
            find_mul_items_and_coeff_rec( a.func_data()->children[0], items );
            find_mul_items_and_coeff_rec( a.func_data()->children[1], items );
            //
            for(unsigned i=0;i<items.size();++i)
                items[i].e *= b.number_data()->val;
            return make_mul_seq( items );
            
        }
        // ( a^2 ) ^ 3 -> a^6
        if ( a.type == STRING_pow_NUM and a.func_data()->children[1]->type == Op::NUMBER ) {
            Rationnal e0 = a.func_data()->children[1]->number_data()->val;
            Rationnal nn = e0 * b.number_data()->val;
            // (a^0.5)^3 -> a^1.5
            // (a^2)^0.5 -> abs(a)
            if ( need_abs_for_pow( a.func_data()->children[0], e0, nn ) ) {
                if ( nn.is_one() )
                    return abs( *a.func_data()->children[0] );
                return pow( abs( *a.func_data()->children[0] ), Op::new_number( nn ) );
                //                 return Op::new_function( STRING_pow_NUM, abs( *a.func_data()->children[0] ), Op::new_number( nn ) );
            } else {
                if ( nn.is_one() )
                    return a.func_data()->children[0]->inc_ref();
                return pow( *a.func_data()->children[0], Op::new_number( nn ) );
                //                 return Op::new_function( STRING_pow_NUM, *a.func_data()->children[0], Op::new_number( nn ) );
            }
        }
    }
    //
    return Op::new_function( STRING_pow_NUM, a, b );
}


std::string graphviz_repr( Op *op ) { std::ostringstream res; graphviz_repr( res, op ); return res.str(); }
std::string cpp_repr     ( Op *op ) { std::ostringstream res; cpp_repr     ( res, op ); return res.str(); }
std::string tex_repr     ( Op *op ) { std::ostringstream res; tex_repr     ( res, op ); return res.str(); }

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
Op &diff_rec( Thread *th, const void *tok, Op &a, SplittedVec<Op *,1024,4096> &of, Op &z, Op &o );

Op &diff_rec( Thread *th, const void *tok, Op &a, SplittedVec<Op *,1024,4096> &of, Op &z, Op &o ) {
    if ( a.additional_info )
        return *a.additional_info; // 
    //
    #define GET_D0 Op &c0 = *a.func_data()->children[0]; Op &da = diff_rec( th, tok, c0, of, z, o ); if (&da==&z) { a.additional_info = &z; return z; }
    #define GET_D01 Op &c0 = *a.func_data()->children[0]; Op &c1 = *a.func_data()->children[1]; Op &da = diff_rec( th, tok, c0, of, z, o ); Op &db = diff_rec( th, tok, c1, of, z, o ); if (&da==&z and &db==&z) { a.additional_info = &z; return z; }
    // 
    switch ( a.type ) {
        case Op::NUMBER:
        case Op::SYMBOL:
            a.additional_info = &z;
            return z;
        //
        case STRING_heaviside_NUM: a.additional_info = &z; return z;
        case STRING_eqz_NUM:       a.additional_info = &z; return z;
        //
        case STRING_log_NUM:       { GET_D0; Op &n = o / c0; Op &r = da * n; of.push_back(&n); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_abs_NUM:       { GET_D0; Op &h = heaviside(c0); Op &_2 = Op::new_number(2); Op &h2 = h * _2; Op &h2m1 = h2 - o; Op &r = da * h2m1;
                                        of.push_back(&h); of.push_back(&_2); of.push_back(&h2); of.push_back(&h2m1); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_exp_NUM:       { GET_D0; Op &r = da * a; of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_sin_NUM:       { GET_D0; Op &cos_c0 = cos(c0); Op &r = da * cos_c0; of.push_back(&cos_c0); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_cos_NUM:       { GET_D0; Op &sin_c0 = sin(c0); Op &msin_c0 = - sin_c0; Op &r = da * msin_c0; of.push_back(&sin_c0); of.push_back(&msin_c0); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_tan_NUM:       { GET_D0; Op &_2 = Op::new_number(2); Op &pow_a_2 = pow(a,_2); Op &_1_p_pow_a_2 = o+pow_a_2; Op &r = da / _1_p_pow_a_2;
                                        of.push_back(&_2); of.push_back(&pow_a_2); of.push_back(&_1_p_pow_a_2); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_asin_NUM:      { GET_D0; Op &_2 = Op::new_number(2); Op &pow_c0_2 = pow(c0,_2); Op &_1_s_pow_c0_2 = o-pow_c0_2; Op &z5 = Op::new_number(Rationnal(-1,2)); Op &pow_1_s_pow_c0_2_z5 = pow( _1_s_pow_c0_2, z5 );
                                        Op &r = da * pow_1_s_pow_c0_2_z5; of.push_back(&_2); of.push_back(&pow_c0_2); of.push_back(&_1_s_pow_c0_2); of.push_back(&z5); of.push_back(&pow_1_s_pow_c0_2_z5); of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_acos_NUM:      { GET_D0; Op &_2 = Op::new_number(2); Op &pow_c0_2 = pow(c0,_2); Op &_1_s_pow_c0_2 = o-pow_c0_2; Op &z5 = Op::new_number(Rationnal(-1,2)); Op &pow_1_s_pow_c0_2_z5 = pow( _1_s_pow_c0_2, z5 );
                                        Op &mr = da * pow_1_s_pow_c0_2_z5; Op &r = -mr; of.push_back(&_2); of.push_back(&pow_c0_2); of.push_back(&_1_s_pow_c0_2); of.push_back(&z5); of.push_back(&pow_1_s_pow_c0_2_z5); of.push_back(&mr); 
                                        of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_atan_NUM:      { GET_D0; Op &_2 = Op::new_number(2); Op &pow_c0_2 = pow(c0,_2); Op &_1_p_pow_c0_2 = o+pow_c0_2; Op &r = da / _1_p_pow_c0_2;
                                        of.push_back(&_2); of.push_back(&pow_c0_2); of.push_back(&_1_p_pow_c0_2); of.push_back(&r); a.additional_info = &r; return r; }
                                        
        case STRING_add_NUM:       { GET_D01; Op &r = da + db; of.push_back(&r); a.additional_info = &r; return r; }
        case STRING_mul_NUM:       { GET_D01; Op &_1 = da * c1; Op &_2 = c0 * db; Op &r = _1 + _2; of.push_back(&r); of.push_back(&_1); of.push_back(&_2); a.additional_info = &r; return r; }
        case STRING_pow_NUM:       {
            if ( a.func_data()->children[1]->type == Op::NUMBER ) {
                GET_D0;
                Op &_1 = Op::new_number( a.func_data()->children[1]->number_data()->val - Rationnal(1) );
                Op &_2 = pow( c0, _1 );
                Op &_3 = *a.func_data()->children[1] * _2;
                Op &r = da * _3;
                of.push_back(&r); of.push_back(&_1); of.push_back(&_2); of.push_back(&_3);
                a.additional_info = &r;
                return r;
            } else {
                GET_D01;
                Op &_0 = Op::new_number( - Rationnal(1) );
                Op &_1 = *a.func_data()->children[1] + _0;
                Op &_2 = pow( c0, _1 );
                Op &_3 = *a.func_data()->children[1] * _2;
                Op &_4 = da * _3;
                //
                Op &_5 = log( c0 );
                Op &_6 = db * _5;
                Op &_7 = _6 * a;
                Op &r  = _4 + _7;
                of.push_back(&r); of.push_back(&_0); of.push_back(&_1); of.push_back(&_2); of.push_back(&_3); of.push_back(&_4); of.push_back(&_5); of.push_back(&_6); of.push_back(&_7);
                a.additional_info = &r;
                return r;
            }
        }
        
        default:
            th->add_error( "for now, no rules to differentiate function '"+std::string(Nstring(a.type))+"'.", tok );
    }
    #undef GET_D0
    return z;
}

Op *diff( Thread *th, const void *tok, Op *a, Op *b ) {
    SplittedVec<Op *,1024,4096> of;
    
    a->clear_additional_info_rec();
    b->additional_info = &Op::new_number( 1 );
    
    Op *z = &Op::new_number( 0 );
    Op *o = &Op::new_number( 1 );
    of.push_back( b->additional_info );
    of.push_back( z );
    of.push_back( o );
    
    //
    Op &res = diff_rec( th, tok, *a, of, *z, *o );
    res.inc_ref();
    for(unsigned i=0;i<of.size();++i)
        dec_ref( of[i] );
    return &res;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
Op &subs_rec( Thread *th, const void *tok, Op &a, SplittedVec<Op *,1024,4096> &of );

Op &subs_rec( Thread *th, const void *tok, Op &a, SplittedVec<Op *,1024,4096> &of ) {
    if ( a.additional_info )
        return *a.additional_info;
    // 
    #define GET_S0 Op &c0 = *a.func_data()->children[0]; Op &s0 = subs_rec( th, tok, c0, of ); if (&s0==&c0) { a.additional_info = &a; return a; }
    #define GET_S01 Op &c0 = *a.func_data()->children[0]; Op &c1 = *a.func_data()->children[1]; Op &s0 = subs_rec( th, tok, c0, of ); Op &s1 = subs_rec( th, tok, c1, of ); if (&s0==&c0 and &s1==&c1) { a.additional_info = &a; return a; }
    switch ( a.type ) {
        case Op::NUMBER:
        case Op::SYMBOL:
            a.additional_info = &a;
            return *a.additional_info;
        //
        case STRING_heaviside_NUM: { GET_S0; Op &r = heaviside( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_eqz_NUM:       { GET_S0; Op &r = eqz      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_log_NUM:       { GET_S0; Op &r = log      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_abs_NUM:       { GET_S0; Op &r = abs      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_exp_NUM:       { GET_S0; Op &r = exp      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_sin_NUM:       { GET_S0; Op &r = sin      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_cos_NUM:       { GET_S0; Op &r = cos      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_tan_NUM:       { GET_S0; Op &r = tan      ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_asin_NUM:      { GET_S0; Op &r = asin     ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_acos_NUM:      { GET_S0; Op &r = acos     ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_atan_NUM:      { GET_S0; Op &r = atan     ( s0 ); a.additional_info = &r; of.push_back(&r); return r; }
        
        case STRING_mul_NUM:       { GET_S01; Op &r =    s0 * s1; a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_add_NUM:       { GET_S01; Op &r =    s0 + s1; a.additional_info = &r; of.push_back(&r); return r; }
        case STRING_pow_NUM:       { GET_S01; Op &r = pow(s0,s1); a.additional_info = &r; of.push_back(&r); return r; }
        
        default:
            th->add_error( "for now, no rules to substitute function '"+std::string(Nstring(a.type))+"'.", tok );
    }
    return *a.additional_info;
}

Op *subs( Thread *th, const void *tok, Op *self, VarArgs &a, VarArgs &b ) {
    if ( a.variables.size() != b.variables.size() ) {
        std::ostringstream ss;
        ss << "attempting to make 'op.subs(a,b)' with a and b of different sizes (respectively " << a.variables.size() << " and " << b.variables.size() << ").";
        th->add_error(ss.str(),tok);
        return &self->inc_ref();
    }
    
    SplittedVec<Op *,1024,4096> of;
    self->clear_additional_info_rec();
    for(unsigned i=0;i<a.variables.size();++i)
        (*reinterpret_cast<Op **>(a.variables[i].data))->additional_info = *reinterpret_cast<Op **>(b.variables[i].data);
    //
    Op &res = subs_rec( th, tok, *self, of ).inc_ref();
    for(unsigned i=0;i<of.size();++i) dec_ref( of[i] );
    return &res;
}

Op *subs( Thread *th, const void *tok, Op *self, Op *a, Op *b ) {
    SplittedVec<Op *,1024,4096> of;
    self->clear_additional_info_rec();
    a->additional_info = b;
    //
    Op &res = subs_rec( th, tok, *self, of ).inc_ref();
    for(unsigned i=0;i<of.size();++i) dec_ref( of[i] );
    return &res;
}

Rationnal subs_numerical( Thread *th, const void *tok, Op *op, const Rationnal &a ) {
    SplittedVec<Op *,32> symbols;
    get_sub_symbols( op, symbols );
    if ( symbols.size() > 1 ) {
        th->add_error("subs_numerical works only with expressions which contains at most 1 variable.",tok);
        return 0;
    }
    //
    SplittedVec<Op *,1024,4096> of;
    op->clear_additional_info_rec();
    if ( symbols.size() ) {
        Op &n = Op::new_number( a );
        of.push_back( &n );
        symbols[0]->additional_info = &n;
    }
    //
    Op &res = subs_rec( th, tok, *op, of ).inc_ref();
    for(unsigned i=0;i<of.size();++i) dec_ref( of[i] );
    assert( res.type == Op::NUMBER );
    Rationnal n_res = res.number_data()->val; dec_ref( &res );
    return n_res;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
void get_sub_symbols( Op *op, SplittedVec<Op *,32> &symbols ) {
    if ( op->type == Op::SYMBOL ) {
        for(unsigned i=0;;++i) {
            if ( i == symbols.size() ) {
                symbols.push_back( op );
                return;
            }
            if ( symbols[i] == op )
                return;
        }
    }
    if ( op->type > 0 ) {
        Op::FuncData *fd = op->func_data();
        if ( fd->children[0] ) get_sub_symbols( fd->children[0], symbols );
        if ( fd->children[1] ) get_sub_symbols( fd->children[1], symbols );
    }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool depends_on_rec( Op *op, Op *var ) {
    if ( op == var )
        return true;
    if ( op->type >= 0 ) {
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i)
            if ( depends_on_rec( op->func_data()->children[i], var ) )
                return true;
    }
    return false;
}

// bool op_depends_on_symbols( Op *op, VarArgs &symbols ) {
//     if ( symbols.variables.size() == 0 )
//         return true;
//     for(unsigned i=0;i<symbols.variables.size();++i)
//         if ( symbols.variables[i].type==global_data.Op and depends_on_rec( op, *reinterpret_cast<Op **>(symbols.variables[i].data) ) )
//             return true;
//     return false;
// }

Op *find_deepest_disc_func_which_depends_on( Op *op ) {
    if ( op->type >= 0 ) {
        // look in children
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            Op *c = find_deepest_disc_func_which_depends_on( op->func_data()->children[i] );
            if ( c ) 
                return c;
        }
        // abs, heaviside, ... ?
        if ( ( op->type == STRING_heaviside_NUM or op->type == STRING_abs_NUM ) and op->additional_info )
            return op;
    }
    return NULL;
}

void get_discontinuities_rec( Thread *th, const void *tok, Op *op, SplittedVec<Op*,32> &res, SplittedVec<Op *,1024,4096> &of, Op *z, Op *o ) {
    Op *h = find_deepest_disc_func_which_depends_on( op );
    //
    if ( h ) {
        Op *old = res.back();
        // NULL
        res.back() = NULL;
        // condition
        Op *c = h->func_data()->children[0];
        res.push_back( c );
        
        // substitutions
        for(unsigned val_cond=0;val_cond<2;++val_cond) {
            old->clear_additional_info_rec();
            //
            if ( h->type == STRING_heaviside_NUM )
                h->additional_info = ( val_cond ? o : z );
            else if ( h->type == STRING_abs_NUM ) {
                if ( val_cond ) {
                    h->additional_info = c;
                } else {
                    h->additional_info = &operator-( *c );
                    of.push_back( h->additional_info );
                }
            }
            else {
                std::cout << "unmanaged " << Nstring( h->type ) << std::endl;
                assert(0);
            }
            //
            Op *new_op = &subs_rec( th, tok, *old, of );
            res.push_back( new_op );
            get_discontinuities_rec( th, tok, new_op, res, of, z, o );
        }
    }
}
    
void make_varargs_from_disc_desc( Variable *return_var, SplittedVec<Op *,32> res, unsigned &n ) {
    Op *op = res[n];
    if ( op == NULL ) { // -> ...
        return_var->init( global_data.VarArgs, 0 );
        VarArgs *va = reinterpret_cast<VarArgs *>( return_var->data ); va->init();
        make_varargs_from_disc_desc( va->variables.new_elem(), res, ++n ); // cond
        make_varargs_from_disc_desc( va->variables.new_elem(), res, ++n ); // subs 0
        make_varargs_from_disc_desc( va->variables.new_elem(), res, ++n ); // subs 1
    }
    else {
        return_var->init( global_data.Op, 0 );
        *reinterpret_cast<Op **>(return_var->data) = &op->inc_ref();
    }
}

bool mark_dependances_on_rec( Op *expr, VarArgs &symbols, unsigned clear_id ) {
    if ( expr->cleared_id == clear_id )
        return expr->additional_info;
    expr->cleared_id = clear_id;
            
    //
    bool dep = false;
    if ( expr->type > 0 )
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i)
            dep |= mark_dependances_on_rec( expr->func_data()->children[i], symbols, clear_id );
    
    //
    if ( dep ) {
        expr->additional_info = expr;
        return true;
    }
    for(unsigned i=0;i<symbols.variables.size();++i) {
        if ( symbols.variables[i].type==global_data.Op and expr == *reinterpret_cast<Op **>(symbols.variables[i].data) ) {
            expr->additional_info = expr;
            return true;
        }
    }
    return false;
}
void mark_dependances_on( Op *expr, VarArgs &symbols ) {
    mark_dependances_on_rec( expr, symbols, ++clear_id_clear_additional_info_rec );
}

void discontinuities_separation( Thread *th, const void *tok, Variable *return_var, Op *expr, VarArgs &symbols ) {
    if ( not return_var ) return;
    
    // res -> [ NULL cond1, a, NULL, cond2, b, c ]
    SplittedVec<Op *,1024,4096> of;
    Op *z = &Op::new_number( 0 ); of.push_back( z );
    Op *o = &Op::new_number( 1 ); of.push_back( o );
    
    //
    mark_dependances_on( expr, symbols );
    
    //
    SplittedVec<Op *,32> res;
    res.push_back( expr );
    get_discontinuities_rec( th, tok, expr, res, of, z, o );
        
    // return_var -> [ [...] ]
    unsigned n = 0;
    make_varargs_from_disc_desc( return_var, res, n );
    
    // free
    for(unsigned i=0;i<of.size();++i) dec_ref( of[i] );
}




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::string tex_repr( const Rationnal &val, int type_parent ) {
    std::ostringstream os;
    int local_type = 10000;
    if ( val.num.is_negative() and val.den == Rationnal::BI(1) )
        local_type = STRING_sub_NUM;
    else if ( val.den != Rationnal::BI(1) )
        local_type = STRING_div_NUM;
    bool want_par = type_parent > local_type;
    
    if ( want_par ) os << "(";
    if ( val.den == Rationnal::BI(1) ) os << val.num;
    else os << "\\frac{" << val.num << "}{" << val.den << "}";;
    if ( want_par ) os << ")";
    return os.str();
}


void tex_repr_rec( std::ostream &os, Op *op, int type_parent ) {
    if ( op->type == Op::NUMBER ) {
        os << tex_repr( op->number_data()->val, type_parent );
    } else if ( op->type == Op::SYMBOL ) {
        os << op->symbol_data()->tex_name_str;
    } else if ( op->type == STRING_add_NUM ) { // (...) + (...)
        bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
        
        std::vector<Op *> p_part;
        std::vector<Op *> s_part;
        find_p_and_s_parts_rec( op, p_part, s_part );
    
        for(unsigned i=0;i<p_part.size();++i) {
            if ( i ) os << "+";
            tex_repr_rec( os, p_part[i], STRING_add_NUM );
        }
        for(unsigned i=0;i<s_part.size();++i) {
            os << "-";
            if ( s_part[i]->type == Op::NUMBER ) // + (-2)
                os << tex_repr( - s_part[i]->number_data()->val, STRING_add_NUM );
            else { // ( -2 ) * a
                Op *ch_0 = s_part[i]->func_data()->children[0];
                Op *ch_1 = s_part[i]->func_data()->children[1];
                if ( ch_0->number_data()->val.is_minus_one() )
                    tex_repr_rec( os, ch_1, STRING_sub_NUM );
                else {
                    std::cout << ch_0->number_data()->val << std::endl;
                    os << tex_repr( - ch_0->number_data()->val, STRING_mul_NUM );
                    //os << "*";
                    tex_repr_rec( os, ch_1, STRING_mul_NUM );
                }
            }
        }
        
        if ( np ) os << ")";
        /*
        if ( is_a_sub( *op->func_data()->children[1] ) ) { // a + ( -b )
            bool np = type_parent > STRING_sub_NUM; if ( np ) os << "(";
            tex_repr_rec( os, op->func_data()->children[0], STRING_add_NUM );
            os << "-";
            tex_repr_rec( os, op->func_data()->children[1]->func_data()->children[1], STRING_sub_NUM );
            if ( np ) os << ")";
        } else if ( is_a_sub( *op->func_data()->children[0] ) ) { // (-a) + b
            bool np = type_parent > STRING_sub_NUM; if ( np ) os << "(";
            tex_repr_rec( os, op->func_data()->children[1], STRING_add_NUM );
            os << "-";
            tex_repr_rec( os, op->func_data()->children[0]->func_data()->children[1], STRING_sub_NUM );
            if ( np ) os << ")";
        } else if ( op->func_data()->children[0]->type == Op::NUMBER and op->func_data()->children[0]->number_data()->val.num.is_negative() ) { // (-2) + b
            bool np = type_parent > STRING_sub_NUM; if ( np ) os << "(";
            tex_repr_rec( os, op->func_data()->children[1], STRING_add_NUM ); 
            os << "-" << tex_repr( - op->func_data()->children[0]->number_data()->val, STRING_sub_NUM );
            if ( np ) os << ")";
        } else { // a + b
            bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
            tex_repr_rec( os, op->func_data()->children[0], STRING_add_NUM );
            os << "+";
            tex_repr_rec( os, op->func_data()->children[1], STRING_add_NUM );
            if ( np ) os << ")";
        }
        */
    } else if ( op->type == STRING_mul_NUM ) { // (...) * (...)
        SplittedVec<MulSeq,4,16,true> items;
        find_mul_items_and_coeff_rec( op, items );
        SplittedVec<MulSeq,4,16,true> items_[2];
        for(unsigned i=0;i<items.size();++i)
            items_[ items[i].e.num.is_negative() ].push_back( items[i] );
        //
        if ( items_[1].size() ) {
            for(unsigned i=0;i<items_[1].size();++i)
                items_[1][i].e.num.val *= -1;
            //
            os << "\\frac{ "; // \\displaystyle
            for(unsigned i=0;i<items_[0].size();++i)
                items_[0][i].tex_repr( os << ( i ? "\\," : " " ) );
            if ( not items_[0].size() )
                os << "1";
            os << "}{ "; // \\displaystyle
            for(unsigned i=0;i<items_[1].size();++i)
                items_[1][i].tex_repr( os << ( i ? "\\," : " " ) );
            os << "}";
        } else { // only *
            //             for(unsigned i=0;i<items.size();++i)
            //                 std::cout << *items[i].op << std::endl;
            for(unsigned i=0;i<items.size();++i)
                items[i].tex_repr( os << ( i ? "\\," : " " ) );
        }
    } else if ( op->type == STRING_pow_NUM ) { // (...) ^ (...)
        bool np = type_parent > STRING_pow_NUM; if ( np ) os << "(";
        if ( op->func_data()->children[1]->type == Op::NUMBER and op->func_data()->children[1]->number_data()->val.num.is_one() and op->func_data()->children[1]->number_data()->val.den.is_one()==false ) {
            if ( op->func_data()->children[1]->number_data()->val.den.is_two() )
                os << "\\sqrt{";
            else
                os << "\\sqrt[" << op->func_data()->children[1]->number_data()->val.den << "]{";
            tex_repr_rec( os, op->func_data()->children[0], 0 );
            os << "}";
        } else {
            os << "{";
            tex_repr_rec( os, op->func_data()->children[0], STRING_pow_NUM );
            os << "}^{";
            tex_repr_rec( os, op->func_data()->children[1], STRING_pow_NUM );
            os << "}";
        }
        if ( np ) os << ")";
    } else if ( is_a_sub( *op ) ) { // -a
        assert( op->func_data()->children[1] == NULL );
        bool np = type_parent > op->type;
        if ( np ) os << "(";
        os << "-";
        tex_repr_rec( os, op->func_data()->children[1], op->type );
        if ( np ) os << ")";
    } else if ( op->type == STRING_mod_NUM ) { // (...) % (...)
        bool np = type_parent > op->type; if ( np ) os << "(";
        os << "{";
        tex_repr_rec( os, op->func_data()->children[0], op->type );
        os << "}\\bmod{";
        tex_repr_rec( os, op->func_data()->children[1], op->type );
        os << "}";
        if ( np ) os << ")";
    } else if ( op->type == STRING_abs_NUM ) { // abs( a )
        os << "\\left|";
        tex_repr_rec( os, op->func_data()->children[0], op->type );
        os << "\\right|";
    } else {
        if ( op->type==STRING_heaviside_NUM )
            os << "\\mathop{\\mathrm{H}}(";
        else
            os << "\\mathop{\\mathrm{" << Nstring(op->type) << "}}(";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            if ( i ) os << ",";
            tex_repr_rec( os, op->func_data()->children[i], 0 );
        }
        os << ")";
    }
}



void tex_repr( std::ostream &os, Op *op ) {
    tex_repr_rec( os, op, 0 );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
Op &mod( Op &a, Op &b ) {
    if ( are_numbers(a,b) ) return Op::new_number( mod( a.number_data()->val, b.number_data()->val ) );
    if ( a.is_zero() ) return a.inc_ref();
    return Op::new_function( STRING_mod_NUM, a, b );
}

Op &atan2( Op &a, Op &b ) {
    if ( are_numbers(a,b) ) return Op::new_number( Rationnal( atan2( Float96(a.number_data()->val), Float96(b.number_data()->val) ) ) );
    return Op::new_function( STRING_atan2_NUM, a );
}

Op &abs( Op &a ) {
    if ( a.type == Op::NUMBER ) return Op::new_number( abs( a.number_data()->val ) );
    if ( a.necessary_positive_or_null() ) return a.inc_ref();
    return Op::new_function( STRING_abs_NUM, a );
}

Op &log      ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( log_96   ( a.number_data()->val ) ); return Op::new_function( STRING_log_NUM      , a ); }
Op &heaviside( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( heaviside( a.number_data()->val ) ); if ( a.necessary_positive_or_null() ) return Op::new_number(1); return Op::new_function( STRING_heaviside_NUM, a ); }
Op &eqz      ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( eqz      ( a.number_data()->val ) ); return Op::new_function( STRING_eqz_NUM      , a ); }
Op &exp      ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( exp_96   ( a.number_data()->val ) ); return Op::new_function( STRING_exp_NUM      , a ); }
Op &sin      ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( sin_96   ( a.number_data()->val ) ); return Op::new_function( STRING_sin_NUM      , a ); }
Op &cos      ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( cos_96   ( a.number_data()->val ) ); return Op::new_function( STRING_cos_NUM      , a ); }
Op &tan      ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( tan_96   ( a.number_data()->val ) ); return Op::new_function( STRING_tan_NUM      , a ); }
Op &asin     ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( asin_96  ( a.number_data()->val ) ); return Op::new_function( STRING_asin_NUM     , a ); }
Op &acos     ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( acos_96  ( a.number_data()->val ) ); return Op::new_function( STRING_acos_NUM     , a ); }
Op &atan     ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( atan_96  ( a.number_data()->val ) ); return Op::new_function( STRING_atan_NUM     , a ); }
Op &sinh     ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( sinh_96  ( a.number_data()->val ) ); return Op::new_function( STRING_sinh_NUM     , a ); }
Op &cosh     ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( cosh_96  ( a.number_data()->val ) ); return Op::new_function( STRING_cosh_NUM     , a ); }
Op &tanh     ( Op &a ) { if ( a.type == Op::NUMBER ) return Op::new_number( tanh_96  ( a.number_data()->val ) ); return Op::new_function( STRING_tanh_NUM     , a ); }


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------

Op *__integration__( Thread *th, const void *tok, Op *expr, Op *var, Op *beg, Op *end, Int32 deg_poly_max ) {
    // look for discontinuities
    expr->inc_ref();
    
    // taylor expansion
    std::vector<Op *> taylor_expansion;
    Op *r = expr->inc_ref(); // -> hum... mem leak
    for(unsigned i=0;i<=deg_poly_max;++i) {
        taylor_expansion.push_back( subs( r, var, beg ) );
        if ( i < deg_poly_max )
            r = diff( th, tok, r, var );
    }
    return integration_taylor_expansion( taylor_expansion );
}



