#include "polynomial_expansion.h"

void PolynomialExpansion::exp_rec_pow( Op *a ) {
    Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
    Op *ch_1 = a->func_data()->children[1]; exp_rec( ch_1 ); const Ex *r_1 = reinterpret_cast<const Ex *>( ch_1->additional_info );
    if ( r_0==NULL and r_1==NULL ) {
        a->additional_info = NULL;
    } else {
        r_0 = complete_if_necessary( r_0, ch_0 );
        r_1 = complete_if_necessary( r_1, ch_1 );
        Ex *r = tmp_vec.get_room_for( nb_elements );
        a->additional_info = reinterpret_cast<Op *>( r );
        //
        bool r_1_is_a_cst = true; for(unsigned i=1;i<nb_elements;++i) r_1_is_a_cst &= r_1[i].op->is_zero();
        if ( r_1_is_a_cst ) {
            if ( r_1[0].op->type == Op::NUMBER ) {
                Rationnal n = r_1[0].op->number_data()->val;
                if ( n.is_integer() and abs(n.num) < 50 ) { // a ^ n
                    int abs_n = abs(n.num);
                    const Ex *r_o = r_0;
                    if ( n.is_neg() ) { // a ^ ( -... )
                        Ex *t_o = tmp_vec.get_room_for( nb_elements );
                        const Ex *r_o = t_o;
                        switch ( nb_elements ) {
                            case 9:
                                t_o[8] = -40320*(-4*r_0[1]*r_0[1]*r_0[1]*r_0[5]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+5*r_0[1]*r_0[1]*r_0[1]*r_0[1]*
                                    r_0[4]*r_0[0]*r_0[0]*r_0[0]-6*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[3]*r_0[0]*r_0[0]+7*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]
                                    *r_0[2]*r_0[0]-r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]+3*r_0[2]*r_0[3]*r_0[3]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-
                                    r_0[2]*r_0[2]*r_0[2]*r_0[2]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-r_0[4]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-2*r_0[7]*r_0[1]*r_0[0]
                                    *r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+3*r_0[1]*r_0[1]*r_0[6]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-2*r_0[2]*r_0[6]*r_0[0]*r_0[0]*
                                    r_0[0]*r_0[0]*r_0[0]*r_0[0]-2*r_0[3]*r_0[5]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+r_0[8]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]
                                    *r_0[0]+20*r_0[1]*r_0[1]*r_0[1]*r_0[2]*r_0[3]*r_0[0]*r_0[0]*r_0[0]-12*r_0[1]*r_0[2]*r_0[2]*r_0[3]*r_0[0]*r_0[0]*r_0[0]*r_0[0]
                                    -6*r_0[1]*r_0[1]*r_0[3]*r_0[3]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-12*r_0[1]*r_0[1]*r_0[2]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+6*
                                    r_0[1]*r_0[3]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+3*r_0[2]*r_0[2]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+6*r_0[1]*r_0[2]
                                    *r_0[5]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-15*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[2]*r_0[2]*r_0[0]*r_0[0]+10*r_0[1]*r_0[1]*r_0[2]*
                                    r_0[2]*r_0[2]*r_0[0]*r_0[0]*r_0[0])/(r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]);
                            case 8:
                                t_o[7] = -5040*(-2*r_0[1]*r_0[6]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-2*r_0[2]*r_0[5]*r_0[0]*r_0[0]*r_0[0]*
                                    r_0[0]*r_0[0]-2*r_0[3]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+3*r_0[1]*r_0[1]*r_0[5]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+3*r_0[2]
                                    *r_0[2]*r_0[3]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-4*r_0[1]*r_0[1]*r_0[1]*r_0[4]*r_0[0]*r_0[0]*r_0[0]+5*r_0[1]*r_0[1]*r_0[1]*r_0[1]*
                                    r_0[3]*r_0[0]*r_0[0]+10*r_0[1]*r_0[1]*r_0[1]*r_0[2]*r_0[2]*r_0[0]*r_0[0]+r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]-6*
                                    r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[2]*r_0[0]-12*r_0[1]*r_0[1]*r_0[2]*r_0[3]*r_0[0]*r_0[0]*r_0[0]-4*r_0[1]*r_0[2]*r_0[2]*
                                    r_0[2]*r_0[0]*r_0[0]*r_0[0]+6*r_0[1]*r_0[2]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+3*r_0[1]*r_0[3]*r_0[3]*r_0[0]*r_0[0]*r_0[0]*r_0[0]
                                    +r_0[7]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0])/(r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]);
                            case 7:
                                t_o[6] = -720*(-r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]+5*r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[2]*r_0[0]-6*r_0[1]
                                    *r_0[1]*r_0[2]*r_0[2]*r_0[0]*r_0[0]-4*r_0[1]*r_0[1]*r_0[1]*r_0[3]*r_0[0]*r_0[0]+r_0[2]*r_0[2]*r_0[2]*r_0[0]*r_0[0]*r_0[0]+6*
                                    r_0[1]*r_0[2]*r_0[3]*r_0[0]*r_0[0]*r_0[0]+3*r_0[1]*r_0[1]*r_0[4]*r_0[0]*r_0[0]*r_0[0]-r_0[3]*r_0[3]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-2
                                    *r_0[2]*r_0[4]*r_0[0]*r_0[0]*r_0[0]*r_0[0]-2*r_0[1]*r_0[5]*r_0[0]*r_0[0]*r_0[0]*r_0[0]+r_0[6]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0])/(
                                    r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]);
                            case 6:
                                t_o[5] = -120*(r_0[1]*r_0[1]*r_0[1]*r_0[1]*r_0[1]-4*r_0[1]*r_0[1]*r_0[1]*r_0[2]*r_0[0]+3*r_0[1]*r_0[2]*r_0[2]*
                                    r_0[0]*r_0[0]+3*r_0[1]*r_0[1]*r_0[3]*r_0[0]*r_0[0]-2*r_0[2]*r_0[3]*r_0[0]*r_0[0]*r_0[0]-2*r_0[1]*r_0[4]*r_0[0]*r_0[0]*r_0[0]
                                    +r_0[5]*r_0[0]*r_0[0]*r_0[0]*r_0[0])/(r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]);
                            case 5:
                                t_o[4] = -24*(-r_0[1]*r_0[1]*r_0[1]*r_0[1]+3*r_0[1]*r_0[1]*r_0[2]*r_0[0]-r_0[2]*r_0[2]*r_0[0]*r_0[0]-2*r_0[1]*
                                    r_0[3]*r_0[0]*r_0[0]+r_0[4]*r_0[0]*r_0[0]*r_0[0])/(r_0[0]*r_0[0]*r_0[0]*r_0[0]*r_0[0]);
                            case 4:
                                t_o[3] = -6*(r_0[1]*r_0[1]*r_0[1]-2*r_0[1]*r_0[2]*r_0[0]+r_0[3]*r_0[0]*r_0[0])/(r_0[0]*r_0[0]*r_0[0]*r_0[0]);
                            case 3:
                                t_o[2] = -2*(-r_0[1]*r_0[1]+r_0[2]*r_0[0])/(r_0[0]*r_0[0]*r_0[0]);
                            case 2:
                                t_o[1] = -1/(r_0[0]*r_0[0])*r_0[1];
                            case 1:
                                t_o[0] = 1/r_0[0];
                                break;
                            default:
                                th->add_error( "TODO : PolynomialExpansion for pow( a, < - 7 ). -> see file 'ex.cpp'.", tok );
                        }
                        if ( abs_n == 1 )
                            for(unsigned i=0;i<nb_elements;++i)
                                r[ i ] = r_o[ i ];
                    }
                    if ( abs_n > 1 ) {
                        Ex *r_n = tmp_vec.get_room_for( nb_elements );
                        for(unsigned i=0;i<nb_elements;++i)
                            r_n[ i ] = r_o[ i ];
                        for(int c=1;c<abs_n;++c) {
                            for(unsigned i=0;i<nb_elements;++i) {
                                r[ i ] = r_n[ 0 ] * r_o[ i ];
                                for(unsigned j=1;j<=i;++j)
                                    r[ i ] += r_n[ j ] * r_o[ i - j ];
                            }
                            if ( c + 1 < abs_n )
                                for(unsigned i=0;i<nb_elements;++i)
                                    r_n[ i ] = r[ i ];
                        }
                    }
                } else if ( n.is_one_half() ) { // a ^ 0.5
                    #include "series_sqrt.h"
                } else if ( n.is_minus_one_half() ) { // a ^ (-0.5)
                    #include "series_rsqrt.h"
                } else {
                    // std::ostringstream s; s << n;
                    // th->add_error( "TODO : PolynomialExpansion for pow( a, "+s.str()+" ). -> see file 'ex.cpp'.", tok );
                    #include "series_pow.h"
                }
            } else {
                th->add_error( "TODO : PolynomialExpansion for pow( a, not a simple number ). -> see file 'ex.cpp'.", tok );
            }
        } else {
            th->add_error( "TODO : PolynomialExpansion for pow( a, not a constant ). -> see file 'ex.cpp'.", tok );
        }
    }
}
