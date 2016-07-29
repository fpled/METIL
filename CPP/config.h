#ifndef CONFIG_H
#define CONFIG_H

#include <cmath>
#include <assert.h>
#include "typeconfig.h"
#include "bigrat.h"

typedef BigRat<> Rationnal;
typedef BigInt<Rationnal::base> BigInteger;

#include "basic_type_promote.h"

typedef long int SignedSize;
typedef void *UntypedPtr;
                
template<class T> void fake_func(const T &val) {}

namespace METIL_NS {
    template<class T> T mod(T a,T b) { return a % b; }
    inline Float32 mod(Float32 a,Float32 b) { return fmod( a, b ); }
    inline Float64 mod(Float64 a,Float64 b) { return fmod( a, b ); }
    inline Float96 mod(Float96 a,Float96 b) { return fmod( a, b ); }
    inline bool mod(Bool a,Bool b) { return false; }
    
    template<class T> T add( T a, T b ) { return a + b; }
    template<class T> T sub( T a, T b ) { return a - b; }
    template<class T> T mul( T a, T b ) { return a * b; }
    template<class T> T div( T a, T b ) { return a / b; }
    template<class T> T pow( T a, T b ) { assert(0); return 0; }
    
    inline bool add( Bool a,Bool b ) { return a + b; }
    inline bool sub( Bool a,Bool b ) { return a - b; }
    inline bool mul( Bool a,Bool b ) { return a * b; }
    inline bool div( Bool a,Bool b ) { return a / b; }
    inline bool pow( Bool a,Bool b ) { assert(0); return 0; }
    
    inline Float32 pow( Float32 a, Float32 b ) { return std::pow( a, b ); }
    inline Float64 pow( Float64 a, Float64 b ) { return std::pow( a, b ); }
    inline Float96 pow( Float96 a, Float96 b ) { return std::pow( a, b ); }
    
    inline Float32 pow( Float32 a, const Rationnal &b ) { return std::pow( a, Float32(b) ); }
    inline Float64 pow( Float64 a, const Rationnal &b ) { return std::pow( a, Float64(b) ); }
    inline Float96 pow( Float96 a, const Rationnal &b ) { return std::pow( a, Float96(b) ); }
    //     inline Int8  pow( Int8  a, Int8  b ) { return pow( a, b ); }
    //     inline Int16 pow( Int16 a, Int16 b ) { return pow( a, b ); }
    //     inline Int32 pow( Int32 a, Int32 b ) { return pow( a, b ); }
    //     inline Int64 pow( Int64 a, Int64 b ) { return pow( a, b ); }
    //     
    //     inline Unsigned8  pow( Unsigned8  a, Unsigned8  b ) { return pow( a, b ); }
    //     inline Unsigned16 pow( Unsigned16 a, Unsigned16 b ) { return pow( a, b ); }
    //     inline Unsigned32 pow( Unsigned32 a, Unsigned32 b ) { return pow( a, b ); }
    //     inline Unsigned64 pow( Unsigned64 a, Unsigned64 b ) { return pow( a, b ); }
    inline Rationnal pow( Rationnal a, Bool b ) { return b ? a : Rationnal(1); }
    
    template<class T> Rationnal pow( Rationnal a, T b ) {
        if ( b == 0 )
            return Rationnal( 1, 1 );
        if ( b <= 0 ) {
            b = -b;
            std::swap( a.num, a.den );
        }
        if ( b == 1 )
            return a;
        Rationnal tmp = a;
        while ( --b ) {
            a.num = a.num * tmp.num;
            a.den = a.den * tmp.den;
        }
        a.cannonicalize(); // necessary ?
        return a;
    }
    
    template<class T> T div_int( T a, T b ) { return a / b; }
    template<class T> bool div_int( Bool a, T b ) { return false; }
    template<class T> bool div_int( T a, Bool b ) { return false; }
    inline bool div_int( Bool a, Bool b ) { return false; }
    
    inline BigRat<> div( Int8  a, Int8  b ) { return BigRat<>( a, b ); }
    inline BigRat<> div( Int16 a, Int16 b ) { return BigRat<>( a, b ); }
    inline BigRat<> div( Int32 a, Int32 b ) { return BigRat<>( a, b ); }
    inline BigRat<> div( Int64 a, Int64 b ) { return BigRat<>( a, b ); }

    //
    template<class T1,class T2> typename TypePromote<T1,T2,Mod>::T mod    ( T1 a,T2 b ) { typedef typename TypePromote<T1,T2,Mod>::T T; return METIL_NS::mod( T(a), T(b) ); }
    template<class T1,class T2> typename TypePromote<T1,T2,Add>::T add    ( T1 a,T2 b ) { typedef typename TypePromote<T1,T2,Add>::T T; return T(a)+T(b); }
    template<class T1,class T2> typename TypePromote<T1,T2,Sub>::T sub    ( T1 a,T2 b ) { typedef typename TypePromote<T1,T2,Sub>::T T; return T(a)-T(b); }
    template<class T1,class T2> typename TypePromote<T1,T2,Add>::T mul    ( T1 a,T2 b ) { typedef typename TypePromote<T1,T2,Mul>::T T; return T(a)*T(b); }
    template<class T1,class T2> typename TypePromote<T1,T2,Div>::T div    ( T1 a,T2 b ) { typedef typename TypePromote<T1,T2,Div>::T T; return T(a)/T(b); }
    template<class T1,class T2> typename TypePromote<T1,T2,Pow>::T pow    ( T1 a,T2 b ) { assert(0); return a; }
    template<class T1,class T2> typename TypePromote<T1,T2,Div_int>::T div_int( T1 a,T2 b ) { assert(0); return a; }
    inline Rationnal pow( Rationnal a,Rationnal b ) { assert(0); return a; }

    inline bool is_integer( const Int8       &val ) { return true; }
    inline bool is_integer( const Int16      &val ) { return true; }
    inline bool is_integer( const Int32      &val ) { return true; }
    inline bool is_integer( const Int64      &val ) { return true; }
    inline bool is_integer( const Unsigned8  &val ) { return true; }
    inline bool is_integer( const Unsigned16 &val ) { return true; }
    inline bool is_integer( const Unsigned32 &val ) { return true; }
    inline bool is_integer( const Unsigned64 &val ) { return true; }
    inline bool is_integer( const Float32    &val ) { return floor(val)==val; }
    inline bool is_integer( const Float64    &val ) { return floor(val)==val; }
    inline bool is_integer( const Float96    &val ) { return floor(val)==val; }
    inline bool is_integer( const Rationnal  &val ) { return val.den == Rationnal::BI(1); }

    template<class T> T abs( const T &val ) { return std::abs(val); }
    inline signed char abs( signed char val ) { return ( val > 0 ? val : val ); }
    inline short int abs( short int val ) { return ( val > 0 ? val : val ); }
    inline int abs( int val ) { return ( val > 0 ? val : val ); }
    inline long long abs( long long val ) { return ( val > 0 ? val : val ); }
    inline Rationnal abs( const Rationnal &val ) { Rationnal res = val; res.num.val = abs(res.num.val); return res; }
    inline Unsigned8  abs( const Unsigned8  &val ) { return val; }
    inline Unsigned16 abs( const Unsigned16 &val ) { return val; }
    inline Unsigned32 abs( const Unsigned32 &val ) { return val; }
    inline Unsigned64 abs( const Unsigned64 &val ) { return val; }
    inline bool abs( const Bool &val ) { return val; }
    
    template<class T1,class T2> bool equal( const T1 &a, const T2 &b ) { return a == b; }
    
    /*
lst := ["Int8" "Unsigned8" "Int16" "Unsigned16" "Int32" "Unsigned32" "Int64" "Unsigned64" "Float32" "Float64" "Float96" "Rationnal"]
for i in 0 .. lst.size
    for j in 0 .. lst.size
        c := "a == b"
        if i > j  c = "a == $(lst[i])(b)"
        if i < j  c = "$(lst[j])(a) == b"
        print "inline bool equal( const $(lst[i]) &a, const $(lst[j]) &b ) { return $c; }"
    */  
    inline bool equal( const Int8 &a, const Int8 &b ) { return a == b; }
    inline bool equal( const Int8 &a, const Unsigned8 &b ) { return Unsigned8(a) == b; }
    inline bool equal( const Int8 &a, const Int16 &b ) { return Int16(a) == b; }
    inline bool equal( const Int8 &a, const Unsigned16 &b ) { return Unsigned16(a) == b; }
    inline bool equal( const Int8 &a, const Int32 &b ) { return Int32(a) == b; }
    inline bool equal( const Int8 &a, const Unsigned32 &b ) { return Unsigned32(a) == b; }
    inline bool equal( const Int8 &a, const Int64 &b ) { return Int64(a) == b; }
    inline bool equal( const Int8 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Int8 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Int8 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Int8 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Int8 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Unsigned8 &a, const Int8 &b ) { return a == Unsigned8(b); }
    inline bool equal( const Unsigned8 &a, const Unsigned8 &b ) { return a == b; }
    inline bool equal( const Unsigned8 &a, const Int16 &b ) { return Int16(a) == b; }
    inline bool equal( const Unsigned8 &a, const Unsigned16 &b ) { return Unsigned16(a) == b; }
    inline bool equal( const Unsigned8 &a, const Int32 &b ) { return Int32(a) == b; }
    inline bool equal( const Unsigned8 &a, const Unsigned32 &b ) { return Unsigned32(a) == b; }
    inline bool equal( const Unsigned8 &a, const Int64 &b ) { return Int64(a) == b; }
    inline bool equal( const Unsigned8 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Unsigned8 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Unsigned8 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Unsigned8 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Unsigned8 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Int16 &a, const Int8 &b ) { return a == Int16(b); }
    inline bool equal( const Int16 &a, const Unsigned8 &b ) { return a == Int16(b); }
    inline bool equal( const Int16 &a, const Int16 &b ) { return a == b; }
    inline bool equal( const Int16 &a, const Unsigned16 &b ) { return Unsigned16(a) == b; }
    inline bool equal( const Int16 &a, const Int32 &b ) { return Int32(a) == b; }
    inline bool equal( const Int16 &a, const Unsigned32 &b ) { return Unsigned32(a) == b; }
    inline bool equal( const Int16 &a, const Int64 &b ) { return Int64(a) == b; }
    inline bool equal( const Int16 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Int16 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Int16 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Int16 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Int16 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Unsigned16 &a, const Int8 &b ) { return a == Unsigned16(b); }
    inline bool equal( const Unsigned16 &a, const Unsigned8 &b ) { return a == Unsigned16(b); }
    inline bool equal( const Unsigned16 &a, const Int16 &b ) { return a == Unsigned16(b); }
    inline bool equal( const Unsigned16 &a, const Unsigned16 &b ) { return a == b; }
    inline bool equal( const Unsigned16 &a, const Int32 &b ) { return Int32(a) == b; }
    inline bool equal( const Unsigned16 &a, const Unsigned32 &b ) { return Unsigned32(a) == b; }
    inline bool equal( const Unsigned16 &a, const Int64 &b ) { return Int64(a) == b; }
    inline bool equal( const Unsigned16 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Unsigned16 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Unsigned16 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Unsigned16 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Unsigned16 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Int32 &a, const Int8 &b ) { return a == Int32(b); }
    inline bool equal( const Int32 &a, const Unsigned8 &b ) { return a == Int32(b); }
    inline bool equal( const Int32 &a, const Int16 &b ) { return a == Int32(b); }
    inline bool equal( const Int32 &a, const Unsigned16 &b ) { return a == Int32(b); }
    inline bool equal( const Int32 &a, const Int32 &b ) { return a == b; }
    inline bool equal( const Int32 &a, const Unsigned32 &b ) { return Unsigned32(a) == b; }
    inline bool equal( const Int32 &a, const Int64 &b ) { return Int64(a) == b; }
    inline bool equal( const Int32 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Int32 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Int32 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Int32 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Int32 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Unsigned32 &a, const Int8 &b ) { return a == Unsigned32(b); }
    inline bool equal( const Unsigned32 &a, const Unsigned8 &b ) { return a == Unsigned32(b); }
    inline bool equal( const Unsigned32 &a, const Int16 &b ) { return a == Unsigned32(b); }
    inline bool equal( const Unsigned32 &a, const Unsigned16 &b ) { return a == Unsigned32(b); }
    inline bool equal( const Unsigned32 &a, const Int32 &b ) { return a == Unsigned32(b); }
    inline bool equal( const Unsigned32 &a, const Unsigned32 &b ) { return a == b; }
    inline bool equal( const Unsigned32 &a, const Int64 &b ) { return Int64(a) == b; }
    inline bool equal( const Unsigned32 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Unsigned32 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Unsigned32 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Unsigned32 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Unsigned32 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Int64 &a, const Int8 &b ) { return a == Int64(b); }
    inline bool equal( const Int64 &a, const Unsigned8 &b ) { return a == Int64(b); }
    inline bool equal( const Int64 &a, const Int16 &b ) { return a == Int64(b); }
    inline bool equal( const Int64 &a, const Unsigned16 &b ) { return a == Int64(b); }
    inline bool equal( const Int64 &a, const Int32 &b ) { return a == Int64(b); }
    inline bool equal( const Int64 &a, const Unsigned32 &b ) { return a == Int64(b); }
    inline bool equal( const Int64 &a, const Int64 &b ) { return a == b; }
    inline bool equal( const Int64 &a, const Unsigned64 &b ) { return Unsigned64(a) == b; }
    inline bool equal( const Int64 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Int64 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Int64 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Int64 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Unsigned64 &a, const Int8 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Unsigned8 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Int16 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Unsigned16 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Int32 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Unsigned32 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Int64 &b ) { return a == Unsigned64(b); }
    inline bool equal( const Unsigned64 &a, const Unsigned64 &b ) { return a == b; }
    inline bool equal( const Unsigned64 &a, const Float32 &b ) { return Float32(a) == b; }
    inline bool equal( const Unsigned64 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Unsigned64 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Unsigned64 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Float32 &a, const Int8 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Unsigned8 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Int16 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Unsigned16 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Int32 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Unsigned32 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Int64 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Unsigned64 &b ) { return a == Float32(b); }
    inline bool equal( const Float32 &a, const Float32 &b ) { return a == b; }
    inline bool equal( const Float32 &a, const Float64 &b ) { return Float64(a) == b; }
    inline bool equal( const Float32 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Float32 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Float64 &a, const Int8 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Unsigned8 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Int16 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Unsigned16 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Int32 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Unsigned32 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Int64 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Unsigned64 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Float32 &b ) { return a == Float64(b); }
    inline bool equal( const Float64 &a, const Float64 &b ) { return a == b; }
    inline bool equal( const Float64 &a, const Float96 &b ) { return Float96(a) == b; }
    inline bool equal( const Float64 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Float96 &a, const Int8 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Unsigned8 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Int16 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Unsigned16 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Int32 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Unsigned32 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Int64 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Unsigned64 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Float32 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Float64 &b ) { return a == Float96(b); }
    inline bool equal( const Float96 &a, const Float96 &b ) { return a == b; }
    inline bool equal( const Float96 &a, const Rationnal &b ) { return Rationnal(a) == b; }
    inline bool equal( const Rationnal &a, const Int8 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Unsigned8 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Int16 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Unsigned16 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Int32 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Unsigned32 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Int64 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Unsigned64 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Float32 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Float64 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Float96 &b ) { return a == Rationnal(b); }
    inline bool equal( const Rationnal &a, const Rationnal &b ) { return a == b; }
    
    template<class T> bool equal( const Bool &a, const T &b ) { return a == bool(b); }
    template<class T> bool equal( const T &a, const Bool &b ) { return bool(a) == b; }
    inline bool equal( const Bool &a, const Bool &b ) { return a == b; }
}       
        
        
// inline BigRat<> div_int( Unsigned8  a, Unsigned8  b ) { return BigRat<>( a, b ); }
// inline BigRat<> div_int( Unsigned16 a, Unsigned16 b ) { return BigRat<>( a, b ); }
// inline BigRat<> div_int( Unsigned32 a, Unsigned32 b ) { return BigRat<>( a, b ); }
// inline BigRat<> div_int( Unsigned64 a, Unsigned64 b ) { return BigRat<>( a, b ); }

template<class T> bool heaviside( T a ) { return a >= 0; }
inline bool heaviside( Bool a ) { return a; }

inline bool heaviside( Unsigned8  a ) { return true; }
inline bool heaviside( Unsigned16 a ) { return true; }
inline bool heaviside( Unsigned32 a ) { return true; }
inline bool heaviside( Unsigned64 a ) { return true; }

template<class T> bool eqz( T a ) { return not a; }

template<class T> T pos_part( T a ) { return a >= 0 ? a : T(0); }

template<class T> Int32 sgn( const T &val ) { return ( val>T(0) ? 1 : (val==T(0) ? 0 : -1 ) ); }
inline Int32 sgn( const Bool &val ) { return val; }

template<class T1,class T2> void init_arithmetic( T1 &self, const T2 &val ) { self = val; }

template<class T2> void init_arithmetic( Rationnal &self, const T2 &val ) { self.init(); self = val; }
    
inline Rationnal pow_96( const Rationnal &a, const Rationnal &b ) {
    if ( b.is_zero() ) return 1;
    if ( a.is_one () ) return a;
    if ( a.is_zero() ) return a;
    if ( b.is_integer() and b.num.n == 0 ) {
        if ( b.num.val == 1 ) return a;
        //
        int e = b.num.val;
        Rationnal tmp = a;
        if ( e < 0 )  {
            std::swap( tmp.num, tmp.den );
            e = -e;
        }
        Rationnal cp_a = tmp;
        while ( --e ) {
            tmp.num = tmp.num * cp_a.num;
            tmp.den = tmp.den * cp_a.den;
        }
        tmp.cannonicalize(); // necessary ?
        return tmp;
    }
    return pow( Float96(a), Float96(b) );
}

inline Rationnal atan2_96 ( const Rationnal &a, const Rationnal &b ) { return Rationnal( atan2( Float96(a), Float96(b) ) ); }

inline Rationnal sqrt_96  ( const Rationnal &a ) { return pow_96( a, Rationnal( 1, 2 ) ); }

inline Rationnal log_96   ( const Rationnal &a ) { assert( a.is_pos() ); return Rationnal( log ( Float96(a) ) ); }
inline Rationnal exp_96   ( const Rationnal &a ) { return Rationnal( exp ( Float96(a) ) ); }
inline Rationnal sin_96   ( const Rationnal &a ) { return Rationnal( sin ( Float96(a) ) ); }
inline Rationnal cos_96   ( const Rationnal &a ) { return Rationnal( cos ( Float96(a) ) ); }
inline Rationnal tan_96   ( const Rationnal &a ) { return Rationnal( tan ( Float96(a) ) ); }
inline Rationnal asin_96  ( const Rationnal &a ) { return Rationnal( asin( Float96(a) ) ); }
inline Rationnal acos_96  ( const Rationnal &a ) { return Rationnal( acos( Float96(a) ) ); }
inline Rationnal atan_96  ( const Rationnal &a ) { return Rationnal( atan( Float96(a) ) ); }
inline Rationnal sinh_96  ( const Rationnal &a ) { return Rationnal( sinh( Float96(a) ) ); }
inline Rationnal cosh_96  ( const Rationnal &a ) { return Rationnal( cosh( Float96(a) ) ); }
inline Rationnal tanh_96  ( const Rationnal &a ) { return Rationnal( tanh( Float96(a) ) ); }

#endif // CONFIG_H
