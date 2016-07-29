#include "ex.h"
#include "op.h"
#include "thread.h"

// ------------------------------------------------------------------------------------------------------------
struct QuadraticExpansion {
    QuadraticExpansion( Thread *th, const void *tok, int nb_variables ) : th(th), tok(tok), zero(0), one(1), nb_variables(nb_variables) {
        nb_elements = 1 + nb_variables + ( nb_variables + 1 ) * nb_variables / 2;
    }
    
    const Ex *complete_if_necessary( const Ex *r_, Op *ch ) {
        if ( not r_ ) {
            Ex *r = tmp_vec.get_room_for( nb_elements );
            ch->additional_info = reinterpret_cast<Op *>( r );
            r[ 0 ] = ch;
            return r;
        }
        return r_;
    }
    struct QI {
        QI( Ex *r, unsigned nb_variables ) : r(r), nb_variables(nb_variables) {}
        Ex &scal() { return r[0]; }
        Ex &vec(unsigned i) { return r[1+i]; }
        Ex &mat(unsigned i,unsigned j) { return r[1+nb_variables+i*(i+1)/2+j]; }
        Ex *r;
        unsigned nb_variables;
    };
    struct CQI {
        CQI( const Ex *r, unsigned nb_variables ) : r(r), nb_variables(nb_variables) {}
        const Ex &scal() const { return r[0]; }
        const Ex &vec(unsigned i) const { return r[1+i]; }
        const Ex &mat(unsigned i,unsigned j) const { return r[1+nb_variables+i*(i+1)/2+j]; }
        const Ex *r;
        unsigned nb_variables;
    };
    void exp_rec_add( Op *a ) {
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
    void exp_rec_mul( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        Op *ch_1 = a->func_data()->children[1]; exp_rec( ch_1 ); const Ex *r_1 = reinterpret_cast<const Ex *>( ch_1->additional_info );
        if ( r_0==NULL and r_1==NULL ) {
            a->additional_info = NULL;
        } else {
            r_0 = complete_if_necessary( r_0, ch_0 ); CQI q_0( r_0, nb_variables );
            r_1 = complete_if_necessary( r_1, ch_1 ); CQI q_1( r_1, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r  , nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            q.scal() = q_0.scal() * q_1.scal();
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = q_0.scal() * q_1.vec(i) + q_0.vec(i) * q_1.scal();
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = q_0.scal() * q_1.mat(i,j) + q_0.mat(i,j) * q_1.scal() + q_0.vec(i) * q_1.vec(j) + q_0.vec(j) * q_1.vec(i);
        }
    }
    
    void exp_rec_abs( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            Ex *r = tmp_vec.get_room_for( nb_elements );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex m = 2 * heaviside( r_0[0] ) - 1;
            r[ 0 ] = abs( r_0[ 0 ] );
            for(unsigned i=1;i<nb_elements;++i)
                r[ i ] = m * r_0[ i ];
        }
    }
    
    void exp_rec_ppa( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            Ex *r = tmp_vec.get_room_for( nb_elements );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex m = heaviside( r_0[0] );
            r[ 0 ] = pos_part( r_0[ 0 ] );
            for(unsigned i=1;i<nb_elements;++i)
                r[ i ] = m * r_0[ i ];
        }
    }
    
    void exp_rec_exp( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex e = exp( q_0.scal() );
            q.scal() = e;
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = e * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = e * ( q_0.mat(i,j) + q_0.vec(i) * q_0.vec(j) );
        }
    }
    
    void exp_rec_log( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = 1 / q_0.scal();
            q.scal() = log( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * ( q_0.mat(i,j) - d * q_0.vec(i) * q_0.vec(j) );
        }
    }
    
    /*
a := symbol("q_0.scal()")
for f in [ sin cos tan asin acos atan ]
    stdout <<<<
            void exp_rec_$f( Op *a ) {
                Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
                if ( r_0==NULL ) {
                    a->additional_info = NULL;
                } else {
                    CQI q_0( r_0, nb_variables );
                    Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
                    a->additional_info = reinterpret_cast<Op *>( r );
                    Ex d = $( f(a).diff(a) );
                    Ex d2 = $( f(a).diff(a).diff(a) );
                    q.scal() = $f( q_0.scal() );
                    for(unsigned i=0;i<nb_variables;++i)
                        q.vec(i) = d * q_0.vec(i);
                    for(unsigned i=0;i<nb_variables;++i)
                        for(unsigned j=0;j<=i;++j)
                            q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j) );
                }
            }
    */
    // ********************************************************************************
    void exp_rec_sin( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = cos(q_0.scal());
            Ex d2 = -sin(q_0.scal());
            q.scal() = sin( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j);
        }
    }

    void exp_rec_cos( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = -sin(q_0.scal());
            Ex d2 = -cos(q_0.scal());
            q.scal() = cos( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j);
        }
    }

    void exp_rec_tan( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = 1+pow(tan(q_0.scal()),2);
            Ex d2 = 2*tan(q_0.scal())*(1+pow(tan(q_0.scal()),2));
            q.scal() = tan( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j);
        }
    }

    void exp_rec_asin( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = pow(1-pow(q_0.scal(),2),-1.0/2.0);
            Ex d2 = q_0.scal()*pow(1-pow(q_0.scal(),2),-3.0/2.0);
            q.scal() = asin( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j);
        }
    }

    void exp_rec_acos( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = -pow(1-pow(q_0.scal(),2),-1.0/2.0);
            Ex d2 = -q_0.scal()*pow(1-pow(q_0.scal(),2),-3.0/2.0);
            q.scal() = acos( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j);
        }
    }

    void exp_rec_atan( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        if ( r_0==NULL ) {
            a->additional_info = NULL;
        } else {
            CQI q_0( r_0, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r, nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex d = pow(1+pow(q_0.scal(),2),-1);
            Ex d2 = (-2)*q_0.scal()*pow(1+pow(q_0.scal(),2),-2);
            q.scal() = atan( q_0.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = d * q_0.vec(i);
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = d * q_0.mat(i,j) + d2 * q_0.vec(i) * q_0.vec(j);
        }
    }
    
    void exp_rec_atan2( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        Op *ch_1 = a->func_data()->children[1]; exp_rec( ch_1 ); const Ex *r_1 = reinterpret_cast<const Ex *>( ch_1->additional_info );
        if ( r_0==NULL and r_1==NULL ) {
            a->additional_info = NULL;
        } else {
            r_0 = complete_if_necessary( r_0, ch_0 ); CQI q_0( r_0, nb_variables );
            r_1 = complete_if_necessary( r_1, ch_1 ); CQI q_1( r_1, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r  , nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            Ex dy = q_1.scal()*pow(pow(q_0.scal(),2)+pow(q_1.scal(),2),-1);
            Ex dx = -q_0.scal()*pow(pow(q_0.scal(),2)+pow(q_1.scal(),2),-1);
            Ex dy2 = (-2)*q_1.scal()*q_0.scal()*pow(pow(q_0.scal(),2)+pow(q_1.scal(),2),-2);
            Ex dx2 = 2*q_1.scal()*q_0.scal()*pow(pow(q_0.scal(),2)+pow(q_1.scal(),2),-2);
            Ex dydx = (pow(q_1.scal(),2)-pow(q_0.scal(),2))*pow(pow(q_0.scal(),2)+pow(q_1.scal(),2),-2);
            q.scal() = atan2( q_0.scal(), q_1.scal() );
            for(unsigned i=0;i<nb_variables;++i)
                q.vec(i) = dy * q_0.vec(i) + dx * q_1.scal();
            for(unsigned i=0;i<nb_variables;++i)
                for(unsigned j=0;j<=i;++j)
                    q.mat(i,j) = dy * q_0.mat(i,j) + dx * q_1.mat(i,j) + dy2 * q_0.vec(i) * q_0.vec(j) + dx2 * q_1.vec(i) * q_1.vec(j) + dydx * ( q_0.vec(i) * q_1.vec(j) + q_0.vec(j) * q_1.vec(i) );
        }
    }

    // ********************************************************************************
    void exp_rec_pow( Op *a ) {
        Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
        Op *ch_1 = a->func_data()->children[1]; exp_rec( ch_1 ); const Ex *r_1 = reinterpret_cast<const Ex *>( ch_1->additional_info );
        if ( r_0==NULL and r_1==NULL ) {
            a->additional_info = NULL;
        } else {
            r_0 = complete_if_necessary( r_0, ch_0 ); CQI q_0( r_0, nb_variables );
            r_1 = complete_if_necessary( r_1, ch_1 ); CQI q_1( r_1, nb_variables );
            Ex *r = tmp_vec.get_room_for( nb_elements ); QI q( r  , nb_variables );
            a->additional_info = reinterpret_cast<Op *>( r );
            bool r_1_is_a_cst = true; for(unsigned i=1;i<nb_elements;++i) r_1_is_a_cst &= r_1[i].op->is_zero();
            if ( r_1_is_a_cst ) {
                // Rationnal n = r_1[0].op->number_data()->val;
                Ex d = q_1.scal() * pow( q_0.scal(), q_1.scal() - 1 );
                Ex d2 = ( q_1.scal() - 1 ) * q_1.scal() * pow( q_0.scal(), q_1.scal() - 2 );
                q.scal() = pow( q_0.scal(), q_1.scal() );
                for( unsigned i = 0; i < nb_variables; ++i )
                    q.vec(i) = d * q_0.vec(i);
                for( unsigned i = 0; i < nb_variables; ++i )
                    for( unsigned j = 0; j <= i; ++j )
                        q.mat(i,j) = d * q_0.mat( i, j ) + d2 * q_0.vec( i ) * q_0.vec( j );
            } else {
                th->add_error( "TODO : quadratic expansion for x^y when y contains a variable.", tok );
                assert( 0 );
            }
        }
    }
    
    
    void exp_rec( Op *a ) {
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
                th->add_error( "for now, no rules for QuadraticExpansion for functions of type '"+std::string(Nstring(a->type))+"'. -> see file 'ex.cpp'.", tok );
        }
    }
    
    Thread *th;
    const void *tok;
    Ex zero, one;
    unsigned nb_elements, nb_variables;
    SplittedVec<Ex,2048,2048,true> tmp_vec;
};

void quadratic_expansion( Thread *th, const void *tok, const SEX &expressions, const SEX &variables, SEX &res ) {
    QuadraticExpansion pe( th, tok, variables.size() );
    //
    ++Op::current_op;
    for(unsigned i=0;i<variables.size();++i) {
        const Ex &var = variables[i];
        Ex *r = pe.tmp_vec.get_room_for( pe.nb_elements );
        var.op->additional_info = reinterpret_cast<Op *>( r );
        r[ 1 + i ] = pe.one;
        var.op->op_id = Op::current_op;
    }
    //
    for(unsigned n=0;n<expressions.size();++n) {
        pe.exp_rec( expressions[n].op );
        pe.complete_if_necessary( reinterpret_cast<Ex *>( expressions[n].op->additional_info ), expressions[n].op );
    }
    //
    for(unsigned n=0,c=0;n<expressions.size();++n)
        for(unsigned i=0;i<pe.nb_elements;++i,++c)
            res[ c ] = reinterpret_cast<Ex *>( expressions[n].op->additional_info )[ i ];
}

void quadratic_expansion( Thread *th, const void *tok, const VarArgs &expressions, const VarArgs &variables, VarArgs &res ) {
    int s = variables.nb_uargs();
    int n = s * ( s + 1 ) / 2 + s + 1;
    assert( res.nb_uargs() >= n * expressions.nb_uargs() );
    //
    SEX sex_expressions;
    for(unsigned i=0;i<expressions.nb_uargs();++i)
        sex_expressions.push_back( *reinterpret_cast<Ex *>(expressions.uarg(i)->data) );
    //
    SEX sex_variables;
    for(unsigned i=0;i<variables.nb_uargs();++i)
        sex_variables.push_back( *reinterpret_cast<Ex *>(variables.uarg(i)->data) );
    //
    SEX sex_res; sex_res.get_room_for( n );
    quadratic_expansion( th, tok, sex_expressions, sex_variables, sex_res );
    for(int i=0;i<n;++i)
        *reinterpret_cast<Ex *>(res.uarg(i)->data) = sex_res[ i ];
}
