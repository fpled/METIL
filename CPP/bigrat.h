#ifndef BIGRAT_H
#define BIGRAT_H

#include <limits>

#include <assert.h>
#include <cstdlib>
#include <iomanip>

#include "bigint.h"
#include "bool.h"
#include "metil_ieee754_config.h"

///
#define BASE_BIG_RAT 32768

/**
   Big rational
 */
template< int base_=BASE_BIG_RAT, class T=int, int offset=0, bool owning = true >
struct BigRat {
    static const int base = base_;
    typedef BigInt<base,T,offset> BI;
    
    ///
    BigRat():den(1) {}
    ///
    template<class T2> BigRat( T2 numerator, T2 denominator = 1 ) : num(numerator), den(denominator) { cannonicalize(); }
    ///
    BigRat(const BigRat &b) : num( b.num ), den( b.den ) { }
    
    void init() { num.init(0); den.init(1); }
    
    void init( const BigRat &b ) { num.init(b.num); den.init(b.den); }
    
    void destroy() {
        den.destroy();
        num.destroy(); 
    }
    
    ///
    BigRat( const Bool &val ) : num(bool(val)), den(1) { }
    ///
    void init_from_sign_expo_mant( bool sign, Int32 expo, Unsigned64 mant ) {
        num = mant;
        den = 1;
        BI b2( 2 );
        if      ( expo > 0 ) { while ( expo-- ) num = num * b2; }
        else if ( expo < 0 ) { while ( expo++ ) den = den * b2; }
        else                 { num = 1; den = 1; }
        if ( sign )
            num.val = - num.val;
        cannonicalize();
    }
    ///
    void setNum( const Float32 &val ) { approx<Float32,base,T,offset,owning>(val,num,den); }
    void setNum( const Float64 &val ) { approx<Float64,base,T,offset,owning>(val,num,den); }
    void setNum( const Float96 &val ) { approx<Float96,base,T,offset,owning>(val,num,den); }
    

//     BigRat( const Float32 &val ) {
//         if ( val == 0 ) { num = 0; den = 1; return; }
//         const ieee754_float &v = reinterpret_cast<const ieee754_float &>( val );
//         init_from_sign_expo_mant( v.ieee.negative, (Int32)v.ieee.exponent - IEEE754_FLOAT_BIAS - 23, v.ieee.mantissa + 0x00800000UL );
//     }
    BigRat( const Float32 &val ) { approx<Float32,base,T,offset,owning>(val,num,den); }

    ///
    BigRat( const Float64 &val ) { approx<Float64,base,T,offset,owning>(val,num,den); }

    /*!
    La fonction utilise les fractions continues : cf par exemple manuel de calcul numérique appliqué C. Guilpin, ed. EDP Sciences.
    Un mot sur le paramètre tol. On compare notre fraction q à la valeur initiale val en calculant | q - val | / (1 + | val | ) et en comparant le résultat à l'epsilon du type flottant fois 2^tol. Expérimentalement tol = 4 convient.
    Un pire cas pour les fractions continues est le nombre d'or, noté phi,  solution positive de l'équation x^2 = x + 1 soit x = 1 + 1 / x d'où le développement en fraction continue : 1 + 1 / ( 1 + 1 / ( 1 + 1 / ( ...   ))))). Tous les coefficients valent un. Voir la page de wikipedia \a http://fr.wikipedia.org/wiki/Nombre_d'or . C'est pour ce nombre que j'ai choisi une limite de 40 pour la boucle.
    phi = (1+sqrt(5))/2 ~ 1.618033988749894848204586834365638117720309179805.

    \friend raphael.pasquier@lmt.ens-cachan.fr

    */
    template<class TFLOAT,int base2,class T2,int offset2,bool owning2> static void approx(const TFLOAT &val, BigInt<base2,T2,offset2,owning2> &nume, BigInt<base2,T2,offset2,owning2> &deno, int tol=4 ) {
        Float64 ent,v,abs_v, i_abs_v;
        BigInt<base2,T2,offset2,owning2> tmpA,tmpB;

        abs_v = fabs(val);
        i_abs_v = 1./(1.+abs_v);
        v = modf(abs_v,&ent);
        //std::cout << " frac  = " << std::setprecision(17) << v << "    ent = " << std::setprecision(17) << ent << std::endl;
        BigInt<base2,T2,offset2,owning2> A0(ent);
        BigInt<base2,T2,offset2,owning2> B0(1);

        //if (v/(ent+1.)<1.e-10){ //if (fabs(v)<ldexp(std::numeric_limits<Float64>::epsilon(),20)) {
        if ((fabs(abs_v-Float64(A0)/Float64(B0))*i_abs_v)<ldexp(std::numeric_limits<TFLOAT>::epsilon(),tol)) {
            if (val>0.)
                nume = A0;
            else
                nume = -A0;
            deno = B0;
            return;
        } else
            v = 1./v;
        v = modf(v,&ent);
        BigInt<base2,T2,offset2,owning2> B1(ent);
        BigInt<base2,T2,offset2,owning2> A1;
        BigInt<base2,T2,offset2,owning2> one(1);
        A1 = A0*B1+one;
        //std::cout << " frac  = " << std::setprecision(17) << v << "    ent = " << std::setprecision(17) << ent << std::endl;
        //if (v/(ent+1.)<1.e-8) { //if (fabs(v)<ldexp(std::numeric_limits<Float64>::epsilon(),20)) {
        if ((fabs(abs_v-Float64(A1)/Float64(B1))*i_abs_v)<ldexp(std::numeric_limits<TFLOAT>::epsilon(),tol)) {
            if (val>0.)
                nume = A1;
            else
                nume = -A1;
            deno = B1;
            return;
        } else
            v = 1./v;

        for(int jj=0;jj<40;jj++) {
            v = modf(v,&ent);
            //std::cout << " @ " << jj << " @  frac  = " << std::setprecision(17) << v << "    ent = " << std::setprecision(17) << ent << std::endl;
            tmpA = A1;
            tmpB = B1;
            BigInt<base2,T2,offset2,owning2> tmp(ent);
            A1 = A1 * tmp + A0;
            B1 = B1 * tmp + B0;
            //std::cout << jj << " ____ " << A1 << "  /  " << B1 << " ____ " << std::endl;
            //std::cout << " erreur = " << (fabs(abs_v-Float64(A1)/Float64(B1))*i_abs_v) << std::endl;
            //if (v/(ent+1.)<1.e-8) { //if (fabs(v)<ldexp(std::numeric_limits<Float64>::epsilon(),26)) {
            if ((fabs(abs_v-Float64(A1)/Float64(B1))*i_abs_v)<ldexp(std::numeric_limits<TFLOAT>::epsilon(),tol))
                break;
            v = 1./v;
            A0 = tmpA;
            B0 = tmpB;
        }
        if (val>0.)
            nume = A1;
        else
            nume = -A1;
        deno = B1;
    }

//     BigRat( const Float64 &val ) {
//         const ieee754_double &v = reinterpret_cast<const ieee754_double &>( val );
//         init_from_sign_expo_mant( v.ieee.negative, (Int32)v.ieee.exponent - IEEE754_DOUBLE_BIAS - 52, ( Unsigned64( v.ieee.mantissa0 + 0x100000UL ) << 32 ) + v.ieee.mantissa1 );
//     }

//     BigRat( const Float96 &val ) {
//         if ( val == 0 ) { num = 0; den = 1; return; }
//         const ieee854_long_double &v = reinterpret_cast<const ieee854_long_double &>( val );
//         init_from_sign_expo_mant( v.ieee.negative, (Int32)v.ieee.exponent - IEEE854_LONG_DOUBLE_BIAS - 63, ( Unsigned64( v.ieee.mantissa0 ) << 32 ) + v.ieee.mantissa1 );
//     }

    BigRat( const Float96 &val ) {
        approx<Float96,base,T,offset,owning>(val,num,den);
    }

    ///
    template<int b2,class T2,int o2,bool own2> BigRat &operator=(const BigRat<b2,T2,o2,own2> &b) {
        num = b.num;
        den = b.den;
        return *this;
    }
    
    ///
    BigRat &operator=(const BigRat &b) {
        num = b.num;
        den = b.den;
        return *this;
    }

    BigRat &operator=(const Bool &val) { num = (int)val; den = 1; return *this; }
    BigRat &operator=(const Int32 &val) { num = val; den = 1; return *this; }
    BigRat &operator=(const Unsigned32 &val) { num = val; den = 1; return *this; }
    BigRat &operator=(const Int64 &val) { num = val; den = 1; return *this; }
    BigRat &operator=(const Unsigned64 &val) { num = val; den = 1; return *this; }
    
    BigRat &operator=(const Float32 &val) { approx<Float32,base,T,offset,owning>(val,num,den); return *this; }
    BigRat &operator=(const Float64 &val) { approx<Float64,base,T,offset,owning>(val,num,den); return *this; }
    BigRat &operator=(const Float96 &val) { approx<Float96,base,T,offset,owning>(val,num,den); return *this; }

    /// assume base 10
    BigRat(unsigned nnum,const char *snum,unsigned nden,const char *sden) {
        BigInt<base,T,offset,owning> m(10);
        for(unsigned i=0;i<nnum;++i) {
            num = m * num;
            num = num + BigInt<base,T,offset,owning>(snum[i]-'0');
        }
        for(unsigned i=0;i<nden;++i) {
            den = m * den;
            den = den + BigInt<base,T,offset,owning>(sden[i]-'0');
        }
    }
    ///
    BigRat(unsigned si,const char *str) {
        den = 1;
    
        bool comma = false;
        for(unsigned i=0;i<si;++i) {
            if ( str[i]=='e' or str[i]=='E' ) {
                bool minus = false;
                ++i;
                if ( str[i]=='-' ) { ++i; minus = true; }
                else if ( str[i]=='+' ) { ++i; }
                unsigned expo = 0;
                for(;i<si and str[i]>='0' and str[i]<='9';++i) {
                    expo *= 10;
                    expo += (int)( str[i] - '0' );
                }
                // we put expo in den or in num
                if ( minus ) den.mul_by_base(expo);
                else         num.mul_by_base(expo);
                break;
            }
            else if ( str[i]=='.' ) {
                comma = true;
            }
            else {
                if ( comma )
                    den.mul_by_base();
                num.mul_by_base();
                num = num + BigInt<base,T,offset>( str[i] - '0' );
            }
        }
        cannonicalize();
    }    
    ///
    void cannonicalize() {
        if ( den.val < 0 ) {
            num.val = -num.val;
            den.val = -den.val;
        }
        //
        if ( num.val < 0 ) {
            num.val = -num.val;
            BigInt<base,T,offset> g = gcd( num, den );
            if ( g.val ) {
                num = num / g;
                den = den / g;
            }
            num.val = -num.val;
        } else {
            BigInt<base,T,offset> g = gcd( num, den );
            if ( g.val ) {
                num = num / g;
                den = den / g;
            }
        }
        
    }
    ///
    BigRat operator-() const { BigRat res = *this; res.num.val = - res.num.val; return res; }
    
    void operator+=(const BigRat &b) { *this = *this + b; }
    void operator-=(const BigRat &b) { *this = *this - b; }
    void operator*=(const BigRat &b) { *this = *this * b; }
    void operator/=(const BigRat &b) { *this = *this / b; }
    void operator%=(const BigRat &b) { *this = *this % b; }
    
    ///
    operator bool   () const { return bool(num); }
    
    operator Int8 () const { return Int8 ( num / den ); }
    operator Int16() const { return Int16( num / den ); }
    operator Int32() const { return Int32( num / den ); }
    operator Int64() const { return Int64( num / den ); }
    
    operator Unsigned8 () const { return Unsigned8 ( num / den ); }
    operator Unsigned16() const { return Unsigned16( num / den ); }
    operator Unsigned32() const { return Unsigned32( num / den ); }
    operator Unsigned64() const { return Unsigned64( num / den ); }
    
    operator Float32() const { return Float32(num) / Float32(den); }
    operator Float64() const {
        static const unsigned lim = unsigned( log( std::numeric_limits<Float64>::max() ) / log( base ) );
        if ( num.n > lim or den.n > lim ) {
            BigInt<base,T> n = num;
            BigInt<base,T> d = den;
            assert ( abs( num.n - den.n ) < int(lim) );
            while ( n.n >= lim or d.n >= lim ) {
                n.div_by_base();
                d.div_by_base();
            }
            return Float64(n) / Float64(d);
        }
        // std::cout << Float64(num) << " " << Float64(den) << " " << Float64(num) / Float64(den) << std::endl;
        return Float64(num) / Float64(den);
    }
    operator Float96() const { return Float96(num) / Float96(den); }
    
    ///
    bool operator==(const BigRat &b) const { return num==b.num and den==b.den; }
    bool operator!=(const BigRat &b) const { return num!=b.num or  den!=b.den; }
    bool operator<(const BigRat &b)  const { BigInt<base,T,offset> i = num * b.den, j = b.num * den; return i <  j; }
    bool operator<=(const BigRat &b) const { BigInt<base,T,offset> i = num * b.den, j = b.num * den; return i <= j; }
    bool operator>(const BigRat &b)  const { BigInt<base,T,offset> i = num * b.den, j = b.num * den; return i >  j; }
    bool operator>=(const BigRat &b) const { BigInt<base,T,offset> i = num * b.den, j = b.num * den; return i >= j; }
    
    bool operator>=(int b) const { return *this >= BigRat(b); }
    
    bool is_integer  ()      const { return den.n == 0 and den.val == 1; }
    bool is_odd      ()      const { return is_integer() and num.is_odd (); }
    bool is_even     ()      const { return is_integer() and num.is_even(); }
    bool is_neg      ()      const { return num.val < 0; }
    bool is_pos      ()      const { return num.val > 0; }
    bool is_zero     ()      const { return num.val ==  0; }
    bool is_one      ()      const { return num.n == 0 and num.val ==  1 and den.n == 0 and den.val == 1; }
    bool is_two      ()      const { return num.n == 0 and num.val ==  2 and den.n == 0 and den.val == 1; }
    bool is_three    ()      const { return num.n == 0 and num.val ==  3 and den.n == 0 and den.val == 1; }
    bool is_minus_one()      const { return num.n == 0 and num.val == -1 and den.n == 0 and den.val == 1; }
    bool is_minus_two()      const { return num.n == 0 and num.val == -2 and den.n == 0 and den.val == 1; }
    bool is_pos_or_null()    const { return num.val >= 0; }
    bool is_neg_or_null()    const { return num.val <= 0; }
    bool is_one_half()       const { return num.n == 0 and num.val ==  1 and den.n == 0 and den.val == 2; }
    bool is_minus_one_half() const { return num.n == 0 and num.val == -1 and den.n == 0 and den.val == 2; }
    
    BigInt<base,T,offset,owning> num, den;
};

template<class T> bool are_different(BigRat<> a,T b) { return a != BigRat<>(b); } /// done only to avoid "warning: comparison between signed and unsigned"
template<class T> bool are_different(T a,BigRat<> b) { return BigRat<>(a) != b; } /// done only to avoid "warning: comparison between signed and unsigned"
inline bool are_different(BigRat<> a,BigRat<> b) { return a != b; } /// done only to avoid "warning: comparison between signed and unsigned"

template<int base, class T, int offset, bool owning> std::ostream &operator<<(std::ostream &os, const BigRat<base,T,offset,owning> &v ) {
    os << v.num;
    if ( v.den.n == 0 and v.den.val == 1 )
        return os;
    os << "/" << v.den;
    return os;
}
        
template<int base,class T,int offset,bool owning> BigRat<base,T,offset> inv( const BigRat<base,T,offset,owning> &a ) {
    if ( a.num.val == 0 )
        return 0;
    if ( a.num.val < 0 )
        return BigRat<base,T,offset>( -a.den, -a.num );
    return BigRat<base,T,offset>( a.den, a.num );
}

template<int base,class T,int offset,bool owninga,bool owningb> BigRat<base,T,offset> operator+(const BigRat<base,T,offset,owninga> &a,const BigRat<base,T,offset,owningb> &b) {
    BigRat<base,T,offset> res;
    res.den = a.den * b.den;
//     a.num.basic_assert(); 
//     a.den.basic_assert(); 
//     b.num.basic_assert(); 
//     b.den.basic_assert();
//     (b.num * a.den).basic_assert();
//     (a.num * b.den).basic_assert();
    res.num = a.num * b.den + b.num * a.den;
    res.cannonicalize();
//     if ( std::abs( Float96( res ) - ( Float96( a ) + Float96( b ) ) ) > ( std::abs( Float96( res ) ) + std::abs( Float96( a ) ) + std::abs( Float96( b ) ) ) * 1e-16 ) {
//         std::cout << res << " ?= " << a << " " << b << std::endl;
//         std::cout << Float96( res ) << " ?= " << Float96( a ) << " + " << Float96( b ) << std::endl;
//         assert( 0 );
//     }
    return res;
}

template<int base,class T,int offset,bool owninga,bool owningb> BigRat<base,T,offset> operator-(const BigRat<base,T,offset,owninga> &a,const BigRat<base,T,offset,owningb> &b) {
    BigRat<base,T,offset> res;
    res.den = a.den * b.den;
    res.num = a.num * b.den - b.num * a.den;
    res.cannonicalize();
//     if ( std::abs( Float96( res ) - ( Float96( a ) - Float96( b ) ) ) > ( std::abs( Float96( res ) ) + std::abs( Float96( a ) ) + std::abs( Float96( b ) ) ) * 1e-16 ) {
//         std::cout << res << " ?= " << a << " " << b << std::endl;
//         std::cout << Float96( res ) << " ?= " << Float96( a ) << " + " << Float96( b ) << std::endl;
//         assert( 0 );
//     }
    return res;
}

template<int base,class T,int offset,bool owninga,bool owningb> BigRat<base,T,offset> operator*(const BigRat<base,T,offset,owninga> &a,const BigRat<base,T,offset,owningb> &b) {
    BigRat<base,T,offset> res;
    res.num = a.num * b.num;
    res.den = a.den * b.den;
    res.cannonicalize();
//     if ( std::abs( Float96( res ) - ( Float96( a ) * Float96( b ) ) ) > std::abs( Float96( res ) ) * 1e-16 ) {
//         std::cout << res << " ?= " << a << " " << b << std::endl;
//         std::cout << Float96( res ) << " ?= " << Float96( a ) << " + " << Float96( b ) << std::endl;
//         assert( 0 );
//     }
    return res;
}

template<int base,class T,int offset,bool owninga,bool owningb> BigRat<base,T,offset> operator/(const BigRat<base,T,offset,owninga> &a,const BigRat<base,T,offset,owningb> &b) {
    BigRat<base,T,offset> res;
    res.num = a.num * b.den;
    res.den = a.den * b.num;
    res.cannonicalize();
//     if ( std::abs( Float96( res ) - ( Float96( a ) / Float96( b ) ) ) > std::abs( Float96( res ) ) * 1e-16 ) {
//         std::cout << res << " ?= " << a << " " << b << std::endl;
//         std::cout << Float96( res ) << " ?= " << Float96( a ) << " + " << Float96( b ) << std::endl;
//         assert( 0 );
//     }
    return res;
}

template<int base,class T,int offset,bool owninga,bool owningb> BigRat<base,T,offset> operator%(const BigRat<base,T,offset,owninga> &a,const BigRat<base,T,offset,owningb> &b) {
    BigRat<base,T,offset> res;
    res.num = ( a.num * b.den ) % ( b.num * a.den );
    res.den = a.den * b.den;
    res.cannonicalize();
    return res;
}

template<int base,class T,int offset,bool owning> BigRat<base,T,offset> abs(const BigRat<base,T,offset,owning> &a) {
    BigRat<base,T,offset> res = a;
    res.num.val = abs(a.num.val);
    return res;
}

template<int base,class T,int offset,bool owninga,bool owningb> BigRat<base,T,offset> mod(const BigRat<base,T,offset,owninga> &a,const BigRat<base,T,offset,owningb> &b) {
    if ( a.is_integer() and b.is_integer() )
        return a.num % b.num;
    BigRat<base,T,offset> res = a;
    assert(0); // TODO
    return res;
}

template<int base,class T,int offset,bool owning> BigRat<base,T,offset> floor(const BigRat<base,T,offset,owning> &a) { /// hum
    using namespace std;
    return int( floor( double( a ) ) );
}

template<int base,class T,int offset,bool owning> BigRat<base,T,offset> ceil (const BigRat<base,T,offset,owning> &a) { /// hum
    using namespace std;
    return int( ceil( double( a ) ) );
}

template<int base,class T,int offset,bool owning> BigRat<base,T,offset> round(const BigRat<base,T,offset,owning> &a) { /// hum
    using namespace std;
    return int( round( double( a ) ) );
}

#endif // BIGRAT_H
