#include "op.h"
#include "basic_string_manip.h"
#include "nstring.h"
    
unsigned Op::current_op = 0;

void Op::init( int type_ ) {
    parents.init();
    chroot.init();
    type = type_;
    cpt_use = 0;
    op_id = 0;
    beg_value_valid = false;
    end_value_valid = false;
    simplified = false;
    integer_type = false;
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
    for(unsigned i=0;i<chroot.size();++i)
        dec_ref( chroot[ i ] );
    chroot.destroy();
    //
    if ( beg_value_valid )
        beg_value.destroy();
    if ( end_value_valid )
        end_value.destroy();
}

Op *Op::new_number( const Rationnal &val ) {
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(NumberData) ); res->init( NUMBER );
    
    init_arithmetic( res->number_data()->val, val );
    
    res->integer_type = val.is_integer();
    
    res->set_beg_value( val, true );
    res->set_end_value( val, true );
    return res;
}

Op *Op::new_symbol( const char *cpp_name_str, unsigned cpp_name_si, const char *tex_name_str, unsigned tex_name_si ) {
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(SymbolData) ); res->init( SYMBOL );
    
    SymbolData *ds = res->symbol_data();
    ds->cpp_name_str = strdupp0( cpp_name_str, cpp_name_si );
    ds->tex_name_str = strdupp0( tex_name_str, tex_name_si );
    ds->access_cost = 1.0;
    ds->nb_simd_terms = 1;
    return res;
}

Op *Op::new_function( int type, Op *a ) {
    // look in parents of a if function already created somewhere
    for(unsigned i=0;i<a->parents.size();++i) {
        Op *p = a->parents[ i ];
        if ( p->type == type and same_op( a, p->func_data()->children[0] ) and not p->func_data()->children[1] )
            return p->inc_ref();
    }
    //
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(FuncData) ); res->init( type );
    
    FuncData *ds = res->func_data();
    ds->children[0] = a->inc_ref();
    ds->children[1] = NULL;
    ds->children[2] = NULL;
    ds->children[3] = NULL;
    ds->children[4] = NULL;
    ds->children[5] = NULL;
    
    if ( a->type != Op::NUMBER ) // should be ...
        a->parents.push_back( res );
    
    return res;
}

Op *Op::new_function( int type, Op *a, Op *b ) {
    // look in parents of a or b if function already created somewhere
    Op *c = ( a->type == Op::NUMBER ? b : a );
    for(unsigned i=0;i<c->parents.size();++i) {
        Op *p = c->parents[ i ];
        if ( p->type == type and same_op( a, p->func_data()->children[0] ) and same_op( b, p->func_data()->children[1] ) )
            return p->inc_ref();
    }
    //
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(FuncData) ); res->init( type );
    
    FuncData *ds = res->func_data();
    ds->children[0] = a->inc_ref();
    ds->children[1] = b->inc_ref();
    ds->children[2] = NULL;
    ds->children[3] = NULL;
    ds->children[4] = NULL;
    ds->children[5] = NULL;
    
    if ( a->type != Op::NUMBER )
        a->parents.push_back( res );
    if ( b->type != Op::NUMBER )
        b->parents.push_back( res );
        
    if ( type == STRING_add_NUM )
        res->integer_type = a->integer_type and b->integer_type;
    if ( type == STRING_mul_NUM )
        res->integer_type = a->integer_type and b->integer_type;
    
    return res;
}

Op *Op::new_function( int type, Op *a, Op *b, Op *d, Op *e, Op *f, Op *g ) {
    // look in parents of a or b if function already created somewhere
    Op *c = ( a->type == Op::NUMBER ? b : a );
    for(unsigned i=0;i<c->parents.size();++i) {
        Op *p = c->parents[ i ];
        if ( p->type == type and same_op( a, p->func_data()->children[0] ) and same_op( b, p->func_data()->children[1] ) and same_op( d, p->func_data()->children[2] ) and same_op( e, p->func_data()->children[3]) and same_op( f, p->func_data()->children[4]) and same_op( g, p->func_data()->children[5]) )
            return p->inc_ref();
    }
    //
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(FuncData) ); res->init( type );
    
    FuncData *ds = res->func_data();
    ds->children[0] = a->inc_ref();
    ds->children[1] = b->inc_ref();
    ds->children[2] = d->inc_ref();
    ds->children[3] = e->inc_ref();
    ds->children[4] = f->inc_ref();
    ds->children[5] = g->inc_ref();
    
    if ( a->type != Op::NUMBER )
        a->parents.push_back( res );
    if ( b->type != Op::NUMBER )
        b->parents.push_back( res );
    if ( d->type != Op::NUMBER )
        d->parents.push_back( res );
    if ( e->type != Op::NUMBER )
        e->parents.push_back( res );
    if ( f->type != Op::NUMBER )
        f->parents.push_back( res );
    if ( g->type != Op::NUMBER )
        g->parents.push_back( res );    
        
    if ( type == STRING_add_NUM )
        res->integer_type = a->integer_type and b->integer_type;
    if ( type == STRING_mul_NUM )
        res->integer_type = a->integer_type and b->integer_type;
    
    return res;
}

Op *Op::new_function( int type, Op *a, Op *b, Op *d, Op *e, Op *f ) {
    // look in parents of a or b if function already created somewhere
    Op *c = ( a->type == Op::NUMBER ? b : a );
    for(unsigned i=0;i<c->parents.size();++i) {
        Op *p = c->parents[ i ];
        if ( p->type == type and same_op( a, p->func_data()->children[0] ) and same_op( b, p->func_data()->children[1] ) and same_op( d, p->func_data()->children[2] ) and same_op( e, p->func_data()->children[3]) and same_op( f, p->func_data()->children[4]))
            return p->inc_ref();
    }
    //
    Op *res = (Op *)malloc( sizeof(Op) + sizeof(FuncData) ); res->init( type );
    
    FuncData *ds = res->func_data();
    ds->children[0] = a->inc_ref();
    ds->children[1] = b->inc_ref();
    ds->children[2] = d->inc_ref();
    ds->children[3] = e->inc_ref();
    ds->children[4] = f->inc_ref();
    ds->children[5] = NULL;
    
    if ( a->type != Op::NUMBER )
        a->parents.push_back( res );
    if ( b->type != Op::NUMBER )
        b->parents.push_back( res );
    if ( d->type != Op::NUMBER )
        d->parents.push_back( res );
    if ( e->type != Op::NUMBER )
        e->parents.push_back( res );
    if ( f->type != Op::NUMBER )
        f->parents.push_back( res );
        
    if ( type == STRING_add_NUM )
        res->integer_type = a->integer_type and b->integer_type;
    if ( type == STRING_mul_NUM )
        res->integer_type = a->integer_type and b->integer_type;
    
    return res;
}

bool Op::is_zero     () const { return type==NUMBER and number_data()->val.is_zero(); }
bool Op::is_one      () const { return type==NUMBER and number_data()->val.is_one(); }
bool Op::is_minus_one() const { return type==NUMBER and number_data()->val.is_minus_one(); }


bool Op::necessary_positive() const {
    if ( beg_value_valid ) {
        if ( beg_value_inclusive ) {
            if ( beg_value.is_pos() )
                return true;
        }
        else {
            if ( beg_value.is_pos_or_null() )
                return true;
        }
    }
    return false;
}


bool Op::necessary_positive_or_null() const {
    return beg_value_valid and beg_value.is_pos_or_null();
}

bool Op::necessary_negative() const {
    if ( end_value_valid ) {
        if ( end_value_inclusive ) {
            if ( end_value.is_neg() )
                return true;
        }
        else {
            if ( end_value.is_neg_or_null() )
                return true;
        }
    }
    return false;
}


bool Op::necessary_negative_or_null() const {
    return end_value_valid and end_value.is_neg_or_null();
}

/// @see operator*
void find_mul_items_and_coeff_rec( const Op *a, SplittedVec<MulSeq,4,16,true> &items ) {
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

int Op::set_beg_value( Rationnal b, bool inclusive ) {
    beg_value_inclusive = inclusive;
    if ( beg_value_valid )
        beg_value = b;
    else {
        beg_value.init( b );
        beg_value_valid = true;
    }
    return parents.size();
}

int Op::set_end_value( Rationnal e, bool inclusive ) {
    end_value_inclusive = inclusive;
    if ( end_value_valid )
        end_value = e;
    else {
        end_value.init( e );
        end_value_valid = true;
    }
    return parents.size();
}

// ----------------------------------------------------------------------------------------------------------
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
void find_p_and_s_parts_rec( const Op *op, std::vector<const Op *> &p_part, std::vector<const Op *> &s_part ) {
    if ( op->type == Op::NUMBER and op->number_data()->val.is_neg() )
        s_part.push_back( op );
    else if ( op->type != STRING_add_NUM )
        p_part.push_back( op );
    else {
        if ( is_a_mul_by_neg_number( op->func_data()->children[0] ) ) // ( -2 ) * a
            s_part.push_back( op->func_data()->children[0] );
        else
            find_p_and_s_parts_rec( op->func_data()->children[0], p_part, s_part );
        //
        if ( is_a_mul_by_neg_number( op->func_data()->children[1] ) ) // ( -2 ) * a
            s_part.push_back( op->func_data()->children[1] );
        else
            find_p_and_s_parts_rec( op->func_data()->children[1], p_part, s_part );
    }
}

void cpp_repr_rec( std::ostream &os, const Op *op, int type_parent ) {
    if ( op->type == Op::NUMBER ) {
        os << cpp_repr( op->number_data()->val, type_parent );
    } else if ( op->type == Op::SYMBOL ) {
        os << op->symbol_data()->cpp_name_str;
    } else if ( op->type == STRING_add_NUM ) { // (...) + (...)
        bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
        
        std::vector<const Op *> p_part;
        std::vector<const Op *> s_part;
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
    } else if ( is_a_sub( op ) ) { // - a ( encoded as (-1)*a )
        bool np = type_parent > op->type;
        if ( np ) os << "(";
        os << "-";
        cpp_repr_rec( os, op->func_data()->children[1], op->type );
        if ( np ) os << ")";
    } else if ( op->type == STRING_mul_NUM ) { // (...) * (...)
        if ( is_a_inv( op->func_data()->children[1] ) ) { // a * ( 1/b )
            bool np = type_parent > STRING_div_NUM; if ( np ) os << "(";
            cpp_repr_rec( os, op->func_data()->children[0], STRING_mul_NUM );
            os << "/";
            cpp_repr_rec( os, op->func_data()->children[1]->func_data()->children[0], STRING_div_NUM );
            if ( np ) os << ")";
        } else if ( is_a_inv( op->func_data()->children[0] ) ) { // (1/a) * b
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
    } else {
        os << Nstring(op->type) << "(";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            if ( i ) os << ",";
            cpp_repr_rec( os, op->func_data()->children[i], 0 );
        }
        os << ")";
    }
}

void Op::cpp_repr( std::ostream &os ) const {
    cpp_repr_rec( os, this, 0 );
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

void tex_repr( const MulSeq &ms, std::ostream &os, int type_parent ) {
    if ( ms.e.is_one() ) {
        if ( ms.op->type == Op::NUMBER and ms.op->number_data()->val.is_minus_one() ) {
            bool np = ( type_parent > STRING_sub_NUM ) and ms.op->type >= 0;
            if ( np ) os << "(";
            os << "-";
            if ( np ) os << ")";
        }
        else {
            bool np = ( type_parent > ms.op->type ) and ms.op->type >= 0;
            if ( np ) os << "(";
            ms.op->tex_repr( os );
            if ( np ) os << ")";
        }
    }
    else if ( ms.e.num.is_one() ) {
        if ( ms.e.den.is_two() )
            os << "\\sqrt{";
        else    
            os << "\\sqrt[" << ms.e.den << "]{";
        ms.op->tex_repr( os ); 
        os << "}";
    } else {
        bool p = ms.op->type == STRING_add_NUM or ms.op->type == STRING_mul_NUM;
        os << "{";
        if ( p ) os << "(";
        ms.op->tex_repr( os );
        if ( p ) os << ")";
        os << "}";
        if ( ms.e.is_integer() )
            os << "^{" << ms.e.num << "} ";
        else
            os << "^{ \\frac{" << ms.e.num << "}{" << ms.e.den << "} } ";
    }
    os << " ";
}

void tex_repr_rec( std::ostream &os, const Op *op, int type_parent ) {
    if ( op->type == Op::NUMBER ) {
        os << tex_repr( op->number_data()->val, type_parent );
    } else if ( op->type == Op::SYMBOL ) {
        os << op->symbol_data()->tex_name_str;
    } else if ( op->type == STRING_add_NUM ) { // (...) + (...)
        bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
        
        std::vector<const Op *> p_part;
        std::vector<const Op *> s_part;
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
                    os << tex_repr( - ch_0->number_data()->val, STRING_mul_NUM );
                    //os << "*";
                    tex_repr_rec( os, ch_1, STRING_mul_NUM );
                }
            }
        }
        
        if ( np ) os << ")";
    } else if ( op->type == STRING_mul_NUM ) { // (...) * (...)
        SplittedVec<MulSeq,4,16,true> items;
        find_mul_items_and_coeff_rec( op, items );
        SplittedVec<MulSeq,4,16,true> items_[2];
        Rationnal n_num = 1;
        Rationnal n_den = 1;
        for(unsigned i=0;i<items.size();++i) {
            if ( items[i].op->type == Op::NUMBER ) {
                n_num *= items[i].op->number_data()->val.num;
                n_den *= items[i].op->number_data()->val.den;
            }
            else
                items_[ items[i].e.num.is_negative() ].push_back( items[i] );
        }
        //
        if ( items_[1].size() ) { // den
            for(unsigned i=0;i<items_[1].size();++i)
                items_[1][i].e.num.val *= -1;
            //
            os << " \\frac{ "; // \\displaystyle
            if ( not n_num.is_one() )
                os << n_num << " ";
            for(unsigned i=0;i<items_[0].size();++i)
                tex_repr( items_[0][i], os << ( i ? "\\," : " " ), ( items_[0].size() == 1 and n_num.is_one() ? 0 : STRING_mul_NUM ) );
            os << "}{ "; // \\displaystyle
            if ( not n_den.is_one() )
                os << n_den << " ";
            for(unsigned i=0;i<items_[1].size();++i)
                tex_repr( items_[1][i], os << ( i ? "\\," : " " ), ( items_[1].size() == 1 and n_den.is_one() ? 0 : STRING_mul_NUM ) );
            os << "}";
        } else { // only *
            if ( n_num.is_one()==false or n_den.is_one()==false )
                os << tex_repr( n_num / n_den, STRING_mul_NUM ) << " ";
            for(unsigned i=0;i<items_[0].size();++i)
                tex_repr( items_[0][i], os << ( i ? "\\," : " " ), STRING_mul_NUM ) ;
        }
    } else if ( is_a_sub( op ) ) { // -a
        bool np = type_parent > op->type;
        if ( np ) os << "(";
        os << "-";
        tex_repr_rec( os, op->func_data()->children[1], op->type );
        if ( np ) os << ")";
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
        else if ( op->type==STRING_pos_part_NUM )
            os << "\\mathop{\\mathrm{P}_{\\geq 0}}(";
        else
            os << "\\mathop{\\mathrm{" << Nstring(op->type) << "}}(";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            if ( i ) os << ",";
            tex_repr_rec( os, op->func_data()->children[i], 0 );
        }
        os << ")";
    }
}

void Op::tex_repr( std::ostream &os ) const {
    tex_repr_rec( os, this, 0 );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::string maple_repr( const Rationnal &val, int type_parent ) {
    std::ostringstream os;
    int local_type = 10000;
    if ( val.num.is_negative() and val.den == Rationnal::BI(1) )
        local_type = STRING_sub_NUM;
    else if ( val.den != Rationnal::BI(1) )
        local_type = STRING_div_NUM;
    bool want_par = type_parent > local_type;
    
    if ( want_par ) os << "(";
    if ( val.den == Rationnal::BI(1) ) os << val.num;
    else os << val.num << "/" << val.den ;
    if ( want_par ) os << ") ";
    return os.str();
}

void maple_repr( const MulSeq &ms, std::ostream &os, int type_parent ) {
    if ( ms.e.is_one() ) {
        if ( ms.op->type == Op::NUMBER and ms.op->number_data()->val.is_minus_one() ) {
            bool np = ( type_parent > STRING_sub_NUM ) and ms.op->type >= 0;
            if ( np ) os << "(";
            os << "-";
            if ( np ) os << ") ";
        }
        else {
            bool np = ( type_parent > ms.op->type ) and ms.op->type >= 0;
            if ( np ) os << "(";
            ms.op->maple_repr( os );
            if ( np ) os << ") ";
        }
    }
    else if ( ms.e.num.is_one() ) {
        if ( ms.e.den.is_two() )
            os << "sqrt(";
        else
            os << "(";
            //os << "\\sqrt[" << ms.e.den << "]{";
        ms.op->maple_repr( os );
        if ( ms.e.den.is_two() )
            os << ")";
        else
            os << ")**(1/" << ms.e.den << ") ";
    } else {
        bool p = ms.op->type == STRING_add_NUM or ms.op->type == STRING_mul_NUM;
        //os << "{";
        //if ( p ) os << "(";
        os << "(";
        ms.op->maple_repr( os );
        os << ")";
        //if ( p ) os << ")";
        //os << "}";
        if ( ms.e.is_integer() )
            os << "**(" << ms.e.num << ") ";
        else
            os << "**( " << ms.e.num << "/" << ms.e.den << ") ";
    }
}

void maple_repr_rec( std::ostream &os, const Op *op, int type_parent ) {
    if ( op->type == Op::NUMBER ) {
        os << maple_repr( op->number_data()->val, type_parent );
    } else if ( op->type == Op::SYMBOL ) {
        os << op->symbol_data()->cpp_name_str; /// PROVISOIRE
    } else if ( op->type == STRING_add_NUM ) { // (...) + (...)
        bool np = type_parent > STRING_add_NUM; if ( np ) os << "(";
        
        std::vector<const Op *> p_part;
        std::vector<const Op *> s_part;
        find_p_and_s_parts_rec( op, p_part, s_part );
    
        for(unsigned i=0;i<p_part.size();++i) {
            if ( i ) os << "+";
            maple_repr_rec( os, p_part[i], STRING_add_NUM );
        }
        for(unsigned i=0;i<s_part.size();++i) {
            os << "-";
            if ( s_part[i]->type == Op::NUMBER ) // + (-2)
                os << maple_repr( - s_part[i]->number_data()->val, STRING_add_NUM );
            else { // ( -2 ) * a
                Op *ch_0 = s_part[i]->func_data()->children[0];
                Op *ch_1 = s_part[i]->func_data()->children[1];
                if ( ch_0->number_data()->val.is_minus_one() )
                    maple_repr_rec( os, ch_1, STRING_sub_NUM );
                else {
                    os << maple_repr( - ch_0->number_data()->val, STRING_mul_NUM );
                    //os << "*";
                    maple_repr_rec( os, ch_1, STRING_mul_NUM );
                }
            }
        }
        
        if ( np ) os << ")";
    } else if ( op->type == STRING_mul_NUM ) { // (...) * (...)
        SplittedVec<MulSeq,4,16,true> items;
        find_mul_items_and_coeff_rec( op, items );
        SplittedVec<MulSeq,4,16,true> items_[2];
        Rationnal n_num = 1;
        Rationnal n_den = 1;
        for(unsigned i=0;i<items.size();++i) {
            if ( items[i].op->type == Op::NUMBER ) {
                n_num *= items[i].op->number_data()->val.num;
                n_den *= items[i].op->number_data()->val.den;
            }
            else
                items_[ items[i].e.num.is_negative() ].push_back( items[i] );
        }
        //
        if ( items_[1].size() ) { // den
            for(unsigned i=0;i<items_[1].size();++i)
                items_[1][i].e.num.val *= -1;
            //
            os << " ( "; // \\displaystyle
            if ( not n_num.is_one() )
                os << n_num << " ";
            for(unsigned i=0;i<items_[0].size();++i)
                maple_repr( items_[0][i], os << ( i ? "\\," : " " ), ( items_[0].size() == 1 and n_num.is_one() ? 0 : STRING_mul_NUM ) ); /// ?????????????????
            os << " )/( "; // \\displaystyle
            if ( not n_den.is_one() )
                os << n_den << " ";
            for(unsigned i=0;i<items_[1].size();++i)
                maple_repr( items_[1][i], os << ( i ? "\\," : " " ), ( items_[1].size() == 1 and n_den.is_one() ? 0 : STRING_mul_NUM ) );  /// ?????????????????
            os << ")";
        } else { // only *
            if ( n_num.is_one()==false or n_den.is_one()==false )
                os << maple_repr( n_num / n_den, STRING_mul_NUM ) << " ";
            for(unsigned i=0;i<items_[0].size();++i)
                maple_repr( items_[0][i], os << ( i ? "\\," : " " ), STRING_mul_NUM ) ;  /// ?????????????????
        }
    } else if ( is_a_sub( op ) ) { // -a
        bool np = type_parent > op->type;
        if ( np ) os << "(";
        os << "-";
        maple_repr_rec( os, op->func_data()->children[1], op->type );
        if ( np ) os << ")";
    } else if ( op->type == STRING_pow_NUM ) { // (...) ^ (...)
        bool np = type_parent > STRING_pow_NUM;
        if ( np ) os << "(";
        if ( op->func_data()->children[1]->type == Op::NUMBER and op->func_data()->children[1]->number_data()->val.num.is_one() and op->func_data()->children[1]->number_data()->val.den.is_one()==false ) {
            if ( op->func_data()->children[1]->number_data()->val.den.is_two() )
                os << "sqrt(";
            else
                os << "(";
                //os << "\\sqrt[" << op->func_data()->children[1]->number_data()->val.den << "]{";
            maple_repr_rec( os, op->func_data()->children[0], 0 );
            if ( op->func_data()->children[1]->number_data()->val.den.is_two() )
                os << ")";
            else
                os << ")**(1/" << op->func_data()->children[1]->number_data()->val.den << ")";
        } else {
            os << "(";
            maple_repr_rec( os, op->func_data()->children[0], STRING_pow_NUM );
            os << ")^(";
            maple_repr_rec( os, op->func_data()->children[1], STRING_pow_NUM );
            os << ")";
        }
        if ( np ) os << ")";
    } else if ( op->type == STRING_mod_NUM ) { // (...) % (...)
        bool np = type_parent > op->type; if ( np ) os << "(";
        os << "(";
        maple_repr_rec( os, op->func_data()->children[0], op->type );
        os << ") mod(";
        maple_repr_rec( os, op->func_data()->children[1], op->type );
        os << ")";
        if ( np ) os << ")";
    } else if ( op->type == STRING_abs_NUM ) { // abs( a )
        os << "abs(";
        maple_repr_rec( os, op->func_data()->children[0], op->type );
        os << ")";
    } else {
        if ( op->type==STRING_heaviside_NUM )
            os << "Heaviside(";
        else if ( op->type==STRING_pos_part_NUM )
            os << "\\mathop{\\mathrm{P}_{\\geq 0}}("; /// ?????????????????
        else
            os << "\\mathop{\\mathrm{" << Nstring(op->type) << "}}(";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            if ( i ) os << ",";
            maple_repr_rec( os, op->func_data()->children[i], 0 );
        }
        os << ")";
    }
}

void Op::maple_repr( std::ostream &os ) const {
    maple_repr_rec( os, this, 0 );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Op::poly_deg_rec() const {
    if ( op_id == current_op )
        return additional_int;
    op_id = current_op;
    //
    if ( type == STRING_mul_NUM ) {
        int d0 = func_data()->children[0]->poly_deg_rec();
        int d1 = func_data()->children[1]->poly_deg_rec();
        additional_int = ( d0 < 0 or d1 < 0 ? -1 : d0 + d1 );
    } else if ( type == STRING_add_NUM ) {
        int d0 = func_data()->children[0]->poly_deg_rec();
        int d1 = func_data()->children[1]->poly_deg_rec();
        additional_int = ( d0 < 0 or d1 < 0 ? -1 : std::max( d0, d1 ) );
    } else if ( type == STRING_pow_NUM and func_data()->children[1]->type == Op::NUMBER and func_data()->children[1]->number_data()->val.is_integer() and func_data()->children[1]->number_data()->val.is_pos() ) {
        int d0 = func_data()->children[0]->poly_deg_rec();
        additional_int = ( d0 < 0 ? -1 : d0 * int( func_data()->children[1]->number_data()->val.num ) );
    } else if ( type == Op::NUMBER or type == Op::SYMBOL ) {
        additional_int = 0;
    } else if ( type >= 0 ) {
        additional_int = 0;
        // if there is a children that depends on var
        for(unsigned i=0;i<FuncData::max_nb_children and func_data()->children[i];++i) {
            if ( func_data()->children[i]->poly_deg_rec() != 0 ) {
                additional_int = -1;
                break;
            }
        }
    } else {
        additional_int = -1;
    }
    
    return additional_int;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ser_repr_rec( std::ostream &os, const Op *op ) {
    if ( op->op_id == op->current_op )
        return;
    op->op_id = op->current_op;
    //
    if ( op->type == Op::NUMBER )
        os << 'N' << op->number_data()->val;
    else if ( op->type == Op::SYMBOL )
        os << 'S' << strlen( op->symbol_data()->cpp_name_str ) << '_' << op->symbol_data()->cpp_name_str;
    else {
        std::string s( Nstring(op->type) );
        os << 'O' << s.size() << '_' << s;
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i)
            ser_repr_rec( os, op->func_data()->children[i] );
    }
}

void Op::ser_repr( std::ostream &os ) const {
    ++Op::current_op;
    ser_repr_rec( os, this );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool Op::depends_on_rec( const Op *a ) const {
    if ( op_id == current_op )
        return false;
    op_id = current_op;
    if ( this == a )
        return true;
    if ( type > 0 ) {
        if ( func_data()->children[0]->depends_on_rec( a ) ) return true;
        return ( func_data()->children[1] and func_data()->children[1]->depends_on_rec( a ) );
    }
    return false;
}

bool Op::depends_on( const Op *a ) const {
    ++Op::current_op;
    return depends_on_rec( a );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool graphviz_repr_rec( std::ostream &os, const Op *op, const SplittedVec<const Op *,32> &dep ) {
    if ( op->op_id == Op::current_op )
        return op->additional_info; // already outputted ?
    op->op_id = Op::current_op;
    
    bool has_dep = false;
    if ( op->type == Op::NUMBER )
        os << "    node" << op << " [label=\"" << op->number_data()->val << "\"];\n";
    else if ( op->type == Op::SYMBOL ) {
        os << "    node" << op << " [label=\"" << op->symbol_data()->cpp_name_str << "\"];\n";
    } else {
        os << "    node" << op << " [label=\"" << Nstring( op->type ) << "\"];\n";
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
            bool h = graphviz_repr_rec( os, op->func_data()->children[i], dep );
            os << "    node" << op << " -> node" << op->func_data()->children[i] << " [color=" << ( h ? "red" : "black" ) << "];\n";
            has_dep |= h;
        }
    }
    bool h = has_dep or dep.has( op );
    op->additional_info = const_cast<Op *>( h ? op : NULL );
    return h;
}

void Op::graphviz_repr( std::ostream &os, const SplittedVec<const Op *,32> &dep ) const {
    ++Op::current_op;
    graphviz_repr_rec( os, this, dep );
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
void get_sub_symbols_rec( const Op *op, SplittedVec<const Op *,32> &symbols ) {
    if ( op->op_id == Op::current_op )
        return;
    op->op_id = Op::current_op;
    //
    if ( op->type == Op::SYMBOL )
        symbols.push_back( op );
    else if ( op->type > 0 )
        for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i)
            get_sub_symbols_rec( op->func_data()->children[i], symbols );
}

void get_sub_symbols( const Op *op, SplittedVec<const Op *,32> &symbols ) {
    ++Op::current_op;
    get_sub_symbols_rec( op, symbols );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Op::find_discontinuities_rec( const Op *var, SimpleVector<Op *> &discontinuities ) const {
    if ( op_id == current_op )
        return;
    op_id = current_op;
    
    //
    if ( type > 0 ) {
        if ( ( type == STRING_heaviside_NUM or type == STRING_abs_NUM or type == STRING_pos_part_NUM ) and func_data()->children[0]->depends_on( var ) ) {
            discontinuities.push_back_unique( func_data()->children[0] );
            return;
        }
        // else
        func_data()->children[0]->find_discontinuities_rec( var, discontinuities );
        if ( func_data()->children[1] )
            func_data()->children[1]->find_discontinuities_rec( var, discontinuities );
    }
}
void Op::find_discontinuities( const Op *var, SimpleVector<Op *> &discontinuities ) const {
    ++current_op;
    find_discontinuities_rec( var, discontinuities );
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Op::nb_nodes_rec() const {
    if ( op_id == current_op )
        return 0;
    op_id = current_op;
    //
    if ( type > 0 ) {
        int res = 1;
        for(unsigned i=0;i<FuncData::max_nb_children and func_data()->children[i];++i)
            res += func_data()->children[i]->nb_nodes_rec();
        return res;
    }
    return type == SYMBOL;
}

int Op::nb_nodes() const {
    ++current_op;
    return nb_nodes_rec();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Op::nb_ops_rec() const {
    if ( op_id == current_op )
        return 0;
    op_id = current_op;
    //
    if ( type > 0 ) {
        int res = 1;
        for(unsigned i=0;i<FuncData::max_nb_children and func_data()->children[i];++i)
            res += func_data()->children[i]->nb_ops_rec();
        return res;
    }
    return 0;
}

int Op::nb_ops() const {
    ++current_op;
    return nb_ops_rec();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
int Op::nb_nodes_of_type_rec( int t ) const {
    if ( op_id == current_op )
        return 0;
    op_id = current_op;
    //
    if ( type > 0 ) {
        int res = ( type == t );
        for(unsigned i=0;i<FuncData::max_nb_children and func_data()->children[i];++i)
            res += func_data()->children[i]->nb_nodes_of_type_rec( t );
        return res;
    }
    return type == t;
}

int Op::nb_nodes_of_type( int t ) const {
    ++current_op;
    return nb_nodes_of_type_rec( t );
}

void get_child_not_of_type_mul( const Op *op, SplittedVec<const Op *,32> &res ) {
    if ( op->type != STRING_mul_NUM )
        res.push_back( op );
    else {
        get_child_not_of_type_mul( op->func_data()->children[0], res );
        get_child_not_of_type_mul( op->func_data()->children[1], res );
    }
}

void get_child_not_of_type_add( const Op *op, SplittedVec<const Op *,32> &res ) {
    if ( op->type != STRING_add_NUM )
        res.push_back( op );
    else {
        get_child_not_of_type_add( op->func_data()->children[0], res );
        get_child_not_of_type_add( op->func_data()->children[1], res );
    }
}

/*
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

*/

