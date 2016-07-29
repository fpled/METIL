#include "polynomial_expansion.h"

const Ex *PolynomialExpansion::complete_if_necessary( const Ex *r_, Op *ch ) {
    if ( not r_ ) {
        Ex *r = tmp_vec.get_room_for( nb_elements );
        ch->additional_info = reinterpret_cast<Op *>( r );
        r[ 0 ] = ch;
        return r;
    }
    return r_;
}
void PolynomialExpansion::exp_rec_add( Op *a ) {
    Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
    Op *ch_1 = a->func_data()->children[1]; exp_rec( ch_1 ); const Ex *r_1 = reinterpret_cast<const Ex *>( ch_1->additional_info );
    if ( r_0==NULL and r_1==NULL ) {
        a->additional_info = NULL;
    } else {
        r_0 = complete_if_necessary( r_0, ch_0 );
        r_1 = complete_if_necessary( r_1, ch_1 );
        Ex *r = tmp_vec.get_room_for( nb_elements );
        a->additional_info = reinterpret_cast<Op *>( r );
        for(unsigned i=0;i<nb_elements;++i)
            r[i] = r_0[ i ] + r_1[ i ];
    }
}
void PolynomialExpansion::exp_rec_mul( Op *a ) {
    Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
    Op *ch_1 = a->func_data()->children[1]; exp_rec( ch_1 ); const Ex *r_1 = reinterpret_cast<const Ex *>( ch_1->additional_info );
    if ( r_0==NULL and r_1==NULL ) {
        a->additional_info = NULL;
    } else {
        r_0 = complete_if_necessary( r_0, ch_0 );
        r_1 = complete_if_necessary( r_1, ch_1 );
        Ex *r = tmp_vec.get_room_for( nb_elements );
        a->additional_info = reinterpret_cast<Op *>( r );
        for(unsigned i=0;i<nb_elements;++i) {
            r[i] = r_0[ 0 ] * r_1[ i ];
            for(unsigned j=1;j<=i;++j)
                r[i] += r_0[ j ] * r_1[ i - j ];
        }
    }
}
void PolynomialExpansion::exp_rec_abs( Op *a ) {
    Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
    if ( r_0==NULL ) {
        a->additional_info = NULL;
    } else {
        Ex *r = tmp_vec.get_room_for( nb_elements );
        a->additional_info = reinterpret_cast<Op *>( r );
        Ex m = 2 * heaviside( r_0[ 0 ] ) - 1;
        r[ 0 ] = abs( r_0[ 0 ] );
        for(unsigned i=1;i<nb_elements;++i)
            r[ i ] = m * r_0[ i ];
    }
}
void PolynomialExpansion::exp_rec_ppa( Op *a ) {
    Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
    if ( r_0==NULL ) {
        a->additional_info = NULL;
    } else {
        Ex *r = tmp_vec.get_room_for( nb_elements );
        a->additional_info = reinterpret_cast<Op *>( r );
        Ex m = heaviside( r_0[ 0 ] );
        r[ 0 ] = pos_part( r_0[ 0 ] );
        for(unsigned i=1;i<nb_elements;++i)
            r[i] = m * r_0[ i ];
    }
}
void PolynomialExpansion::exp_rec( Op *a ) {
    if ( a->op_id == Op::current_op ) // already done ?
        return;
    a->op_id = Op::current_op;
    //
    switch ( a->type ) {
        case Op::NUMBER:           a->additional_info = NULL; break;
        case Op::SYMBOL:           a->additional_info = NULL; break;
        case STRING_heaviside_NUM: a->additional_info = NULL; break;
        case STRING_eqz_NUM:       a->additional_info = NULL; break;
        case STRING_add_NUM:       exp_rec_add( a ); break;
        case STRING_mul_NUM:       exp_rec_mul( a ); break;
        case STRING_log_NUM:       exp_rec_log( a ); break;
        case STRING_abs_NUM:       exp_rec_abs( a ); break;
        case STRING_pos_part_NUM:  exp_rec_ppa( a ); break;
        case STRING_exp_NUM:       exp_rec_exp( a ); break;
        
        case STRING_sin_NUM:       exp_rec_sin( a ); break;
        case STRING_cos_NUM:       exp_rec_cos( a ); break;
        case STRING_tan_NUM:       exp_rec_tan( a ); break;
        case STRING_asin_NUM:      exp_rec_asin( a ); break;
        case STRING_acos_NUM:      exp_rec_acos( a ); break;
        case STRING_atan_NUM:      exp_rec_atan( a ); break;
        case STRING_atan2_NUM:     exp_rec_atan2( a ); break;
        case STRING_pow_NUM:       exp_rec_pow( a ); break;
        default:
            th->add_error( "for now, no rules for PolynomialExpansion for functions of type '"+std::string(Nstring(a->type))+"'. -> see file 'ex.cpp'.", tok );
    }
}


void polynomial_expansion( Thread *th, const void *tok, const SEX &expressions, const Ex &var, int order, SEX &res, Ex center ) {
    PolynomialExpansion pe( th, tok, order );
    //
    Ex *r = pe.tmp_vec.get_room_for( order + 1 );
    var.op->additional_info = reinterpret_cast<Op *>( r );
    r[0] = center;
    r[1] = pe.one;
    var.op->op_id = ++Op::current_op;
    //
    for(unsigned n=0;n<expressions.size();++n) {
        pe.exp_rec( expressions[n].op );
        pe.complete_if_necessary( reinterpret_cast<Ex *>( expressions[n].op->additional_info ), expressions[n].op );
    }
    //
    for(unsigned n=0,c=0;n<expressions.size();++n)
        for(int i=0;i<=order;++i,++c)
            res[c] = reinterpret_cast<Ex *>( expressions[n].op->additional_info )[i].op;
}

void polynomial_expansion( Thread *th, const void *tok, const VarArgs &expressions, const Ex &var, int order, VarArgs &res, Ex center ) {
    assert( res.nb_uargs() >= ( order + 1 ) * expressions.nb_uargs() );
    SEX sex_expressions;
    for(unsigned i=0;i<expressions.nb_uargs();++i)
        sex_expressions.push_back( *reinterpret_cast<Ex *>(expressions.uarg(i)->data) );
    //
    SEX sex_res; sex_res.get_room_for( res.nb_uargs() );
    polynomial_expansion( th, tok, sex_expressions, var, order, sex_res, center );
    for(unsigned i=0;i<res.nb_uargs();++i)
        *reinterpret_cast<Ex *>(res.uarg(i)->data) = sex_res[ i ];
}
