#include "polynomial_expansion.h"

void PolynomialExpansion::exp_rec_cos( Op *a ) {
    Op *ch_0 = a->func_data()->children[0]; exp_rec( ch_0 ); const Ex *r_0 = reinterpret_cast<const Ex *>( ch_0->additional_info );
    if ( ch_0==NULL ) {
        a->additional_info = NULL;
    } else {
        r_0 = complete_if_necessary( r_0, ch_0 );
        Ex *r = tmp_vec.get_room_for( nb_elements );
        a->additional_info = reinterpret_cast<Op *>( r );
        //
        switch ( nb_elements ) {
            case 1: {
                Ex R_0 = r_0[0]; R_0 = cos(R_0); r[0] = R_0; 
                break;
            }
            case 2: {
                Ex R_0 = r_0[1]; Ex R_1 = r_0[0]; Ex R_2 = sin(R_1); R_0 = R_2*R_0;
                R_0 = Rationnal(-1,1)*R_0; r[1] = R_0; R_1 = cos(R_1); r[0] = R_1;
                break;
            }
            case 3: {
                Ex R_0 = r_0[2]; Ex R_1 = r_0[1]; Ex R_2 = pow(R_1,2); Ex R_3 = r_0[0];
                Ex R_4 = sin(R_3); R_0 = R_0*R_4; R_0 = Rationnal(2,1)*R_0; R_1 = R_4*R_1;
                R_1 = Rationnal(-1,1)*R_1; r[1] = R_1; R_3 = cos(R_3); r[0] = R_3;
                R_2 = R_3*R_2; R_0 = R_2+R_0; R_0 = Rationnal(-1,2)*R_0; r[2] = R_0;
                break;
            }
            case 4: {
                Ex R_0 = r_0[3]; Ex R_1 = r_0[2]; Ex R_2 = r_0[1]; Ex R_3 = pow(R_2,2);
                Ex R_4 = r_0[0]; Ex R_5 = sin(R_4); Ex R_6 = R_5*R_3; R_6 = Rationnal(-1,1)*R_6;
                R_0 = R_0*R_5; R_0 = Rationnal(6,1)*R_0; Ex R_7 = R_1*R_5; R_7 = Rationnal(2,1)*R_7;
                R_5 = R_5*R_2; R_5 = Rationnal(-1,1)*R_5; r[1] = R_5; R_4 = cos(R_4);
                r[0] = R_4; R_1 = R_4*R_1; R_1 = Rationnal(6,1)*R_1; R_6 = R_1+R_6;
                R_6 = R_2*R_6; R_6 = R_0+R_6; R_6 = Rationnal(-1,6)*R_6; r[3] = R_6;
                R_3 = R_4*R_3; R_7 = R_3+R_7; R_7 = Rationnal(-1,2)*R_7; r[2] = R_7;
                break;
            }
            case 5: {
                Ex R_0 = r_0[4]; Ex R_1 = r_0[3]; Ex R_2 = r_0[2]; Ex R_3 = r_0[1];
                Ex R_4 = pow(R_3,2); Ex R_5 = r_0[0]; Ex R_6 = sin(R_5); R_0 = R_0*R_6;
                R_0 = Rationnal(24,1)*R_0; Ex R_7 = R_6*R_4; R_7 = Rationnal(-1,1)*R_7; Ex R_8 = R_1*R_6;
                R_8 = Rationnal(6,1)*R_8; Ex R_9 = R_2*R_6; Ex R_10 = Rationnal(-10,1)*R_9; R_9 = Rationnal(2,1)*R_9;
                R_6 = R_6*R_3; R_6 = Rationnal(-1,1)*R_6; r[1] = R_6; R_5 = cos(R_5);
                r[0] = R_5; R_1 = R_5*R_1; R_1 = Rationnal(24,1)*R_1; R_6 = R_5*R_2;
                R_6 = Rationnal(6,1)*R_6; R_7 = R_6+R_7; R_2 = R_2*R_7; R_2 = Rationnal(2,1)*R_2;
                R_2 = R_0+R_2; R_7 = R_3*R_7; R_7 = R_8+R_7; R_7 = Rationnal(-1,6)*R_7;
                r[3] = R_7; R_4 = R_5*R_4; R_5 = Rationnal(-1,1)*R_4; R_10 = R_5+R_10;
                R_10 = R_3*R_10; R_10 = R_1+R_10; R_10 = R_3*R_10; R_10 = R_2+R_10;
                R_10 = Rationnal(-1,24)*R_10; r[4] = R_10; R_9 = R_4+R_9; R_9 = Rationnal(-1,2)*R_9;
                r[2] = R_9; 
                break;
            }
            case 6: {
                Ex R_0 = r_0[5]; Ex R_1 = r_0[4]; Ex R_2 = r_0[3]; Ex R_3 = r_0[2];
                Ex R_4 = r_0[1]; Ex R_5 = pow(R_4,2); Ex R_6 = r_0[0]; Ex R_7 = sin(R_6);
                R_0 = R_0*R_7; R_0 = Rationnal(120,1)*R_0; Ex R_8 = R_1*R_7; R_8 = Rationnal(24,1)*R_8;
                Ex R_9 = R_7*R_5; Ex R_10 = Rationnal(-1,1)*R_9; Ex R_11 = R_2*R_7; Ex R_12 = Rationnal(-54,1)*R_11;
                R_11 = Rationnal(6,1)*R_11; Ex R_13 = R_3*R_7; Ex R_14 = Rationnal(-20,1)*R_13; Ex R_15 = Rationnal(-10,1)*R_13;
                R_13 = Rationnal(2,1)*R_13; R_7 = R_7*R_4; R_7 = Rationnal(-1,1)*R_7; r[1] = R_7;
                R_6 = cos(R_6); r[0] = R_6; R_1 = R_6*R_1; R_1 = Rationnal(120,1)*R_1;
                R_7 = R_6*R_2; Ex R_16 = Rationnal(42,1)*R_7; R_7 = Rationnal(24,1)*R_7; Ex R_17 = R_6*R_3;
                Ex R_18 = Rationnal(-14,1)*R_17; R_18 = R_9+R_18; R_18 = R_4*R_18; R_18 = R_12+R_18;
                R_18 = R_4*R_18; R_17 = Rationnal(6,1)*R_17; R_10 = R_17+R_10; R_2 = R_2*R_10;
                R_2 = Rationnal(6,1)*R_2; R_2 = R_0+R_2; R_0 = R_3*R_10; R_0 = Rationnal(2,1)*R_0;
                R_0 = R_8+R_0; R_10 = R_4*R_10; R_10 = R_11+R_10; R_10 = Rationnal(-1,6)*R_10;
                r[3] = R_10; R_5 = R_6*R_5; R_6 = Rationnal(-2,1)*R_5; R_14 = R_6+R_14;
                R_14 = R_4*R_14; R_14 = R_16+R_14; R_14 = R_3*R_14; R_14 = Rationnal(2,1)*R_14;
                R_16 = Rationnal(-1,1)*R_5; R_15 = R_16+R_15; R_3 = R_3*R_15; R_3 = Rationnal(2,1)*R_3;
                R_1 = R_3+R_1; R_18 = R_1+R_18; R_18 = R_4*R_18; R_18 = R_2+R_18;
                R_14 = R_18+R_14; R_14 = Rationnal(-1,120)*R_14; r[5] = R_14; R_15 = R_4*R_15;
                R_15 = R_7+R_15; R_15 = R_4*R_15; R_15 = R_0+R_15; R_15 = Rationnal(-1,24)*R_15;
                r[4] = R_15; R_13 = R_5+R_13; R_13 = Rationnal(-1,2)*R_13; r[2] = R_13;
                break;
            }
            #ifdef WANT_SERIES_789
            case 7: {
                Ex R_0 = r_0[6]; Ex R_1 = r_0[5]; Ex R_2 = r_0[4]; Ex R_3 = r_0[3];
                Ex R_4 = r_0[2]; Ex R_5 = r_0[1]; Ex R_6 = pow(R_5,2); Ex R_7 = r_0[0];
                Ex R_8 = sin(R_7); R_0 = R_0*R_8; R_0 = Rationnal(720,1)*R_0; Ex R_9 = R_1*R_8;
                R_9 = Rationnal(120,1)*R_9; Ex R_10 = R_2*R_8; Ex R_11 = Rationnal(-336,1)*R_10; R_10 = Rationnal(24,1)*R_10;
                Ex R_12 = R_8*R_6; Ex R_13 = Rationnal(2,1)*R_12; Ex R_14 = Rationnal(3,1)*R_12; Ex R_15 = Rationnal(-1,1)*R_12;
                Ex R_16 = R_3*R_8; Ex R_17 = Rationnal(-84,1)*R_16; Ex R_18 = Rationnal(-156,1)*R_16; Ex R_19 = Rationnal(-54,1)*R_16;
                R_16 = Rationnal(6,1)*R_16; Ex R_20 = R_4*R_8; Ex R_21 = Rationnal(18,1)*R_20; Ex R_22 = Rationnal(-30,1)*R_20;
                Ex R_23 = Rationnal(-20,1)*R_20; Ex R_24 = Rationnal(-10,1)*R_20; R_20 = Rationnal(2,1)*R_20; R_8 = R_8*R_5;
                R_8 = Rationnal(-1,1)*R_8; r[1] = R_8; R_7 = cos(R_7); r[0] = R_7;
                R_1 = R_7*R_1; R_1 = Rationnal(720,1)*R_1; R_8 = R_7*R_2; Ex R_25 = Rationnal(288,1)*R_8;
                R_8 = Rationnal(120,1)*R_8; Ex R_26 = R_7*R_3; Ex R_27 = Rationnal(-96,1)*R_26; Ex R_28 = Rationnal(60,1)*R_26;
                Ex R_29 = Rationnal(42,1)*R_26; R_26 = Rationnal(24,1)*R_26; Ex R_30 = R_7*R_4; Ex R_31 = Rationnal(-28,1)*R_30;
                R_31 = R_13+R_31; R_31 = R_5*R_31; R_31 = R_17+R_31; R_31 = R_4*R_31;
                R_31 = Rationnal(2,1)*R_31; R_17 = Rationnal(-42,1)*R_30; R_17 = R_14+R_17; R_17 = R_5*R_17;
                R_18 = R_17+R_18; R_18 = R_5*R_18; R_25 = R_18+R_25; R_18 = Rationnal(-14,1)*R_30;
                R_18 = R_12+R_18; R_12 = R_4*R_18; R_12 = Rationnal(2,1)*R_12; R_11 = R_12+R_11;
                R_18 = R_5*R_18; R_18 = R_19+R_18; R_18 = R_5*R_18; R_30 = Rationnal(6,1)*R_30;
                R_15 = R_30+R_15; R_2 = R_2*R_15; R_2 = Rationnal(24,1)*R_2; R_2 = R_0+R_2;
                R_0 = R_3*R_15; R_0 = Rationnal(6,1)*R_0; R_0 = R_9+R_0; R_9 = R_4*R_15;
                R_9 = Rationnal(2,1)*R_9; R_9 = R_10+R_9; R_15 = R_5*R_15; R_15 = R_16+R_15;
                R_15 = Rationnal(-1,6)*R_15; r[3] = R_15; R_6 = R_7*R_6; R_21 = R_6+R_21;
                R_21 = R_5*R_21; R_21 = R_27+R_21; R_21 = R_5*R_21; R_21 = R_11+R_21;
                R_21 = R_5*R_21; R_11 = Rationnal(-3,1)*R_6; R_22 = R_11+R_22; R_11 = R_5*R_22;
                R_11 = R_28+R_11; R_11 = R_3*R_11; R_11 = Rationnal(6,1)*R_11; R_22 = R_4*R_22;
                R_22 = Rationnal(2,1)*R_22; R_22 = R_25+R_22; R_22 = R_4*R_22; R_22 = Rationnal(2,1)*R_22;
                R_2 = R_22+R_2; R_11 = R_2+R_11; R_2 = Rationnal(-2,1)*R_6; R_23 = R_2+R_23;
                R_23 = R_5*R_23; R_23 = R_29+R_23; R_23 = R_4*R_23; R_23 = Rationnal(2,1)*R_23;
                R_29 = Rationnal(-1,1)*R_6; R_24 = R_29+R_24; R_3 = R_3*R_24; R_3 = Rationnal(6,1)*R_3;
                R_31 = R_3+R_31; R_31 = R_1+R_31; R_21 = R_31+R_21; R_21 = R_5*R_21;
                R_21 = R_11+R_21; R_21 = Rationnal(-1,720)*R_21; r[6] = R_21; R_4 = R_4*R_24;
                R_4 = Rationnal(2,1)*R_4; R_8 = R_4+R_8; R_18 = R_8+R_18; R_18 = R_5*R_18;
                R_18 = R_0+R_18; R_23 = R_18+R_23; R_23 = Rationnal(-1,120)*R_23; r[5] = R_23;
                R_24 = R_5*R_24; R_24 = R_26+R_24; R_24 = R_5*R_24; R_24 = R_9+R_24;
                R_24 = Rationnal(-1,24)*R_24; r[4] = R_24; R_20 = R_6+R_20; R_20 = Rationnal(-1,2)*R_20;
                r[2] = R_20; 
                break;
            }
            case 8: {
                Ex R_0 = r_0[7]; Ex R_1 = r_0[6]; Ex R_2 = r_0[5]; Ex R_3 = r_0[4];
                Ex R_4 = r_0[3]; Ex R_5 = r_0[2]; Ex R_6 = r_0[1]; Ex R_7 = pow(R_6,2);
                Ex R_8 = r_0[0]; Ex R_9 = sin(R_8); R_0 = R_0*R_9; R_0 = Rationnal(5040,1)*R_0;
                Ex R_10 = R_1*R_9; R_10 = Rationnal(720,1)*R_10; Ex R_11 = R_2*R_9; Ex R_12 = Rationnal(-2400,1)*R_11;
                R_11 = Rationnal(120,1)*R_11; Ex R_13 = R_3*R_9; Ex R_14 = Rationnal(-672,1)*R_13; Ex R_15 = Rationnal(-1248,1)*R_13;
                Ex R_16 = Rationnal(-336,1)*R_13; R_13 = Rationnal(24,1)*R_13; Ex R_17 = R_9*R_7; Ex R_18 = Rationnal(8,1)*R_17;
                Ex R_19 = Rationnal(4,1)*R_17; Ex R_20 = Rationnal(6,1)*R_17; Ex R_21 = Rationnal(2,1)*R_17; Ex R_22 = Rationnal(3,1)*R_17;
                Ex R_23 = Rationnal(-1,1)*R_17; Ex R_24 = R_4*R_9; Ex R_25 = Rationnal(150,1)*R_24; Ex R_26 = Rationnal(-114,1)*R_24;
                Ex R_27 = Rationnal(-330,1)*R_24; Ex R_28 = Rationnal(-306,1)*R_24; Ex R_29 = Rationnal(-84,1)*R_24; Ex R_30 = Rationnal(-156,1)*R_24;
                Ex R_31 = Rationnal(-54,1)*R_24; R_24 = Rationnal(6,1)*R_24; Ex R_32 = R_5*R_9; Ex R_33 = Rationnal(36,1)*R_32;
                Ex R_34 = Rationnal(54,1)*R_32; Ex R_35 = Rationnal(72,1)*R_32; Ex R_36 = Rationnal(-40,1)*R_32; Ex R_37 = Rationnal(18,1)*R_32;
                Ex R_38 = Rationnal(-30,1)*R_32; Ex R_39 = Rationnal(-20,1)*R_32; Ex R_40 = Rationnal(-10,1)*R_32; R_32 = Rationnal(2,1)*R_32;
                R_9 = R_9*R_6; R_9 = Rationnal(-1,1)*R_9; r[1] = R_9; R_8 = cos(R_8);
                r[0] = R_8; R_1 = R_1*R_8; R_1 = Rationnal(5040,1)*R_1; R_9 = R_8*R_2;
                Ex R_41 = Rationnal(2160,1)*R_9; R_9 = Rationnal(720,1)*R_9; Ex R_42 = R_8*R_3; Ex R_43 = Rationnal(-720,1)*R_42;
                Ex R_44 = Rationnal(528,1)*R_42; Ex R_45 = Rationnal(288,1)*R_42; R_42 = Rationnal(120,1)*R_42; Ex R_46 = R_8*R_4;
                Ex R_47 = Rationnal(-138,1)*R_46; Ex R_48 = Rationnal(-264,1)*R_46; Ex R_49 = Rationnal(-378,1)*R_46; Ex R_50 = Rationnal(78,1)*R_46;
                Ex R_51 = Rationnal(-96,1)*R_46; Ex R_52 = Rationnal(60,1)*R_46; Ex R_53 = Rationnal(42,1)*R_46; R_46 = Rationnal(24,1)*R_46;
                Ex R_54 = R_8*R_5; Ex R_55 = Rationnal(22,1)*R_54; R_55 = R_23+R_55; R_55 = R_6*R_55;
                R_55 = R_25+R_55; R_55 = R_6*R_55; R_25 = Rationnal(-112,1)*R_54; R_25 = R_18+R_25;
                R_25 = R_6*R_25; R_25 = R_27+R_25; R_25 = R_5*R_25; R_25 = Rationnal(2,1)*R_25;
                R_27 = Rationnal(-56,1)*R_54; R_27 = R_19+R_27; R_27 = R_5*R_27; R_27 = Rationnal(2,1)*R_27;
                R_19 = Rationnal(-84,1)*R_54; R_19 = R_20+R_19; R_19 = R_6*R_19; R_28 = R_19+R_28;
                R_28 = R_6*R_28; R_19 = Rationnal(-28,1)*R_54; R_19 = R_21+R_19; R_19 = R_6*R_19;
                R_19 = R_29+R_19; R_19 = R_5*R_19; R_19 = Rationnal(2,1)*R_19; R_29 = Rationnal(-42,1)*R_54;
                R_29 = R_22+R_29; R_22 = R_5*R_29; R_22 = Rationnal(2,1)*R_22; R_29 = R_6*R_29;
                R_26 = R_29+R_26; R_26 = R_4*R_26; R_26 = Rationnal(6,1)*R_26; R_30 = R_29+R_30;
                R_30 = R_6*R_30; R_45 = R_30+R_45; R_30 = Rationnal(-14,1)*R_54; R_30 = R_17+R_30;
                R_17 = R_4*R_30; R_17 = Rationnal(6,1)*R_17; R_29 = R_5*R_30; R_29 = Rationnal(2,1)*R_29;
                R_16 = R_29+R_16; R_30 = R_6*R_30; R_30 = R_31+R_30; R_30 = R_6*R_30;
                R_54 = Rationnal(6,1)*R_54; R_23 = R_54+R_23; R_2 = R_2*R_23; R_2 = Rationnal(120,1)*R_2;
                R_0 = R_2+R_0; R_2 = R_3*R_23; R_2 = Rationnal(24,1)*R_2; R_2 = R_10+R_2;
                R_10 = R_4*R_23; R_10 = Rationnal(6,1)*R_10; R_10 = R_11+R_10; R_11 = R_5*R_23;
                R_11 = Rationnal(2,1)*R_11; R_11 = R_13+R_11; R_23 = R_6*R_23; R_23 = R_24+R_23;
                R_23 = Rationnal(-1,6)*R_23; r[3] = R_23; R_7 = R_8*R_7; R_8 = Rationnal(2,1)*R_7;
                R_33 = R_8+R_33; R_33 = R_6*R_33; R_33 = R_47+R_33; R_33 = R_5*R_33;
                R_33 = Rationnal(2,1)*R_33; R_33 = R_17+R_33; R_33 = R_12+R_33; R_12 = Rationnal(3,1)*R_7;
                R_34 = R_12+R_34; R_34 = R_6*R_34; R_34 = R_48+R_34; R_34 = R_6*R_34;
                R_34 = R_14+R_34; R_34 = R_22+R_34; R_34 = R_5*R_34; R_34 = Rationnal(2,1)*R_34;
                R_22 = Rationnal(4,1)*R_7; R_35 = R_22+R_35; R_35 = R_6*R_35; R_35 = R_49+R_35;
                R_35 = R_6*R_35; R_35 = R_15+R_35; R_27 = R_35+R_27; R_27 = R_6*R_27;
                R_35 = Rationnal(-4,1)*R_7; R_36 = R_35+R_36; R_35 = R_4*R_36; R_35 = Rationnal(6,1)*R_35;
                R_35 = R_41+R_35; R_35 = R_27+R_35; R_25 = R_35+R_25; R_25 = R_5*R_25;
                R_25 = Rationnal(2,1)*R_25; R_36 = R_6*R_36; R_36 = R_50+R_36; R_36 = R_3*R_36;
                R_36 = Rationnal(24,1)*R_36; R_37 = R_7+R_37; R_50 = R_5*R_37; R_50 = Rationnal(2,1)*R_50;
                R_43 = R_50+R_43; R_55 = R_43+R_55; R_55 = R_6*R_55; R_55 = R_33+R_55;
                R_55 = R_6*R_55; R_37 = R_6*R_37; R_37 = R_51+R_37; R_37 = R_6*R_37;
                R_37 = R_16+R_37; R_37 = R_6*R_37; R_16 = Rationnal(-3,1)*R_7; R_38 = R_16+R_38;
                R_16 = R_6*R_38; R_16 = R_52+R_16; R_16 = R_4*R_16; R_16 = Rationnal(6,1)*R_16;
                R_38 = R_5*R_38; R_52 = Rationnal(4,1)*R_38; R_44 = R_52+R_44; R_28 = R_44+R_28;
                R_28 = R_4*R_28; R_28 = Rationnal(6,1)*R_28; R_0 = R_28+R_0; R_25 = R_0+R_25;
                R_25 = R_36+R_25; R_38 = Rationnal(2,1)*R_38; R_38 = R_45+R_38; R_38 = R_5*R_38;
                R_38 = Rationnal(2,1)*R_38; R_2 = R_38+R_2; R_16 = R_2+R_16; R_2 = Rationnal(-2,1)*R_7;
                R_39 = R_2+R_39; R_39 = R_6*R_39; R_39 = R_53+R_39; R_39 = R_5*R_39;
                R_39 = Rationnal(2,1)*R_39; R_53 = Rationnal(-1,1)*R_7; R_40 = R_53+R_40; R_3 = R_3*R_40;
                R_3 = Rationnal(24,1)*R_3; R_26 = R_3+R_26; R_26 = R_34+R_26; R_26 = R_1+R_26;
                R_55 = R_26+R_55; R_55 = R_6*R_55; R_55 = R_25+R_55; R_55 = Rationnal(-1,5040)*R_55;
                r[7] = R_55; R_4 = R_4*R_40; R_4 = Rationnal(6,1)*R_4; R_19 = R_4+R_19;
                R_19 = R_9+R_19; R_37 = R_19+R_37; R_37 = R_6*R_37; R_37 = R_16+R_37;
                R_37 = Rationnal(-1,720)*R_37; r[6] = R_37; R_5 = R_5*R_40; R_5 = Rationnal(2,1)*R_5;
                R_42 = R_5+R_42; R_30 = R_42+R_30; R_30 = R_6*R_30; R_30 = R_10+R_30;
                R_39 = R_30+R_39; R_39 = Rationnal(-1,120)*R_39; r[5] = R_39; R_40 = R_6*R_40;
                R_40 = R_46+R_40; R_40 = R_6*R_40; R_40 = R_11+R_40; R_40 = Rationnal(-1,24)*R_40;
                r[4] = R_40; R_32 = R_7+R_32; R_32 = Rationnal(-1,2)*R_32; r[2] = R_32;
                break;
            }
            case 9: {
                Ex R_0 = r_0[8]; Ex R_1 = r_0[7]; Ex R_2 = r_0[6]; Ex R_3 = r_0[5];
                Ex R_4 = r_0[4]; Ex R_5 = r_0[3]; Ex R_6 = r_0[2]; Ex R_7 = r_0[1];
                Ex R_8 = pow(R_7,2); Ex R_9 = r_0[0]; Ex R_10 = sin(R_9); R_0 = R_0*R_10;
                R_0 = Rationnal(40320,1)*R_0; Ex R_11 = R_1*R_10; R_11 = Rationnal(5040,1)*R_11; Ex R_12 = R_2*R_10;
                Ex R_13 = Rationnal(-19440,1)*R_12; R_12 = Rationnal(720,1)*R_12; Ex R_14 = R_3*R_10; Ex R_15 = Rationnal(-5760,1)*R_14;
                Ex R_16 = Rationnal(-10800,1)*R_14; Ex R_17 = Rationnal(-2400,1)*R_14; R_14 = Rationnal(120,1)*R_14; Ex R_18 = R_4*R_10;
                Ex R_19 = Rationnal(-1128,1)*R_18; Ex R_20 = Rationnal(1320,1)*R_18; Ex R_21 = Rationnal(-3240,1)*R_18; Ex R_22 = Rationnal(-3000,1)*R_18;
                Ex R_23 = Rationnal(-672,1)*R_18; Ex R_24 = Rationnal(-1248,1)*R_18; Ex R_25 = Rationnal(-336,1)*R_18; R_18 = Rationnal(24,1)*R_18;
                Ex R_26 = R_10*R_8; Ex R_27 = Rationnal(-4,1)*R_26; Ex R_28 = Rationnal(-3,1)*R_26; Ex R_29 = Rationnal(-2,1)*R_26;
                Ex R_30 = Rationnal(15,1)*R_26; Ex R_31 = Rationnal(5,1)*R_26; Ex R_32 = Rationnal(-5,1)*R_26; Ex R_33 = Rationnal(40,1)*R_26;
                Ex R_34 = Rationnal(10,1)*R_26; Ex R_35 = Rationnal(8,1)*R_26; Ex R_36 = Rationnal(4,1)*R_26; Ex R_37 = Rationnal(6,1)*R_26;
                Ex R_38 = Rationnal(2,1)*R_26; Ex R_39 = Rationnal(3,1)*R_26; Ex R_40 = Rationnal(-1,1)*R_26; Ex R_41 = R_5*R_10;
                Ex R_42 = Rationnal(-504,1)*R_41; Ex R_43 = Rationnal(-144,1)*R_41; Ex R_44 = Rationnal(576,1)*R_41; Ex R_45 = Rationnal(396,1)*R_41;
                Ex R_46 = Rationnal(204,1)*R_41; Ex R_47 = Rationnal(-564,1)*R_41; Ex R_48 = Rationnal(744,1)*R_41; Ex R_49 = Rationnal(-1632,1)*R_41;
                Ex R_50 = Rationnal(150,1)*R_41; Ex R_51 = Rationnal(-114,1)*R_41; Ex R_52 = Rationnal(-330,1)*R_41; Ex R_53 = Rationnal(-306,1)*R_41;
                Ex R_54 = Rationnal(-84,1)*R_41; Ex R_55 = Rationnal(-156,1)*R_41; Ex R_56 = Rationnal(-54,1)*R_41; R_41 = Rationnal(6,1)*R_41;
                Ex R_57 = R_6*R_10; Ex R_58 = Rationnal(-100,1)*R_57; Ex R_59 = Rationnal(108,1)*R_57; Ex R_60 = Rationnal(144,1)*R_57;
                Ex R_61 = Rationnal(-26,1)*R_57; Ex R_62 = Rationnal(270,1)*R_57; Ex R_63 = Rationnal(90,1)*R_57; Ex R_64 = Rationnal(-50,1)*R_57;
                Ex R_65 = Rationnal(180,1)*R_57; Ex R_66 = Rationnal(-600,1)*R_57; Ex R_67 = Rationnal(36,1)*R_57; Ex R_68 = Rationnal(54,1)*R_57;
                Ex R_69 = Rationnal(72,1)*R_57; Ex R_70 = Rationnal(-40,1)*R_57; Ex R_71 = Rationnal(18,1)*R_57; Ex R_72 = Rationnal(-30,1)*R_57;
                Ex R_73 = Rationnal(-20,1)*R_57; Ex R_74 = Rationnal(-10,1)*R_57; R_57 = Rationnal(2,1)*R_57; R_10 = R_10*R_7;
                R_10 = Rationnal(-1,1)*R_10; r[1] = R_10; R_9 = cos(R_9); r[0] = R_9;
                R_1 = R_1*R_9; R_1 = Rationnal(40320,1)*R_1; R_10 = R_2*R_9; Ex R_75 = Rationnal(18000,1)*R_10;
                R_10 = Rationnal(5040,1)*R_10; Ex R_76 = R_9*R_3; Ex R_77 = Rationnal(-6000,1)*R_76; Ex R_78 = Rationnal(4800,1)*R_76;
                Ex R_79 = Rationnal(2160,1)*R_76; R_76 = Rationnal(720,1)*R_76; Ex R_80 = R_9*R_4; Ex R_81 = Rationnal(840,1)*R_80;
                Ex R_82 = Rationnal(-2448,1)*R_80; Ex R_83 = Rationnal(-1272,1)*R_80; Ex R_84 = Rationnal(-3480,1)*R_80; Ex R_85 = Rationnal(-720,1)*R_80;
                Ex R_86 = Rationnal(528,1)*R_80; Ex R_87 = Rationnal(288,1)*R_80; R_80 = Rationnal(120,1)*R_80; Ex R_88 = R_9*R_5;
                Ex R_89 = Rationnal(-504,1)*R_88; Ex R_90 = Rationnal(-528,1)*R_88; Ex R_91 = Rationnal(-180,1)*R_88; Ex R_92 = Rationnal(216,1)*R_88;
                Ex R_93 = Rationnal(-1308,1)*R_88; Ex R_94 = Rationnal(-684,1)*R_88; Ex R_95 = Rationnal(96,1)*R_88; Ex R_96 = Rationnal(-936,1)*R_88;
                Ex R_97 = Rationnal(-138,1)*R_88; Ex R_98 = Rationnal(-264,1)*R_88; Ex R_99 = Rationnal(-378,1)*R_88; Ex R_100 = Rationnal(78,1)*R_88;
                Ex R_101 = Rationnal(-96,1)*R_88; Ex R_102 = Rationnal(60,1)*R_88; Ex R_103 = Rationnal(42,1)*R_88; R_88 = Rationnal(24,1)*R_88;
                Ex R_104 = R_9*R_6; Ex R_105 = Rationnal(88,1)*R_104; R_105 = R_27+R_105; R_105 = R_7*R_105;
                R_105 = R_44+R_105; R_105 = R_7*R_105; R_105 = R_82+R_105; R_82 = Rationnal(66,1)*R_104;
                R_82 = R_28+R_82; R_82 = R_7*R_82; R_82 = R_45+R_82; R_82 = R_7*R_82;
                R_82 = R_83+R_82; R_83 = Rationnal(44,1)*R_104; R_83 = R_29+R_83; R_83 = R_7*R_83;
                R_83 = R_46+R_83; R_83 = R_6*R_83; R_83 = Rationnal(2,1)*R_83; R_46 = Rationnal(-210,1)*R_104;
                R_46 = R_30+R_46; R_30 = R_7*R_46; R_30 = R_47+R_30; R_30 = R_5*R_30;
                R_30 = Rationnal(6,1)*R_30; R_46 = R_6*R_46; R_46 = Rationnal(2,1)*R_46; R_47 = Rationnal(-70,1)*R_104;
                R_47 = R_31+R_47; R_47 = R_5*R_47; R_47 = Rationnal(6,1)*R_47; R_31 = Rationnal(110,1)*R_104;
                R_31 = R_32+R_31; R_31 = R_7*R_31; R_31 = R_48+R_31; R_31 = R_7*R_31;
                R_84 = R_31+R_84; R_31 = Rationnal(-560,1)*R_104; R_31 = R_33+R_31; R_31 = R_7*R_31;
                R_31 = R_49+R_31; R_31 = R_6*R_31; R_49 = Rationnal(-140,1)*R_104; R_49 = R_34+R_49;
                R_34 = R_7*R_49; R_34 = R_42+R_34; R_34 = R_7*R_34; R_34 = R_81+R_34;
                R_49 = R_6*R_49; R_49 = Rationnal(2,1)*R_49; R_81 = Rationnal(22,1)*R_104; R_81 = R_40+R_81;
                R_42 = R_6*R_81; R_42 = Rationnal(2,1)*R_42; R_20 = R_42+R_20; R_81 = R_7*R_81;
                R_81 = R_50+R_81; R_81 = R_7*R_81; R_50 = Rationnal(-112,1)*R_104; R_50 = R_35+R_50;
                R_50 = R_7*R_50; R_50 = R_52+R_50; R_50 = R_6*R_50; R_50 = Rationnal(2,1)*R_50;
                R_52 = Rationnal(-56,1)*R_104; R_52 = R_36+R_52; R_36 = R_7*R_52; R_36 = R_43+R_36;
                R_36 = R_4*R_36; R_36 = Rationnal(24,1)*R_36; R_43 = R_5*R_52; R_43 = Rationnal(6,1)*R_43;
                R_52 = R_6*R_52; R_52 = Rationnal(2,1)*R_52; R_35 = Rationnal(-84,1)*R_104; R_35 = R_37+R_35;
                R_35 = R_7*R_35; R_53 = R_35+R_53; R_53 = R_7*R_53; R_35 = Rationnal(-28,1)*R_104;
                R_35 = R_38+R_35; R_35 = R_7*R_35; R_35 = R_54+R_35; R_35 = R_6*R_35;
                R_35 = Rationnal(2,1)*R_35; R_54 = Rationnal(-42,1)*R_104; R_54 = R_39+R_54; R_39 = R_6*R_54;
                R_38 = Rationnal(4,1)*R_39; R_19 = R_38+R_19; R_39 = Rationnal(2,1)*R_39; R_54 = R_7*R_54;
                R_51 = R_54+R_51; R_51 = R_5*R_51; R_51 = Rationnal(6,1)*R_51; R_55 = R_54+R_55;
                R_55 = R_7*R_55; R_87 = R_55+R_87; R_55 = Rationnal(-14,1)*R_104; R_55 = R_26+R_55;
                R_26 = R_4*R_55; R_26 = Rationnal(24,1)*R_26; R_54 = R_5*R_55; R_54 = Rationnal(6,1)*R_54;
                R_38 = R_6*R_55; R_38 = Rationnal(2,1)*R_38; R_25 = R_38+R_25; R_55 = R_7*R_55;
                R_55 = R_56+R_55; R_55 = R_7*R_55; R_104 = Rationnal(6,1)*R_104; R_40 = R_104+R_40;
                R_2 = R_2*R_40; R_2 = Rationnal(720,1)*R_2; R_0 = R_2+R_0; R_2 = R_3*R_40;
                R_2 = Rationnal(120,1)*R_2; R_11 = R_2+R_11; R_2 = R_4*R_40; R_2 = Rationnal(24,1)*R_2;
                R_2 = R_12+R_2; R_12 = R_5*R_40; R_12 = Rationnal(6,1)*R_12; R_12 = R_14+R_12;
                R_14 = R_6*R_40; R_14 = Rationnal(2,1)*R_14; R_14 = R_18+R_14; R_40 = R_7*R_40;
                R_40 = R_41+R_40; R_40 = Rationnal(-1,6)*R_40; r[3] = R_40; R_8 = R_9*R_8;
                R_9 = Rationnal(-10,1)*R_8; R_9 = R_58+R_9; R_9 = R_6*R_9; R_9 = Rationnal(2,1)*R_9;
                R_9 = R_34+R_9; R_9 = R_4*R_9; R_9 = Rationnal(24,1)*R_9; R_34 = Rationnal(6,1)*R_8;
                R_59 = R_34+R_59; R_59 = R_7*R_59; R_89 = R_59+R_89; R_89 = R_7*R_89;
                R_19 = R_89+R_19; R_19 = R_5*R_19; R_19 = Rationnal(6,1)*R_19; R_89 = Rationnal(8,1)*R_8;
                R_60 = R_89+R_60; R_60 = R_7*R_60; R_90 = R_60+R_90; R_90 = R_6*R_90;
                R_90 = Rationnal(2,1)*R_90; R_90 = R_15+R_90; R_15 = Rationnal(15,1)*R_8; R_62 = R_15+R_62;
                R_62 = R_7*R_62; R_62 = R_93+R_62; R_62 = R_7*R_62; R_62 = R_21+R_62;
                R_46 = R_62+R_46; R_46 = R_6*R_46; R_46 = Rationnal(2,1)*R_46; R_62 = Rationnal(5,1)*R_8;
                R_63 = R_62+R_63; R_63 = R_6*R_63; R_63 = Rationnal(2,1)*R_63; R_63 = R_84+R_63;
                R_63 = R_7*R_63; R_16 = R_63+R_16; R_63 = Rationnal(-5,1)*R_8; R_63 = R_64+R_63;
                R_64 = R_4*R_63; R_64 = Rationnal(24,1)*R_64; R_63 = R_7*R_63; R_95 = R_63+R_95;
                R_95 = R_3*R_95; R_95 = Rationnal(120,1)*R_95; R_95 = R_0+R_95; R_0 = Rationnal(10,1)*R_8;
                R_65 = R_0+R_65; R_65 = R_7*R_65; R_94 = R_65+R_94; R_94 = R_6*R_94;
                R_94 = Rationnal(2,1)*R_94; R_94 = R_16+R_94; R_47 = R_94+R_47; R_47 = R_7*R_47;
                R_75 = R_47+R_75; R_46 = R_75+R_46; R_30 = R_46+R_30; R_64 = R_30+R_64;
                R_64 = R_6*R_64; R_64 = Rationnal(2,1)*R_64; R_96 = R_65+R_96; R_96 = R_7*R_96;
                R_22 = R_96+R_22; R_49 = R_22+R_49; R_49 = R_7*R_49; R_78 = R_49+R_78;
                R_49 = Rationnal(-60,1)*R_8; R_66 = R_49+R_66; R_66 = R_5*R_66; R_78 = R_66+R_78;
                R_31 = R_78+R_31; R_31 = R_5*R_31; R_31 = Rationnal(6,1)*R_31; R_95 = R_31+R_95;
                R_64 = R_95+R_64; R_95 = Rationnal(2,1)*R_8; R_67 = R_95+R_67; R_67 = R_7*R_67;
                R_67 = R_97+R_67; R_67 = R_6*R_67; R_67 = Rationnal(2,1)*R_67; R_67 = R_54+R_67;
                R_67 = R_17+R_67; R_17 = Rationnal(3,1)*R_8; R_68 = R_17+R_68; R_17 = R_6*R_68;
                R_17 = Rationnal(2,1)*R_17; R_82 = R_17+R_82; R_82 = R_6*R_82; R_82 = Rationnal(2,1)*R_82;
                R_68 = R_7*R_68; R_91 = R_68+R_91; R_91 = R_5*R_91; R_91 = Rationnal(6,1)*R_91;
                R_91 = R_26+R_91; R_91 = R_82+R_91; R_91 = R_13+R_91; R_68 = R_98+R_68;
                R_68 = R_7*R_68; R_68 = R_23+R_68; R_68 = R_39+R_68; R_68 = R_6*R_68;
                R_68 = Rationnal(2,1)*R_68; R_39 = Rationnal(4,1)*R_8; R_69 = R_39+R_69; R_39 = R_6*R_69;
                R_39 = Rationnal(2,1)*R_39; R_105 = R_39+R_105; R_105 = R_7*R_105; R_105 = R_90+R_105;
                R_105 = R_43+R_105; R_105 = R_6*R_105; R_105 = Rationnal(2,1)*R_105; R_69 = R_7*R_69;
                R_69 = R_99+R_69; R_69 = R_7*R_69; R_69 = R_24+R_69; R_52 = R_69+R_52;
                R_52 = R_7*R_52; R_69 = Rationnal(-4,1)*R_8; R_70 = R_69+R_70; R_69 = R_5*R_70;
                R_69 = Rationnal(6,1)*R_69; R_69 = R_79+R_69; R_69 = R_52+R_69; R_50 = R_69+R_50;
                R_50 = R_6*R_50; R_50 = Rationnal(2,1)*R_50; R_70 = R_7*R_70; R_70 = R_100+R_70;
                R_70 = R_4*R_70; R_70 = Rationnal(24,1)*R_70; R_71 = R_8+R_71; R_100 = R_5*R_71;
                R_100 = Rationnal(6,1)*R_100; R_83 = R_100+R_83; R_83 = R_77+R_83; R_77 = R_6*R_71;
                R_77 = Rationnal(2,1)*R_77; R_85 = R_77+R_85; R_81 = R_85+R_81; R_81 = R_7*R_81;
                R_81 = R_67+R_81; R_81 = R_7*R_81; R_71 = R_7*R_71; R_71 = R_101+R_71;
                R_71 = R_7*R_71; R_71 = R_25+R_71; R_71 = R_7*R_71; R_25 = Rationnal(-3,1)*R_8;
                R_72 = R_25+R_72; R_25 = R_7*R_72; R_25 = R_102+R_25; R_25 = R_5*R_25;
                R_25 = Rationnal(6,1)*R_25; R_72 = R_6*R_72; R_102 = Rationnal(4,1)*R_72; R_86 = R_102+R_86;
                R_53 = R_86+R_53; R_53 = R_5*R_53; R_53 = Rationnal(6,1)*R_53; R_11 = R_53+R_11;
                R_50 = R_11+R_50; R_50 = R_70+R_50; R_72 = Rationnal(2,1)*R_72; R_72 = R_87+R_72;
                R_72 = R_6*R_72; R_72 = Rationnal(2,1)*R_72; R_2 = R_72+R_2; R_25 = R_2+R_25;
                R_2 = Rationnal(-2,1)*R_8; R_73 = R_2+R_73; R_73 = R_7*R_73; R_73 = R_103+R_73;
                R_73 = R_6*R_73; R_73 = Rationnal(2,1)*R_73; R_103 = Rationnal(-1,1)*R_8; R_61 = R_103+R_61;
                R_61 = R_7*R_61; R_61 = R_92+R_61; R_61 = R_7*R_61; R_61 = R_20+R_61;
                R_61 = R_7*R_61; R_61 = R_83+R_61; R_61 = R_7*R_61; R_91 = R_61+R_91;
                R_91 = R_7*R_91; R_74 = R_103+R_74; R_3 = R_3*R_74; R_3 = Rationnal(120,1)*R_3;
                R_36 = R_3+R_36; R_36 = R_19+R_36; R_36 = R_105+R_36; R_36 = R_1+R_36;
                R_36 = R_91+R_36; R_36 = R_7*R_36; R_36 = R_64+R_36; R_9 = R_36+R_9;
                R_9 = Rationnal(-1,40320)*R_9; r[8] = R_9; R_4 = R_4*R_74; R_4 = Rationnal(24,1)*R_4;
                R_51 = R_4+R_51; R_51 = R_68+R_51; R_51 = R_10+R_51; R_81 = R_51+R_81;
                R_81 = R_7*R_81; R_81 = R_50+R_81; R_81 = Rationnal(-1,5040)*R_81; r[7] = R_81;
                R_5 = R_5*R_74; R_5 = Rationnal(6,1)*R_5; R_35 = R_5+R_35; R_35 = R_76+R_35;
                R_71 = R_35+R_71; R_71 = R_7*R_71; R_71 = R_25+R_71; R_71 = Rationnal(-1,720)*R_71;
                r[6] = R_71; R_6 = R_6*R_74; R_6 = Rationnal(2,1)*R_6; R_80 = R_6+R_80;
                R_55 = R_80+R_55; R_55 = R_7*R_55; R_55 = R_12+R_55; R_73 = R_55+R_73;
                R_73 = Rationnal(-1,120)*R_73; r[5] = R_73; R_74 = R_7*R_74; R_74 = R_88+R_74;
                R_74 = R_7*R_74; R_74 = R_14+R_74; R_74 = Rationnal(-1,24)*R_74; r[4] = R_74;
                R_57 = R_8+R_57; R_57 = Rationnal(-1,2)*R_57; r[2] = R_57; 
                break;
            }
            #endif // WANT_SERIES_789
            default:
                th->add_error( "TODO : PolynomialExpansion for type "+std::string(Nstring(a->type))+" and order > 9 -> see file 'ex.cpp'.", tok );
        }
    }
}

