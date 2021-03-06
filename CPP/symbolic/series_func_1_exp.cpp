#include "polynomial_expansion.h"
 
void PolynomialExpansion::exp_rec_exp( Op *a ) {
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
                Ex R_0 = r_0[0]; R_0 = exp(R_0); r[0] = R_0; 
                break;
            }
            case 2: {
                Ex R_0 = r_0[1]; Ex R_1 = r_0[0]; R_1 = exp(R_1); r[0] = R_1;
                R_0 = R_1*R_0; r[1] = R_0; 
                break;
            }
            case 3: {
                Ex R_0 = r_0[2]; R_0 = Rationnal(2,1)*R_0; Ex R_1 = r_0[1]; Ex R_2 = pow(R_1,2);
                R_2 = R_0+R_2; R_0 = r_0[0]; R_0 = exp(R_0); r[0] = R_0;
                R_2 = R_0*R_2; R_2 = Rationnal(1,2)*R_2; r[2] = R_2; R_1 = R_0*R_1;
                r[1] = R_1; 
                break;
            }
            case 4: {
                Ex R_0 = r_0[3]; Ex R_1 = r_0[2]; Ex R_2 = Rationnal(2,1)*R_1; Ex R_3 = r_0[1];
                R_1 = R_1*R_3; R_1 = R_0+R_1; R_1 = Rationnal(6,1)*R_1; R_0 = pow(R_3,3);
                R_1 = R_0+R_1; R_0 = pow(R_3,2); R_0 = R_2+R_0; R_2 = r_0[0];
                R_2 = exp(R_2); r[0] = R_2; R_1 = R_2*R_1; R_1 = Rationnal(1,6)*R_1;
                r[3] = R_1; R_0 = R_2*R_0; R_0 = Rationnal(1,2)*R_0; r[2] = R_0;
                R_3 = R_2*R_3; r[1] = R_3; 
                break;
            }
            case 5: {
                Ex R_0 = r_0[4]; R_0 = Rationnal(24,1)*R_0; Ex R_1 = r_0[3]; Ex R_2 = Rationnal(6,1)*R_1;
                Ex R_3 = Rationnal(12,1)*R_1; Ex R_4 = r_0[2]; Ex R_5 = Rationnal(6,1)*R_4; Ex R_6 = Rationnal(2,1)*R_4;
                Ex R_7 = r_0[1]; Ex R_8 = R_4*R_7; Ex R_9 = Rationnal(2,1)*R_8; R_9 = R_2+R_9;
                R_9 = R_7*R_9; R_8 = R_1+R_8; R_8 = Rationnal(6,1)*R_8; R_1 = pow(R_7,3);
                R_8 = R_1+R_8; R_1 = pow(R_7,2); R_5 = R_1+R_5; R_4 = R_4*R_5;
                R_4 = R_9+R_4; R_4 = Rationnal(2,1)*R_4; R_5 = R_7*R_5; R_5 = R_3+R_5;
                R_5 = R_7*R_5; R_5 = R_0+R_5; R_4 = R_5+R_4; R_1 = R_6+R_1;
                R_6 = r_0[0]; R_6 = exp(R_6); r[0] = R_6; R_4 = R_6*R_4;
                R_4 = Rationnal(1,24)*R_4; r[4] = R_4; R_8 = R_6*R_8; R_8 = Rationnal(1,6)*R_8;
                r[3] = R_8; R_1 = R_6*R_1; R_1 = Rationnal(1,2)*R_1; r[2] = R_1;
                R_7 = R_6*R_7; r[1] = R_7; 
                break;
            }
            case 6: {
                Ex R_0 = r_0[5]; R_0 = Rationnal(120,1)*R_0; Ex R_1 = r_0[4]; Ex R_2 = Rationnal(120,1)*R_1;
                R_1 = Rationnal(24,1)*R_1; Ex R_3 = r_0[3]; Ex R_4 = Rationnal(84,1)*R_3; Ex R_5 = Rationnal(42,1)*R_3;
                Ex R_6 = Rationnal(6,1)*R_3; Ex R_7 = Rationnal(12,1)*R_3; Ex R_8 = r_0[2]; Ex R_9 = Rationnal(20,1)*R_8;
                Ex R_10 = Rationnal(18,1)*R_8; Ex R_11 = pow(R_8,2); R_11 = Rationnal(40,1)*R_11; R_2 = R_11+R_2;
                R_11 = Rationnal(6,1)*R_8; Ex R_12 = Rationnal(2,1)*R_8; Ex R_13 = r_0[1]; Ex R_14 = R_8*R_13;
                Ex R_15 = Rationnal(2,1)*R_14; R_15 = R_6+R_15; R_15 = R_13*R_15; R_14 = R_3+R_14;
                R_14 = Rationnal(6,1)*R_14; R_6 = pow(R_13,3); R_14 = R_6+R_14; R_6 = pow(R_13,2);
                Ex R_16 = Rationnal(2,1)*R_6; R_16 = R_9+R_16; R_16 = R_13*R_16; R_16 = R_4+R_16;
                R_16 = R_8*R_16; R_10 = R_6+R_10; R_10 = R_13*R_10; R_10 = R_5+R_10;
                R_10 = R_13*R_10; R_10 = R_2+R_10; R_10 = R_13*R_10; R_11 = R_6+R_11;
                R_8 = R_8*R_11; R_8 = R_15+R_8; R_8 = Rationnal(2,1)*R_8; R_11 = R_13*R_11;
                R_11 = R_7+R_11; R_11 = R_13*R_11; R_11 = R_1+R_11; R_8 = R_11+R_8;
                R_6 = R_12+R_6; R_3 = R_3*R_6; R_3 = Rationnal(18,1)*R_3; R_3 = R_0+R_3;
                R_10 = R_3+R_10; R_16 = R_10+R_16; R_10 = r_0[0]; R_10 = exp(R_10);
                r[0] = R_10; R_16 = R_10*R_16; R_16 = Rationnal(1,120)*R_16; r[5] = R_16;
                R_8 = R_10*R_8; R_8 = Rationnal(1,24)*R_8; r[4] = R_8; R_14 = R_10*R_14;
                R_14 = Rationnal(1,6)*R_14; r[3] = R_14; R_6 = R_10*R_6; R_6 = Rationnal(1,2)*R_6;
                r[2] = R_6; R_13 = R_10*R_13; r[1] = R_13; 
                break;
            }
            #ifdef WANT_SERIES_789
            case 7: {
                Ex R_0 = r_0[6]; R_0 = Rationnal(720,1)*R_0; Ex R_1 = r_0[5]; Ex R_2 = Rationnal(240,1)*R_1;
                R_1 = Rationnal(120,1)*R_1; Ex R_3 = r_0[4]; Ex R_4 = Rationnal(216,1)*R_3; Ex R_5 = Rationnal(96,1)*R_3;
                Ex R_6 = Rationnal(120,1)*R_3; Ex R_7 = Rationnal(24,1)*R_3; Ex R_8 = r_0[3]; Ex R_9 = Rationnal(30,1)*R_8;
                Ex R_10 = Rationnal(60,1)*R_8; Ex R_11 = Rationnal(18,1)*R_8; Ex R_12 = Rationnal(54,1)*R_8; Ex R_13 = Rationnal(24,1)*R_8;
                Ex R_14 = Rationnal(84,1)*R_8; Ex R_15 = Rationnal(42,1)*R_8; Ex R_16 = Rationnal(6,1)*R_8; Ex R_17 = Rationnal(12,1)*R_8;
                Ex R_18 = r_0[2]; Ex R_19 = Rationnal(10,1)*R_18; Ex R_20 = Rationnal(36,1)*R_18; Ex R_21 = Rationnal(14,1)*R_18;
                Ex R_22 = Rationnal(4,1)*R_18; Ex R_23 = Rationnal(20,1)*R_18; Ex R_24 = Rationnal(18,1)*R_18; Ex R_25 = pow(R_18,2);
                Ex R_26 = Rationnal(8,1)*R_25; Ex R_27 = Rationnal(4,1)*R_25; Ex R_28 = Rationnal(12,1)*R_25; R_5 = R_5+R_28;
                R_25 = Rationnal(40,1)*R_25; R_6 = R_25+R_6; R_25 = Rationnal(6,1)*R_18; Ex R_29 = Rationnal(2,1)*R_18;
                Ex R_30 = r_0[1]; Ex R_31 = R_8*R_30; R_31 = Rationnal(6,1)*R_31; R_31 = R_7+R_31;
                R_31 = R_27+R_31; R_31 = R_30*R_31; R_27 = R_18*R_30; Ex R_32 = Rationnal(4,1)*R_27;
                R_12 = R_32+R_12; R_12 = R_18*R_12; R_12 = Rationnal(2,1)*R_12; R_32 = Rationnal(6,1)*R_27;
                R_32 = R_13+R_32; R_32 = R_8*R_32; R_32 = Rationnal(6,1)*R_32; R_13 = Rationnal(2,1)*R_27;
                R_9 = R_13+R_9; R_9 = R_18*R_9; R_9 = Rationnal(2,1)*R_9; R_9 = R_31+R_9;
                R_9 = R_30*R_9; R_11 = R_13+R_11; R_11 = R_30*R_11; R_11 = R_5+R_11;
                R_11 = R_30*R_11; R_13 = R_16+R_13; R_13 = R_30*R_13; R_26 = R_13+R_26;
                R_28 = R_13+R_28; R_28 = Rationnal(4,1)*R_28; R_27 = R_8+R_27; R_27 = Rationnal(6,1)*R_27;
                R_5 = pow(R_30,3); R_27 = R_5+R_27; R_5 = pow(R_30,2); R_19 = R_5+R_19;
                R_19 = R_5*R_19; R_19 = R_28+R_19; R_19 = R_30*R_19; R_19 = R_2+R_19;
                R_19 = R_30*R_19; R_21 = R_5+R_21; R_21 = R_30*R_21; R_21 = R_10+R_21;
                R_21 = R_30*R_21; R_21 = R_4+R_21; R_22 = R_5+R_22; R_22 = R_3*R_22;
                R_22 = Rationnal(24,1)*R_22; R_4 = Rationnal(2,1)*R_5; R_20 = R_4+R_20; R_20 = R_18*R_20;
                R_20 = R_21+R_20; R_20 = R_18*R_20; R_20 = R_22+R_20; R_4 = R_23+R_4;
                R_4 = R_30*R_4; R_4 = R_14+R_4; R_4 = R_18*R_4; R_24 = R_5+R_24;
                R_24 = R_30*R_24; R_24 = R_15+R_24; R_24 = R_30*R_24; R_24 = R_6+R_24;
                R_24 = R_30*R_24; R_25 = R_5+R_25; R_18 = R_18*R_25; R_18 = R_13+R_18;
                R_18 = Rationnal(2,1)*R_18; R_25 = R_30*R_25; R_16 = R_16+R_25; R_16 = R_8*R_16;
                R_16 = Rationnal(12,1)*R_16; R_16 = R_0+R_16; R_25 = R_17+R_25; R_25 = R_30*R_25;
                R_25 = R_7+R_25; R_18 = R_25+R_18; R_5 = R_29+R_5; R_3 = R_3*R_5;
                R_3 = Rationnal(48,1)*R_3; R_16 = R_3+R_16; R_26 = R_5*R_26; R_26 = R_9+R_26;
                R_26 = Rationnal(3,1)*R_26; R_16 = R_26+R_16; R_16 = R_19+R_16; R_8 = R_8*R_5;
                R_19 = Rationnal(6,1)*R_8; R_12 = R_19+R_12; R_12 = R_2+R_12; R_11 = R_12+R_11;
                R_11 = R_30*R_11; R_20 = R_11+R_20; R_20 = R_32+R_20; R_20 = Rationnal(2,1)*R_20;
                R_16 = R_20+R_16; R_8 = Rationnal(18,1)*R_8; R_8 = R_1+R_8; R_24 = R_8+R_24;
                R_4 = R_24+R_4; R_24 = r_0[0]; R_24 = exp(R_24); r[0] = R_24;
                R_16 = R_24*R_16; R_16 = Rationnal(1,720)*R_16; r[6] = R_16; R_4 = R_24*R_4;
                R_4 = Rationnal(1,120)*R_4; r[5] = R_4; R_18 = R_24*R_18; R_18 = Rationnal(1,24)*R_18;
                r[4] = R_18; R_27 = R_24*R_27; R_27 = Rationnal(1,6)*R_27; r[3] = R_27;
                R_5 = R_24*R_5; R_5 = Rationnal(1,2)*R_5; r[2] = R_5; R_30 = R_24*R_30;
                r[1] = R_30; 
                break;
            }
            case 8: {
                Ex R_0 = r_0[7]; R_0 = Rationnal(5040,1)*R_0; Ex R_1 = r_0[6]; Ex R_2 = Rationnal(5040,1)*R_1;
                R_1 = Rationnal(720,1)*R_1; Ex R_3 = r_0[5]; Ex R_4 = Rationnal(3120,1)*R_3; Ex R_5 = Rationnal(1440,1)*R_3;
                Ex R_6 = Rationnal(240,1)*R_3; Ex R_7 = Rationnal(120,1)*R_3; Ex R_8 = r_0[4]; Ex R_9 = Rationnal(912,1)*R_8;
                Ex R_10 = Rationnal(48,1)*R_8; Ex R_11 = Rationnal(144,1)*R_8; Ex R_12 = Rationnal(2736,1)*R_8; Ex R_13 = Rationnal(336,1)*R_8;
                Ex R_14 = Rationnal(384,1)*R_8; Ex R_15 = Rationnal(216,1)*R_8; Ex R_16 = Rationnal(96,1)*R_8; Ex R_17 = Rationnal(120,1)*R_8;
                Ex R_18 = Rationnal(24,1)*R_8; Ex R_19 = r_0[3]; Ex R_20 = Rationnal(1032,1)*R_19; Ex R_21 = Rationnal(2304,1)*R_19;
                Ex R_22 = Rationnal(864,1)*R_19; Ex R_23 = Rationnal(192,1)*R_19; Ex R_24 = Rationnal(78,1)*R_19; Ex R_25 = Rationnal(48,1)*R_19;
                Ex R_26 = Rationnal(1008,1)*R_19; Ex R_27 = Rationnal(30,1)*R_19; Ex R_28 = Rationnal(60,1)*R_19; Ex R_29 = Rationnal(18,1)*R_19;
                Ex R_30 = Rationnal(54,1)*R_19; Ex R_31 = Rationnal(24,1)*R_19; Ex R_32 = Rationnal(84,1)*R_19; Ex R_33 = Rationnal(42,1)*R_19;
                Ex R_34 = Rationnal(6,1)*R_19; Ex R_35 = Rationnal(12,1)*R_19; Ex R_36 = r_0[2]; Ex R_37 = Rationnal(120,1)*R_36;
                Ex R_38 = Rationnal(16,1)*R_36; Ex R_39 = Rationnal(104,1)*R_36; Ex R_40 = Rationnal(1056,1)*R_36; Ex R_41 = Rationnal(504,1)*R_36;
                Ex R_42 = Rationnal(204,1)*R_36; Ex R_43 = Rationnal(26,1)*R_36; Ex R_44 = Rationnal(1920,1)*R_36; Ex R_45 = Rationnal(10,1)*R_36;
                Ex R_46 = Rationnal(36,1)*R_36; Ex R_47 = Rationnal(14,1)*R_36; Ex R_48 = Rationnal(4,1)*R_36; Ex R_49 = Rationnal(20,1)*R_36;
                Ex R_50 = Rationnal(18,1)*R_36; Ex R_51 = pow(R_36,2); Ex R_52 = Rationnal(64,1)*R_51; R_9 = R_52+R_9;
                R_52 = Rationnal(112,1)*R_51; R_13 = R_52+R_13; R_52 = Rationnal(32,1)*R_51; R_52 = R_14+R_52;
                R_14 = Rationnal(8,1)*R_51; Ex R_53 = Rationnal(4,1)*R_51; Ex R_54 = Rationnal(12,1)*R_51; R_16 = R_16+R_54;
                R_51 = Rationnal(40,1)*R_51; R_17 = R_51+R_17; R_51 = Rationnal(6,1)*R_36; Ex R_55 = Rationnal(2,1)*R_36;
                Ex R_56 = r_0[1]; Ex R_57 = R_19*R_56; Ex R_58 = Rationnal(12,1)*R_57; R_58 = R_14+R_58;
                R_58 = R_11+R_58; R_58 = R_36*R_58; R_58 = Rationnal(2,1)*R_58; R_57 = Rationnal(6,1)*R_57;
                R_10 = R_57+R_10; R_10 = R_53+R_10; R_10 = R_56*R_10; R_57 = R_18+R_57;
                R_57 = R_53+R_57; R_57 = R_56*R_57; R_53 = R_36*R_56; R_11 = Rationnal(10,1)*R_53;
                R_23 = R_11+R_23; R_23 = R_36*R_23; R_23 = R_57+R_23; R_23 = Rationnal(4,1)*R_23;
                R_11 = Rationnal(168,1)*R_53; R_26 = R_11+R_26; R_26 = R_19*R_26; R_11 = Rationnal(4,1)*R_53;
                Ex R_59 = R_34+R_11; Ex R_60 = R_11+R_25; R_60 = R_56*R_60; R_52 = R_60+R_52;
                R_52 = R_36*R_52; R_11 = R_11+R_30; R_11 = R_36*R_11; R_11 = Rationnal(2,1)*R_11;
                R_60 = Rationnal(6,1)*R_53; R_60 = R_31+R_60; R_60 = R_19*R_60; R_60 = Rationnal(6,1)*R_60;
                Ex R_61 = Rationnal(2,1)*R_53; R_31 = R_61+R_31; R_31 = R_56*R_31; R_25 = R_61+R_25;
                R_25 = R_36*R_25; R_25 = Rationnal(2,1)*R_25; R_25 = R_7+R_25; R_10 = R_25+R_10;
                R_10 = R_56*R_10; R_27 = R_61+R_27; R_25 = R_19*R_27; R_25 = Rationnal(6,1)*R_25;
                R_27 = R_36*R_27; R_27 = Rationnal(2,1)*R_27; R_27 = R_57+R_27; R_57 = R_56*R_27;
                R_29 = R_61+R_29; R_29 = R_56*R_29; R_29 = R_16+R_29; R_29 = R_56*R_29;
                R_61 = R_34+R_61; R_61 = R_56*R_61; R_14 = R_61+R_14; R_59 = R_14*R_59;
                R_54 = R_61+R_54; R_16 = Rationnal(4,1)*R_54; R_53 = R_19+R_53; R_53 = Rationnal(6,1)*R_53;
                Ex R_62 = pow(R_56,3); R_53 = R_62+R_53; R_62 = pow(R_56,2); Ex R_63 = Rationnal(4,1)*R_62;
                R_39 = R_63+R_39; R_39 = R_56*R_39; R_39 = R_20+R_39; R_39 = R_36*R_39;
                R_20 = Rationnal(96,1)*R_62; R_20 = R_40+R_20; R_20 = R_56*R_20; R_20 = R_21+R_20;
                R_20 = R_8*R_20; R_21 = Rationnal(36,1)*R_62; R_37 = R_21+R_37; R_37 = R_19*R_37;
                R_21 = R_41+R_21; R_21 = R_36*R_21; R_41 = Rationnal(18,1)*R_62; R_41 = R_42+R_41;
                R_41 = R_56*R_41; R_41 = R_22+R_41; R_41 = R_56*R_41; R_41 = R_12+R_41;
                R_21 = R_41+R_21; R_21 = R_19*R_21; R_43 = R_62+R_43; R_43 = R_56*R_43;
                R_43 = R_24+R_43; R_43 = R_56*R_43; R_43 = R_13+R_43; R_43 = R_56*R_43;
                R_13 = Rationnal(720,1)*R_62; R_44 = R_13+R_44; R_44 = R_3*R_44; R_45 = R_62+R_45;
                R_45 = R_62*R_45; R_45 = R_16+R_45; R_45 = R_56*R_45; R_45 = R_6+R_45;
                R_45 = R_56*R_45; R_47 = R_62+R_47; R_47 = R_56*R_47; R_30 = R_30+R_47;
                R_30 = R_56*R_30; R_30 = Rationnal(2,1)*R_30; R_30 = R_9+R_30; R_47 = R_28+R_47;
                R_47 = R_56*R_47; R_47 = R_15+R_47; R_48 = R_62+R_48; R_27 = R_48*R_27;
                R_27 = R_59+R_27; R_54 = R_48*R_54; R_54 = R_52+R_54; R_54 = Rationnal(4,1)*R_54;
                R_48 = R_8*R_48; R_52 = Rationnal(96,1)*R_48; R_2 = R_52+R_2; R_2 = R_54+R_2;
                R_48 = Rationnal(24,1)*R_48; R_54 = Rationnal(2,1)*R_62; R_38 = R_54+R_38; R_38 = R_36*R_38;
                R_31 = R_38+R_31; R_31 = Rationnal(4,1)*R_31; R_31 = R_30+R_31; R_31 = R_56*R_31;
                R_31 = R_4+R_31; R_31 = R_39+R_31; R_37 = R_31+R_37; R_37 = R_36*R_37;
                R_46 = R_54+R_46; R_46 = R_36*R_46; R_46 = R_47+R_46; R_46 = R_36*R_46;
                R_46 = R_48+R_46; R_54 = R_49+R_54; R_54 = R_56*R_54; R_54 = R_32+R_54;
                R_54 = R_36*R_54; R_50 = R_62+R_50; R_50 = R_56*R_50; R_50 = R_33+R_50;
                R_50 = R_56*R_50; R_50 = R_17+R_50; R_50 = R_56*R_50; R_51 = R_62+R_51;
                R_36 = R_36*R_51; R_36 = R_61+R_36; R_36 = Rationnal(2,1)*R_36; R_51 = R_56*R_51;
                R_34 = R_34+R_51; R_34 = R_19*R_34; R_34 = Rationnal(12,1)*R_34; R_34 = R_1+R_34;
                R_51 = R_35+R_51; R_51 = R_56*R_51; R_51 = R_18+R_51; R_36 = R_51+R_36;
                R_62 = R_55+R_62; R_8 = R_8*R_62; R_8 = Rationnal(48,1)*R_8; R_34 = R_8+R_34;
                R_14 = R_62*R_14; R_25 = R_14+R_25; R_25 = R_58+R_25; R_10 = R_25+R_10;
                R_10 = R_56*R_10; R_10 = R_27+R_10; R_10 = Rationnal(3,1)*R_10; R_10 = R_0+R_10;
                R_10 = R_21+R_10; R_14 = R_57+R_14; R_14 = Rationnal(3,1)*R_14; R_34 = R_14+R_34;
                R_34 = R_45+R_34; R_19 = R_19*R_62; R_45 = Rationnal(12,1)*R_19; R_5 = R_45+R_5;
                R_43 = R_5+R_43; R_23 = R_43+R_23; R_23 = R_56*R_23; R_23 = R_2+R_23;
                R_23 = R_26+R_23; R_23 = R_56*R_23; R_10 = R_23+R_10; R_20 = R_10+R_20;
                R_37 = R_20+R_37; R_37 = R_44+R_37; R_44 = Rationnal(6,1)*R_19; R_11 = R_44+R_11;
                R_11 = R_6+R_11; R_29 = R_11+R_29; R_29 = R_56*R_29; R_46 = R_29+R_46;
                R_46 = R_60+R_46; R_46 = Rationnal(2,1)*R_46; R_34 = R_46+R_34; R_19 = Rationnal(18,1)*R_19;
                R_19 = R_7+R_19; R_50 = R_19+R_50; R_54 = R_50+R_54; R_50 = r_0[0];
                R_50 = exp(R_50); r[0] = R_50; R_37 = R_50*R_37; R_37 = Rationnal(1,5040)*R_37;
                r[7] = R_37; R_34 = R_50*R_34; R_34 = Rationnal(1,720)*R_34; r[6] = R_34;
                R_54 = R_50*R_54; R_54 = Rationnal(1,120)*R_54; r[5] = R_54; R_36 = R_50*R_36;
                R_36 = Rationnal(1,24)*R_36; r[4] = R_36; R_53 = R_50*R_53; R_53 = Rationnal(1,6)*R_53;
                r[3] = R_53; R_62 = R_50*R_62; R_62 = Rationnal(1,2)*R_62; r[2] = R_62;
                R_56 = R_50*R_56; r[1] = R_56; 
                break;
            }
            case 9: {
                Ex R_0 = r_0[8]; R_0 = Rationnal(40320,1)*R_0; Ex R_1 = r_0[7]; Ex R_2 = Rationnal(30240,1)*R_1;
                R_1 = Rationnal(5040,1)*R_1; Ex R_3 = r_0[6]; Ex R_4 = Rationnal(9360,1)*R_3; Ex R_5 = Rationnal(3600,1)*R_3;
                Ex R_6 = Rationnal(5760,1)*R_3; Ex R_7 = Rationnal(1440,1)*R_3; Ex R_8 = Rationnal(5040,1)*R_3; Ex R_9 = Rationnal(720,1)*R_3;
                Ex R_10 = r_0[5]; Ex R_11 = Rationnal(1200,1)*R_10; Ex R_12 = Rationnal(3000,1)*R_10; Ex R_13 = Rationnal(840,1)*R_10;
                Ex R_14 = Rationnal(720,1)*R_10; Ex R_15 = Rationnal(480,1)*R_10; Ex R_16 = Rationnal(360,1)*R_10; Ex R_17 = Rationnal(3120,1)*R_10;
                Ex R_18 = Rationnal(1440,1)*R_10; Ex R_19 = Rationnal(240,1)*R_10; Ex R_20 = Rationnal(120,1)*R_10; Ex R_21 = r_0[4];
                Ex R_22 = Rationnal(960,1)*R_21; Ex R_23 = Rationnal(240,1)*R_21; Ex R_24 = Rationnal(480,1)*R_21; Ex R_25 = Rationnal(840,1)*R_21;
                Ex R_26 = Rationnal(72,1)*R_21; Ex R_27 = Rationnal(264,1)*R_21; Ex R_28 = Rationnal(1152,1)*R_21; Ex R_29 = Rationnal(288,1)*R_21;
                Ex R_30 = Rationnal(8640,1)*R_21; Ex R_31 = Rationnal(192,1)*R_21; Ex R_32 = Rationnal(912,1)*R_21; Ex R_33 = Rationnal(48,1)*R_21;
                Ex R_34 = Rationnal(144,1)*R_21; Ex R_35 = Rationnal(2736,1)*R_21; Ex R_36 = Rationnal(336,1)*R_21; Ex R_37 = Rationnal(384,1)*R_21;
                Ex R_38 = Rationnal(216,1)*R_21; Ex R_39 = Rationnal(96,1)*R_21; Ex R_40 = Rationnal(120,1)*R_21; Ex R_41 = Rationnal(24,1)*R_21;
                Ex R_42 = r_0[3]; Ex R_43 = Rationnal(2520,1)*R_42; Ex R_44 = Rationnal(144,1)*R_42; Ex R_45 = Rationnal(96,1)*R_42;
                Ex R_46 = Rationnal(180,1)*R_42; Ex R_47 = Rationnal(360,1)*R_42; Ex R_48 = Rationnal(90,1)*R_42; Ex R_49 = Rationnal(132,1)*R_42;
                Ex R_50 = Rationnal(3168,1)*R_42; Ex R_51 = Rationnal(720,1)*R_42; Ex R_52 = Rationnal(5184,1)*R_42; Ex R_53 = Rationnal(1032,1)*R_42;
                Ex R_54 = Rationnal(2304,1)*R_42; Ex R_55 = Rationnal(864,1)*R_42; Ex R_56 = Rationnal(192,1)*R_42; Ex R_57 = Rationnal(78,1)*R_42;
                Ex R_58 = Rationnal(48,1)*R_42; Ex R_59 = Rationnal(1008,1)*R_42; Ex R_60 = Rationnal(30,1)*R_42; Ex R_61 = Rationnal(60,1)*R_42;
                Ex R_62 = Rationnal(18,1)*R_42; Ex R_63 = Rationnal(54,1)*R_42; Ex R_64 = Rationnal(24,1)*R_42; Ex R_65 = Rationnal(84,1)*R_42;
                Ex R_66 = Rationnal(42,1)*R_42; Ex R_67 = Rationnal(6,1)*R_42; Ex R_68 = Rationnal(12,1)*R_42; Ex R_69 = r_0[2];
                Ex R_70 = Rationnal(252,1)*R_69; Ex R_71 = Rationnal(22,1)*R_69; Ex R_72 = Rationnal(30,1)*R_69; Ex R_73 = Rationnal(1728,1)*R_69;
                Ex R_74 = Rationnal(3840,1)*R_69; Ex R_75 = Rationnal(432,1)*R_69; Ex R_76 = Rationnal(156,1)*R_69; Ex R_77 = Rationnal(3168,1)*R_69;
                Ex R_78 = Rationnal(1104,1)*R_69; Ex R_79 = Rationnal(32,1)*R_69; Ex R_80 = R_69*R_42; R_80 = Rationnal(36,1)*R_80;
                Ex R_81 = Rationnal(15840,1)*R_69; Ex R_82 = Rationnal(120,1)*R_69; Ex R_83 = Rationnal(16,1)*R_69; Ex R_84 = Rationnal(104,1)*R_69;
                Ex R_85 = Rationnal(1056,1)*R_69; Ex R_86 = Rationnal(504,1)*R_69; Ex R_87 = Rationnal(204,1)*R_69; Ex R_88 = Rationnal(26,1)*R_69;
                Ex R_89 = Rationnal(1920,1)*R_69; Ex R_90 = Rationnal(10,1)*R_69; Ex R_91 = Rationnal(36,1)*R_69; Ex R_92 = Rationnal(14,1)*R_69;
                Ex R_93 = Rationnal(4,1)*R_69; Ex R_94 = Rationnal(20,1)*R_69; Ex R_95 = Rationnal(18,1)*R_69; Ex R_96 = pow(R_69,2);
                Ex R_97 = Rationnal(60,1)*R_96; R_22 = R_22+R_97; R_97 = R_25+R_97; R_25 = Rationnal(20,1)*R_96;
                R_23 = R_23+R_25; Ex R_98 = R_41+R_25; Ex R_99 = Rationnal(16,1)*R_96; Ex R_100 = Rationnal(64,1)*R_96;
                R_32 = R_100+R_32; R_100 = Rationnal(112,1)*R_96; R_100 = R_100+R_36; Ex R_101 = Rationnal(32,1)*R_96;
                R_101 = R_37+R_101; Ex R_102 = Rationnal(8,1)*R_96; Ex R_103 = Rationnal(4,1)*R_96; Ex R_104 = Rationnal(12,1)*R_96;
                Ex R_105 = R_39+R_104; R_96 = Rationnal(40,1)*R_96; R_24 = R_96+R_24; R_40 = R_96+R_40;
                R_96 = Rationnal(6,1)*R_69; Ex R_106 = Rationnal(2,1)*R_69; Ex R_107 = r_0[1]; Ex R_108 = R_21*R_107;
                Ex R_109 = Rationnal(48,1)*R_108; R_108 = Rationnal(24,1)*R_108; Ex R_110 = R_20+R_108; R_80 = R_110+R_80;
                R_80 = R_107*R_80; R_110 = R_42*R_107; Ex R_111 = Rationnal(24,1)*R_110; R_111 = R_99+R_111;
                R_111 = R_37+R_111; R_111 = R_69*R_111; R_111 = Rationnal(2,1)*R_111; R_37 = Rationnal(18,1)*R_110;
                R_37 = R_104+R_37; R_37 = R_27+R_37; R_37 = R_42*R_37; R_37 = Rationnal(6,1)*R_37;
                R_27 = Rationnal(12,1)*R_110; R_27 = R_102+R_27; Ex R_112 = R_41+R_27; R_31 = R_27+R_31;
                R_31 = R_69*R_31; R_31 = Rationnal(2,1)*R_31; R_27 = R_34+R_27; R_27 = R_69*R_27;
                R_27 = Rationnal(2,1)*R_27; R_110 = Rationnal(6,1)*R_110; R_34 = R_103+R_110; R_39 = R_39+R_34;
                R_39 = R_107*R_39; R_34 = R_26+R_34; R_34 = R_107*R_34; R_34 = R_16+R_34;
                R_26 = R_110+R_33; R_26 = R_103+R_26; R_26 = R_107*R_26; R_110 = R_41+R_110;
                R_110 = R_103+R_110; R_110 = R_107*R_110; R_103 = R_69*R_107; Ex R_113 = Rationnal(12,1)*R_103;
                R_113 = R_46+R_113; R_113 = R_42*R_113; R_113 = Rationnal(6,1)*R_113; Ex R_114 = Rationnal(16,1)*R_103;
                R_114 = R_47+R_114; R_114 = R_69*R_114; R_114 = Rationnal(2,1)*R_114; R_47 = Rationnal(8,1)*R_103;
                Ex R_115 = R_47+R_48; R_115 = R_21*R_115; R_115 = Rationnal(24,1)*R_115; R_46 = R_47+R_46;
                R_46 = R_69*R_46; R_46 = Rationnal(2,1)*R_46; Ex R_116 = R_18+R_46; R_47 = R_62+R_47;
                Ex R_117 = Rationnal(10,1)*R_103; R_56 = R_117+R_56; R_56 = R_69*R_56; R_56 = R_110+R_56;
                R_56 = Rationnal(4,1)*R_56; R_117 = Rationnal(168,1)*R_103; R_59 = R_117+R_59; R_59 = R_42*R_59;
                R_117 = Rationnal(4,1)*R_103; Ex R_118 = R_117+R_61; R_118 = R_107*R_118; R_118 = R_24+R_118;
                R_118 = R_107*R_118; R_118 = R_12+R_118; R_114 = R_118+R_114; R_114 = R_69*R_114;
                R_114 = Rationnal(2,1)*R_114; R_114 = R_1+R_114; R_118 = R_117+R_57; R_118 = R_42*R_118;
                R_118 = Rationnal(6,1)*R_118; R_49 = R_117+R_49; R_49 = R_69*R_49; R_49 = R_34+R_49;
                R_49 = R_107*R_49; R_34 = R_67+R_117; R_12 = R_42*R_34; R_24 = Rationnal(36,1)*R_12;
                R_12 = Rationnal(18,1)*R_12; Ex R_119 = R_117+R_58; R_119 = R_107*R_119; R_101 = R_119+R_101;
                R_101 = R_69*R_101; R_117 = R_117+R_63; R_117 = R_69*R_117; R_117 = Rationnal(2,1)*R_117;
                R_119 = Rationnal(6,1)*R_103; R_48 = R_119+R_48; R_48 = R_107*R_48; R_22 = R_22+R_48;
                R_22 = R_69*R_22; Ex R_120 = Rationnal(4,1)*R_22; R_22 = Rationnal(2,1)*R_22; R_113 = R_22+R_113;
                R_113 = R_5+R_113; R_97 = R_48+R_97; R_97 = R_42*R_97; R_97 = Rationnal(6,1)*R_97;
                R_114 = R_97+R_114; R_119 = R_64+R_119; R_119 = R_42*R_119; R_119 = Rationnal(6,1)*R_119;
                R_97 = Rationnal(2,1)*R_103; R_48 = R_97+R_65; R_48 = R_69*R_48; R_48 = Rationnal(2,1)*R_48;
                R_48 = R_13+R_48; R_39 = R_48+R_39; R_39 = R_69*R_39; R_48 = R_97+R_68;
                R_48 = R_107*R_48; R_98 = R_48+R_98; R_98 = R_107*R_98; R_48 = R_97+R_63;
                R_48 = R_69*R_48; R_48 = Rationnal(2,1)*R_48; R_98 = R_48+R_98; R_98 = R_107*R_98;
                R_48 = R_97+R_66; R_13 = R_42*R_48; R_13 = Rationnal(6,1)*R_13; R_13 = R_80+R_13;
                R_31 = R_13+R_31; R_31 = R_107*R_31; R_48 = R_69*R_48; R_48 = Rationnal(2,1)*R_48;
                R_48 = R_110+R_48; R_13 = R_107*R_48; R_64 = R_97+R_64; R_64 = R_107*R_64;
                R_58 = R_97+R_58; R_58 = R_69*R_58; R_58 = Rationnal(2,1)*R_58; R_58 = R_20+R_58;
                R_26 = R_58+R_26; R_26 = R_107*R_26; R_60 = R_97+R_60; R_58 = R_107*R_60;
                R_23 = R_58+R_23; R_58 = R_21*R_23; R_58 = Rationnal(24,1)*R_58; R_23 = R_107*R_23;
                R_116 = R_116+R_23; R_116 = R_42*R_116; R_116 = Rationnal(6,1)*R_116; R_23 = R_11+R_23;
                R_23 = R_46+R_23; R_23 = R_107*R_23; R_113 = R_23+R_113; R_113 = R_107*R_113;
                R_113 = R_114+R_113; R_115 = R_113+R_115; R_115 = R_107*R_115; R_113 = R_21*R_60;
                R_113 = Rationnal(24,1)*R_113; R_114 = R_42*R_60; R_114 = Rationnal(6,1)*R_114; R_80 = R_114+R_80;
                R_80 = R_27+R_80; R_60 = R_69*R_60; R_60 = Rationnal(2,1)*R_60; R_60 = R_110+R_60;
                R_47 = R_60*R_47; R_110 = R_107*R_60; R_62 = R_97+R_62; R_46 = R_10*R_62;
                R_46 = Rationnal(120,1)*R_46; R_46 = R_116+R_46; R_46 = R_58+R_46; R_46 = R_115+R_46;
                R_62 = R_107*R_62; R_62 = R_105+R_62; R_62 = R_107*R_62; R_97 = R_67+R_97;
                R_105 = R_107*R_97; R_25 = R_105+R_25; R_25 = Rationnal(6,1)*R_25; R_25 = R_33+R_25;
                R_99 = R_105+R_99; R_99 = R_69*R_99; R_99 = Rationnal(6,1)*R_99; R_98 = R_99+R_98;
                R_98 = Rationnal(5,1)*R_98; R_6 = R_98+R_6; R_6 = R_24+R_6; R_102 = R_105+R_102;
                R_112 = R_102*R_112; R_34 = R_102*R_34; R_39 = R_34+R_39; R_39 = Rationnal(2,1)*R_39;
                R_104 = R_105+R_104; R_97 = R_97*R_104; R_97 = Rationnal(2,1)*R_97; R_31 = R_97+R_31;
                R_97 = Rationnal(4,1)*R_104; R_103 = R_42+R_103; R_103 = Rationnal(6,1)*R_103; R_24 = pow(R_107,3);
                R_103 = R_24+R_103; R_24 = pow(R_107,2); R_98 = Rationnal(6,1)*R_24; R_98 = R_70+R_98;
                R_98 = R_107*R_98; R_98 = R_43+R_98; R_98 = R_42*R_98; R_43 = Rationnal(1200,1)*R_24;
                R_74 = R_43+R_74; R_74 = R_10*R_74; R_43 = Rationnal(24,1)*R_24; R_43 = R_75+R_43;
                R_43 = R_69*R_43; R_75 = Rationnal(3,1)*R_24; R_71 = R_71+R_75; R_71 = R_42*R_71;
                R_71 = Rationnal(6,1)*R_71; R_72 = R_72+R_75; R_72 = R_69*R_72; R_72 = Rationnal(2,1)*R_72;
                R_36 = R_36+R_72; R_75 = R_88+R_75; R_75 = R_69*R_75; R_75 = Rationnal(2,1)*R_75;
                R_70 = Rationnal(336,1)*R_24; R_70 = R_77+R_70; R_70 = R_69*R_70; R_77 = Rationnal(72,1)*R_24;
                R_77 = R_78+R_77; R_77 = R_107*R_77; R_52 = R_77+R_52; R_52 = R_107*R_52;
                R_30 = R_52+R_30; R_70 = R_30+R_70; R_70 = R_21*R_70; R_30 = Rationnal(5040,1)*R_24;
                R_30 = R_81+R_30; R_30 = R_3*R_30; R_3 = Rationnal(4,1)*R_24; R_84 = R_3+R_84;
                R_84 = R_107*R_84; R_84 = R_53+R_84; R_84 = R_69*R_84; R_53 = Rationnal(96,1)*R_24;
                R_73 = R_53+R_73; R_73 = R_107*R_73; R_73 = R_50+R_73; R_73 = R_21*R_73;
                R_53 = R_85+R_53; R_53 = R_107*R_53; R_53 = R_54+R_53; R_53 = R_21*R_53;
                R_54 = Rationnal(36,1)*R_24; R_82 = R_54+R_82; R_82 = R_42*R_82; R_54 = R_86+R_54;
                R_54 = R_69*R_54; R_86 = Rationnal(18,1)*R_24; R_76 = R_86+R_76; R_76 = R_42*R_76;
                R_86 = R_87+R_86; R_86 = R_107*R_86; R_86 = R_55+R_86; R_86 = R_107*R_86;
                R_86 = R_35+R_86; R_54 = R_86+R_54; R_54 = R_42*R_54; R_88 = R_24+R_88;
                R_88 = R_107*R_88; R_88 = R_57+R_88; R_88 = R_107*R_88; R_88 = R_100+R_88;
                R_88 = R_107*R_88; R_100 = Rationnal(720,1)*R_24; R_89 = R_100+R_89; R_89 = R_10*R_89;
                R_90 = R_24+R_90; R_100 = R_107*R_90; R_100 = Rationnal(12,1)*R_100; R_100 = R_51+R_100;
                R_100 = R_107*R_100; R_100 = R_28+R_100; R_43 = R_100+R_43; R_43 = R_42*R_43;
                R_90 = R_24*R_90; R_90 = R_97+R_90; R_90 = R_107*R_90; R_90 = R_19+R_90;
                R_90 = R_107*R_90; R_92 = R_24+R_92; R_97 = R_24*R_92; R_97 = R_25+R_97;
                R_97 = R_107*R_97; R_92 = R_107*R_92; R_25 = R_66+R_92; R_25 = R_69*R_25;
                R_25 = Rationnal(4,1)*R_25; R_25 = R_15+R_25; R_15 = R_63+R_92; R_15 = R_107*R_15;
                R_15 = Rationnal(2,1)*R_15; R_15 = R_32+R_15; R_92 = R_61+R_92; R_92 = R_107*R_92;
                R_92 = R_38+R_92; R_93 = R_24+R_93; R_80 = R_93*R_80; R_80 = R_112+R_80;
                R_112 = R_93*R_60; R_112 = R_34+R_112; R_104 = R_93*R_104; R_13 = R_13+R_104;
                R_104 = R_101+R_104; R_104 = Rationnal(4,1)*R_104; R_93 = R_21*R_93; R_101 = Rationnal(144,1)*R_93;
                R_6 = R_101+R_6; R_101 = Rationnal(72,1)*R_93; R_101 = R_12+R_101; R_12 = Rationnal(96,1)*R_93;
                R_8 = R_12+R_8; R_8 = R_104+R_8; R_93 = Rationnal(24,1)*R_93; R_104 = Rationnal(2,1)*R_24;
                R_12 = R_96+R_104; R_12 = R_60*R_12; R_79 = R_104+R_79; R_79 = R_107*R_79;
                R_79 = R_63+R_79; R_79 = R_10*R_79; R_79 = Rationnal(240,1)*R_79; R_83 = R_104+R_83;
                R_83 = R_69*R_83; R_64 = R_83+R_64; R_64 = Rationnal(4,1)*R_64; R_64 = R_15+R_64;
                R_64 = R_107*R_64; R_64 = R_17+R_64; R_64 = R_84+R_64; R_82 = R_64+R_82;
                R_82 = R_69*R_82; R_91 = R_104+R_91; R_64 = R_107*R_91; R_64 = R_44+R_64;
                R_64 = R_69*R_64; R_64 = Rationnal(2,1)*R_64; R_91 = R_69*R_91; R_91 = R_92+R_91;
                R_91 = R_69*R_91; R_91 = R_93+R_91; R_104 = R_94+R_104; R_104 = R_107*R_104;
                R_104 = R_65+R_104; R_104 = R_69*R_104; R_95 = R_24+R_95; R_95 = R_107*R_95;
                R_45 = R_95+R_45; R_45 = R_107*R_45; R_36 = R_45+R_36; R_36 = R_107*R_36;
                R_36 = R_14+R_36; R_64 = R_36+R_64; R_71 = R_64+R_71; R_71 = R_107*R_71;
                R_45 = R_29+R_45; R_45 = R_72+R_45; R_45 = R_69*R_45; R_23 = R_45+R_23;
                R_23 = Rationnal(2,1)*R_23; R_65 = R_65+R_95; R_65 = R_107*R_65; R_65 = R_29+R_65;
                R_75 = R_65+R_75; R_75 = R_107*R_75; R_25 = R_75+R_25; R_76 = R_25+R_76;
                R_76 = R_42*R_76; R_76 = Rationnal(12,1)*R_76; R_95 = R_66+R_95; R_95 = R_107*R_95;
                R_95 = R_40+R_95; R_95 = R_107*R_95; R_96 = R_24+R_96; R_40 = R_21*R_96;
                R_40 = Rationnal(24,1)*R_40; R_40 = R_71+R_40; R_40 = R_4+R_40; R_40 = R_120+R_40;
                R_40 = R_23+R_40; R_98 = R_40+R_98; R_98 = R_69*R_98; R_98 = R_46+R_98;
                R_98 = Rationnal(2,1)*R_98; R_48 = R_96*R_48; R_31 = R_48+R_31; R_31 = R_107*R_31;
                R_48 = R_69*R_96; R_48 = R_105+R_48; R_48 = Rationnal(2,1)*R_48; R_96 = R_107*R_96;
                R_67 = R_67+R_96; R_67 = R_42*R_67; R_67 = Rationnal(12,1)*R_67; R_67 = R_9+R_67;
                R_96 = R_68+R_96; R_96 = R_107*R_96; R_96 = R_41+R_96; R_48 = R_96+R_48;
                R_24 = R_106+R_24; R_13 = R_24*R_13; R_21 = R_21*R_24; R_21 = Rationnal(48,1)*R_21;
                R_67 = R_21+R_67; R_102 = R_24*R_102; R_9 = R_9+R_102; R_111 = R_9+R_111;
                R_111 = R_49+R_111; R_118 = R_111+R_118; R_118 = R_107*R_118; R_118 = R_12+R_118;
                R_118 = R_37+R_118; R_118 = R_113+R_118; R_39 = R_118+R_39; R_39 = R_107*R_39;
                R_114 = R_102+R_114; R_114 = R_27+R_114; R_26 = R_114+R_26; R_114 = R_69*R_26;
                R_114 = Rationnal(2,1)*R_114; R_80 = R_114+R_80; R_80 = R_47+R_80; R_39 = R_80+R_39;
                R_39 = Rationnal(3,1)*R_39; R_39 = R_0+R_39; R_26 = R_107*R_26; R_26 = R_112+R_26;
                R_26 = Rationnal(3,1)*R_26; R_26 = R_1+R_26; R_26 = R_54+R_26; R_102 = R_110+R_102;
                R_102 = Rationnal(3,1)*R_102; R_67 = R_102+R_67; R_67 = R_90+R_67; R_42 = R_42*R_24;
                R_90 = Rationnal(12,1)*R_42; R_109 = R_109+R_90; R_109 = R_16+R_109; R_109 = R_69*R_109;
                R_109 = Rationnal(4,1)*R_109; R_109 = R_2+R_109; R_14 = R_90+R_14; R_97 = R_14+R_97;
                R_97 = R_107*R_97; R_97 = R_6+R_97; R_97 = R_107*R_97; R_109 = R_97+R_109;
                R_109 = R_73+R_109; R_109 = R_74+R_109; R_109 = R_43+R_109; R_109 = R_107*R_109;
                R_39 = R_109+R_39; R_39 = R_76+R_39; R_39 = R_70+R_39; R_39 = R_79+R_39;
                R_18 = R_90+R_18; R_88 = R_18+R_88; R_56 = R_88+R_56; R_56 = R_107*R_56;
                R_56 = R_8+R_56; R_56 = R_59+R_56; R_56 = R_107*R_56; R_26 = R_56+R_26;
                R_53 = R_26+R_53; R_82 = R_53+R_82; R_82 = R_89+R_82; R_89 = Rationnal(6,1)*R_42;
                R_16 = R_89+R_16; R_16 = R_108+R_16; R_16 = R_107*R_16; R_16 = R_7+R_16;
                R_101 = R_16+R_101; R_101 = R_69*R_101; R_31 = R_101+R_31; R_31 = R_13+R_31;
                R_31 = Rationnal(4,1)*R_31; R_39 = R_31+R_39; R_39 = R_30+R_39; R_98 = R_39+R_98;
                R_117 = R_89+R_117; R_117 = R_19+R_117; R_62 = R_117+R_62; R_62 = R_107*R_62;
                R_91 = R_62+R_91; R_91 = R_119+R_91; R_91 = Rationnal(2,1)*R_91; R_67 = R_91+R_67;
                R_42 = Rationnal(18,1)*R_42; R_42 = R_20+R_42; R_95 = R_42+R_95; R_104 = R_95+R_104;
                R_95 = r_0[0]; R_95 = exp(R_95); r[0] = R_95; R_98 = R_95*R_98;
                R_98 = Rationnal(1,40320)*R_98; r[8] = R_98; R_82 = R_95*R_82; R_82 = Rationnal(1,5040)*R_82;
                r[7] = R_82; R_67 = R_95*R_67; R_67 = Rationnal(1,720)*R_67; r[6] = R_67;
                R_104 = R_95*R_104; R_104 = Rationnal(1,120)*R_104; r[5] = R_104; R_48 = R_95*R_48;
                R_48 = Rationnal(1,24)*R_48; r[4] = R_48; R_103 = R_95*R_103; R_103 = Rationnal(1,6)*R_103;
                r[3] = R_103; R_24 = R_95*R_24; R_24 = Rationnal(1,2)*R_24; r[2] = R_24;
                R_107 = R_95*R_107; r[1] = R_107; 
                break;
            }
            #endif // WANT_SERIES_789
            default:
                th->add_error( "TODO : PolynomialExpansion for type "+std::string(Nstring(a->type))+" and order > 9 -> see file 'ex.cpp'.", tok );
        }
    }
}
