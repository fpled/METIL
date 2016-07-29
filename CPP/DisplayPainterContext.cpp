#include "DisplayPainterContext.h"

// DisplayPainterContext::Matrix::Matrix() {
//     for(unsigned i=0,c=0;i<4;++i)
//         for(unsigned j=0;j<4;++j,++c)
//             data[ c ] = (i==j);
// }
// 
// DisplayPainterContext::Matrix DisplayPainterContext::Matrix::operator*( const Matrix &m ) {
//     Matrix res;
//     for(unsigned i=0;i<4;i++) {
//         for(unsigned j=0;j<4;j++) {
//             double tmp = operator()(i,0)*m(0,j);
//             for(unsigned k=1;k<4;k++)
//                 tmp += operator()(i,k)*m(k,j);
//             res(i,j) = tmp;
//         }
//     }
//     return res;
//     
// }
// 
// DisplayPainterContext::Point DisplayPainterContext::Matrix::operator()( const DisplayPainterContext::Point &p ) const {
//     return P(
//         data[ 4 * 0 + 0 ] * p.x + data[ 4 * 0 + 1 ] * p.y + data[ 4 * 0 + 2 ] * p.z + data[ 4 * 0 + 3 ],
//         data[ 4 * 1 + 0 ] * p.x + data[ 4 * 1 + 1 ] * p.y + data[ 4 * 1 + 2 ] * p.z + data[ 4 * 1 + 3 ],
//         data[ 4 * 2 + 0 ] * p.x + data[ 4 * 2 + 1 ] * p.y + data[ 4 * 2 + 2 ] * p.z + data[ 4 * 2 + 3 ],
//         data[ 4 * 3 + 0 ] * p.x + data[ 4 * 3 + 1 ] * p.y + data[ 4 * 3 + 2 ] * p.z + data[ 4 * 3 + 3 ]
//     );
// }
// 
// DisplayPainterContext::Matrix DisplayPainterContext::Matrix::inverse() const {
//     typedef double T;
//     Matrix res;
//     T reg0=data[14]*data[9]; T reg1=data[13]*data[10]; reg1=reg0-reg1; reg0=data[7]*reg1; T reg2=data[15]*data[10];
//     T reg3=data[14]*data[11]; reg3=reg2-reg3; reg2=data[5]*reg3; T reg4=data[15]*data[9]; T reg5=data[11]*data[13];
//     reg5=reg4-reg5; reg4=data[6]*reg5; reg4=reg2-reg4; reg4=reg0+reg4; reg0=data[0]*reg4;
//     reg2=data[14]*data[8]; T reg6=data[12]*data[10]; reg6=reg2-reg6; reg2=reg6*data[7]; T reg7=data[4]*reg3;
//     T reg8=data[8]*data[15]; T reg9=data[12]*data[11]; reg9=reg8-reg9; reg8=reg9*data[6]; reg8=reg7-reg8;
//     reg8=reg2+reg8; reg2=reg8*data[1]; reg2=reg0-reg2; reg0=data[4]*reg5; reg7=reg9*data[5];
//     reg7=reg0-reg7; reg0=data[8]*data[13]; T reg10=data[12]*data[9]; reg10=reg0-reg10; reg0=reg10*data[7];
//     reg0=reg7+reg0; reg7=reg0*data[2]; reg7=reg2+reg7; reg2=reg10*data[6]; T reg11=data[4]*reg1;
//     T reg12=data[5]*reg6; reg12=reg11-reg12; reg12=reg2+reg12; reg2=data[3]*reg12; reg2=reg7-reg2;
//     res[0]=reg4/reg2; reg4=data[1]*reg3; reg7=data[2]*reg5; reg7=reg4-reg7; reg4=data[3]*reg1;
//     reg4=reg7+reg4; reg4=reg4/reg2; res[1]=-reg4; reg4=data[14]*data[5]; reg7=data[6]*data[13];
//     reg7=reg4-reg7; reg4=data[3]*reg7; reg11=data[6]*data[15]; T reg13=data[14]*data[7]; reg13=reg11-reg13;
//     reg11=data[1]*reg13; T reg14=data[5]*data[15]; T reg15=data[13]*data[7]; reg15=reg14-reg15; reg14=data[2]*reg15;
//     reg14=reg11-reg14; reg14=reg4+reg14; res[2]=reg14/reg2; reg4=data[5]*data[10]; reg11=data[6]*data[9];
//     reg11=reg4-reg11; reg4=data[3]*reg11; reg14=data[6]*data[11]; T reg16=data[7]*data[10]; reg16=reg14-reg16;
//     reg14=data[1]*reg16; T reg17=data[5]*data[11]; T reg18=data[9]*data[7]; reg18=reg17-reg18; reg17=data[2]*reg18;
//     reg17=reg14-reg17; reg17=reg4+reg17; reg17=reg17/reg2; res[3]=-reg17; res[8]=reg0/reg2;
//     reg8=reg8/reg2; res[4]=-reg8; reg0=data[3]*reg6; reg3=data[0]*reg3; reg4=reg9*data[2];
//     reg4=reg3-reg4; reg4=reg0+reg4; res[5]=reg4/reg2; reg0=data[14]*data[4]; reg3=data[12]*data[6];
//     reg3=reg0-reg3; reg0=data[3]*reg3; reg13=data[0]*reg13; reg4=data[4]*data[15]; reg8=data[12]*data[7];
//     reg8=reg4-reg8; reg4=data[2]*reg8; reg4=reg13-reg4; reg4=reg0+reg4; reg4=reg4/reg2;
//     res[6]=-reg4; reg0=data[4]*data[10]; reg4=data[6]*data[8]; reg4=reg0-reg4; reg0=data[3]*reg4;
//     reg16=data[0]*reg16; reg13=data[4]*data[11]; reg14=data[8]*data[7]; reg14=reg13-reg14; reg13=data[2]*reg14;
//     reg13=reg16-reg13; reg13=reg0+reg13; res[7]=reg13/reg2; reg0=reg10*data[3]; reg5=data[0]*reg5;
//     reg9=reg9*data[1]; reg9=reg5-reg9; reg9=reg0+reg9; reg9=reg9/reg2; res[9]=-reg9;
//     reg0=data[4]*data[13]; reg5=data[12]*data[5]; reg5=reg0-reg5; reg0=data[3]*reg5; reg15=data[0]*reg15;
//     reg8=data[1]*reg8; reg8=reg15-reg8; reg8=reg0+reg8; res[10]=reg8/reg2; reg0=data[4]*data[9];
//     reg8=data[5]*data[8]; reg8=reg0-reg8; reg0=data[3]*reg8; reg18=data[0]*reg18; reg14=data[1]*reg14;
//     reg14=reg18-reg14; reg14=reg0+reg14; reg14=reg14/reg2; res[11]=-reg14; reg12=reg12/reg2;
//     res[12]=-reg12; reg10=reg10*data[2]; reg1=data[0]*reg1; reg6=reg6*data[1]; reg6=reg1-reg6;
//     reg6=reg10+reg6; res[13]=reg6/reg2; reg5=data[2]*reg5; reg7=data[0]*reg7; reg3=data[1]*reg3;
//     reg3=reg7-reg3; reg3=reg5+reg3; reg3=reg3/reg2; res[14]=-reg3; reg8=data[2]*reg8;
//     reg11=data[0]*reg11; reg4=data[1]*reg4; reg4=reg11-reg4; reg4=reg8+reg4; res[15]=reg4/reg2;
//     return res;
// }

DisplayPainterContext::DisplayPainterContext( int w, int h, double x0, double y0, double x1, double y1 ) : img( w, h ), x0(x0), y0(y0), x1(x1), y1(y1) {


}



