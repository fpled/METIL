#ifndef BIG_INT_H
#define BIG_INT_H

#include<iostream>
// #include<cstdlib>
// #include<fstream>
#include<iomanip>
// #include<cmath>
// #include"string.h"
// #include"time.h"

#include <sstream>
#include "typeconfig.h"
#include <math.h>
#include <stdlib.h>

/**
   unlimited int.
    \param owning -> true if delete after destruction is authorized
 */
template< int base = 10, class T = int, int offset = 0, bool owning = true >
struct BigInt {
    ///
    BigInt() {
        val = 0;
        n = 0;
        ext = NULL;
    }
    
    BigInt( Unsigned64 v ) { init( v ); }
    BigInt( Int64 v ) { init( v ); }
    BigInt( Unsigned32 v ) { init( v ); }
    BigInt( Int32 v ) { init( v ); }
    BigInt( Float32 v ) { init_f( v ); }
    BigInt( Float64 v ) { init_f( v ); }
    BigInt( Float96 v ) { init_f( v ); }
    
    ~BigInt() { destroy(); }
    
    template<class TT> void init( TT v = 0 ) {
        if ( v < 0 ) {
            init( -v );
            val = - val;
        }
        else {
            n = 0;
            TT tmp = v;
            while ( tmp >= base ) { tmp /= base; ++n; }
            if ( n ) {
                ext = (T *)malloc( sizeof(T) * n );
                unsigned n2 = n-1;
                while ( v >= base ) {
                    ext[n2--] = v % base + offset;
                    v /= base;
                }
            }
            else
                ext = NULL;
            val = v + offset;
        }
    }
    template<class TT> void init_f( TT v = 0 ) { /// version pour Float32, Float64 et Float96 qui est valable seulement si le type Float64 peut représenter le nombre base.
        if ( v < 0. ) {
            init_f( -v );
            val = - val;
        }
        else {
            n = 0;
            TT tmp = v;
            while ( tmp >= base ) { tmp /= base; ++n; }
            if ( n ) {
                ext = (T *)malloc( sizeof(T) * n );
                unsigned n2 = n-1;
                while ( v >= base ) {
                    Float64 tmp2 = fmod(v,base);
//                     std::cout << " tmp2 = " << std::setprecision(18) << tmp2 << std::endl;
//                     Float64 ie,fe;
//                     fe = modf(v,&ie);
//                     std::cout << " fe = " << std::setprecision(18) << fe << " ie = " << std::setprecision(18) << ie <<std::endl;
                    ext[n2--] = (T) (tmp2) + offset;
                    v = (v - tmp2)/base;
                }
            }
            else
                ext = NULL;
            val = (T) v + offset;
        }
    }


    void init( const BigInt &b ) {
        val = b.val;
        n = b.n;
        if ( b.n ) {
            ext = (T *)malloc( sizeof(T) * b.n );
            for(unsigned i=0;i<n;++i) ext[i] = b.ext[i];
        }
        else
            ext = NULL;
    }
    
    void destroy() {
        if ( owning and n )
            free( ext );
        n = 0;
    }
    
    BigInt( const BigInt &b ) {
        init( b );
    }
    
    void operator=(const BigInt &b) {
        if ( n )
            free(ext);
        val = b.val;
        n = b.n;
        if ( b.n ) {
            ext = (T *)malloc( sizeof(T) * b.n );
            for(unsigned i=0;i<n;++i) ext[i] = b.ext[i];
        }
    }
    void operator+=(const BigInt &b) { *this = *this + b; }
    void operator-=(const BigInt &b) { *this = *this - b; }
    void operator*=(const BigInt &b) { *this = *this * b; }
    void operator/=(const BigInt &b) { *this = *this / b; }
    void operator%=(const BigInt &b) { *this = *this % b; }
    
    BigInt<base,T,offset> operator-() const { /*basic_assert();*/ BigInt<base,T,offset> r = *this; r.val = -r.val; /*r.basic_assert();*/ return r; }
    
    template<int b2,class T2,int offset2,bool owning2> BigInt(const BigInt<b2,T2,offset2,owning2> &b) {
        n = 0; val = 0;
        BigInt m(b2);
        *this = BigInt(abs(b.val));
        for(unsigned i=0;i<b.n;++i)
            *this = *this * m + BigInt(b.ext[i]);
        if ( b.val < 0 )
            val = -val;
    }
    template<int b2,class T2,int offset2,bool owning2> BigInt &operator=(const BigInt<b2,T2,offset2,owning2> &b) {
        BigInt m(b2);
        *this = BigInt(abs(b.val));
        for(unsigned i=0;i<b.n;++i)
            *this = *this * m + BigInt(b.ext[i]);
        if ( b.val < 0 )
            val = -val;
        return *this;
    }
    
    void mul_by_base(unsigned nb=1) { /// this = this * base; Works only if owning
        if ( n==0 and val==0 ) return;
        
        T *oe = ext;
        ext = (T *)malloc( sizeof(T) * (n+nb) );
        if ( n ) {
            for(unsigned i=0;i<n;++i)
                ext[i] = oe[i];
            free(oe);
        }
        while ( nb-- )
          ext[n++] = offset;
    }

    bool operator<(const BigInt &b) const {
        if ( val < 0 ) {
            if ( b.val < 0 )
                return -*this > -b;
            return true;
        } else if ( b.val < 0 )
            return false;
        //
        if ( n < b.n ) return true;
        if ( n > b.n ) return false;
        // same n
        if ( n == 0 ) { return val < b.val; }
        if ( b.val < 0 )
            return false;
        //
        if ( val < b.val ) return true;
        if ( val > b.val ) return false;
        for(unsigned i=0;i<n-1;++i) {
            if ( ext[i] < b.ext[i] ) return true;
            if ( ext[i] > b.ext[i] ) return false;
        }
        return ( ext[n-1] < b.ext[n-1] );
    }
    bool operator<=(const BigInt &b) const {
        if ( val < 0 ) {
            if ( b.val < 0 )
                return -*this >= -b;
            return true;
        } else if ( b.val < 0 )
            return false;
        //
        if ( n < b.n ) return true;
        if ( n > b.n ) return false;
        // same n
        if ( n == 0 ) { return val <= b.val; }
        //
        if ( val < b.val ) return true;
        if ( val > b.val ) return false;
        for(unsigned i=0;i<n-1;++i) {
            if ( ext[i] < b.ext[i] ) return true;
            if ( ext[i] > b.ext[i] ) return false;
        }
        return ( ext[n-1] <= b.ext[n-1] );
    }
    bool operator==(const BigInt &b) const {
        if ( n!=b.n ) return false;
        if ( val != b.val ) return false;
        for(unsigned i=0;i<n;++i)
            if ( ext[i] != b.ext[i] )
                return false;
        return true;
    }
    bool operator!=(const BigInt &b) const { return not ( *this == b ); }
    bool operator> (const BigInt &b) const { return ( b <  *this ); }
    bool operator>=(const BigInt &b) const { return ( b <= *this ); }
    
    int to_int() const {
        int res = abs(val);
        for(unsigned i=0;i<n;++i) {
            res *= base;
            res += ext[i];
        }
        if ( val < 0 )
            return -res;
        return res;
    }
    
    bool is_odd() const {
        if ( n == 0 ) return val & 1;
        return ext[ n - 1 ] & 1;
    }
    bool is_even() const {
        if ( n == 0 ) return not ( val & 1 );
        return not ( ext[ n - 1 ] & 1 );
    }
    bool is_negative() const {
        return val < 0;
    }
    
    inline void basic_assert() const {
        //         assert( val < base );
        //         assert( val > -base );
        //         for(unsigned i=0;i<n;++i) assert( ext[i] < base );
        //         for(unsigned i=0;i<n;++i) assert( ext[i] >= 0 );
    }
    
    void div_by_base() { /// divide this by base
        if ( n ) {
            --n;
            if ( n == 0 )
                free( ext );
        }
        else
            val = 0;
    }
    void rm_zeroes_at_beginning() {
        if ( val or not n ) return;
        for(unsigned i=0;i<n;++i) {
            if ( ext[i] ) { // first non zero
                val = ext[i++];
                n -= i;
                for(unsigned j = 0; j<n ; ++i, ++j)
                    ext[j] = ext[i];
                if ( n==0 ) free( ext );
                return;
            }
        }
        // else
        n = 0;
        free( ext );
    }
    
    operator Float32() const { Float32 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Float64() const { Float64 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Float96() const { Float96 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    
    operator Int8 () const { Int8  res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Int16() const { Int16 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Int32() const { Int32 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Int64() const { Int64 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    
    operator Unsigned8 () const { Unsigned8  res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Unsigned16() const { Unsigned16 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Unsigned32() const { Unsigned32 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    operator Unsigned64() const { Unsigned64 res = abs(val); for(unsigned i=0;i<n;++i) { res *= base; res += ext[i]; } return ( val < 0 ? -res : res ); }
    
    operator bool() const { return val; }
    bool is_one() const { return n==0 and val==1; }
    bool is_two() const { return n==0 and val==2; }
    bool is_minus_one() const { return n==0 and val==-1; }
    
    T val;    /// first val ( high weight, the only one which can be < 0 )
    T *ext;   /// supplementary data ( if this > base )
    unsigned n; /// number if integers int ext
};

/** display in base 10 */
template<int base, class T, int offset, bool owning>
std::ostream &operator<<(std::ostream &os, const BigInt<base,T,offset,owning> &v ) {
    if ( v.n==0 )
        os << v.val;
    else if ( base == 10 ) {
        os << v.val;
        for(unsigned i=0;i<v.n;++i) os << v.ext[i];
    }
    else {
        BigInt<10> m( v );
        os << m;
    }
    return os;
}
template<int base, class T, int offset, bool owning>
BigInt<base,T,offset> operator-(const BigInt<base,T,offset,owning> &a,const BigInt<base,T,offset,owning> &b);

template<int base, class T, int offset, bool owning>
BigInt<base,T,offset> operator+(const BigInt<base,T,offset,owning> &a,const BigInt<base,T,offset,owning> &b) {
//     a.basic_assert();    b.basic_assert();    
    if ( b.val < 0 ) {
        if ( a.val < 0 )
            return - ( (-a) + (-b) );
        else
            return a - (-b);
    }
    else if ( a.val < 0 )
        return b - (-a);
        
    BigInt<base,T,offset> res;
    res.n = std::max( a.n, b.n ); // will grow if ret
    if ( res.n )
        res.ext = (T *)malloc( sizeof(T) * (res.n+1) );
    
    int ret = 0;
    if ( res.n ) {
        unsigned i;
        // ext
        for(i=1;i<=std::min( a.n, b.n );++i) {
            int v = a.ext[a.n-i] + b.ext[b.n-i] + ret;
            if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
            res.ext[res.n-i] = v;
        }
        // mixed ext and val
        if ( i<=a.n ) {
            int v = a.ext[a.n-i] + b.val + ret;
            if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
            res.ext[res.n-i] = v;
            while ( ++i <= a.n ) {
                v = a.ext[a.n-i] + ret;
                if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
                res.ext[res.n-i] = v;
            }
            v = a.val + ret;
            if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
            res.val = v;
        }
        else if ( i<=b.n ) {
            int v = b.ext[b.n-i] + a.val + ret;
            if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
            res.ext[res.n-i] = v;
            while ( ++i <= b.n ) {
                v = b.ext[b.n-i] + ret;
                if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
                res.ext[res.n-i] = v;
            }
            v = b.val + ret;
            if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
            res.val = v;
        }
        else {
            int v = a.val + b.val + ret;
            if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
            res.val = v;
        }
    }
    else {
        int v = a.val + b.val;
        if ( v >= base ) { ret = 1; v -= base; } else ret = 0;
        res.val = v;
    }
    
    if ( ret ) {
        if ( res.n == 0 )
            res.ext = (T *)malloc( sizeof(T) * 1 );
        for(unsigned i=1;i<=res.n;++i)
            res.ext[res.n-i+1] = res.ext[res.n-i];
        ++res.n;
        res.ext[0] = res.val;
        res.val = 1;
    }
//     res.basic_assert();    
    return res;
}

template<int base, class T, int offset, bool owning>
BigInt<base,T,offset> operator-(const BigInt<base,T,offset,owning> &a,const BigInt<base,T,offset,owning> &b) {
//     a.basic_assert();    
//     b.basic_assert();    
    
    if ( b.val < 0 ) {
        if ( a.val < 0 )
            return - ( (-a) - (-b) );
        else
            return a + (-b);
    }
    else if ( a.val < 0 )
        return -( (-a) + b );
    
    if ( a < b ) {
        BigInt<base,T,offset> res = b - a;
        res.val = - res.val;
        return res;
    }
    
    BigInt<base,T,offset> res( 0 );
    if ( a.n ) {
        res.ext = (T *)malloc( sizeof(T) * a.n );
        res.n = a.n;
    }
    // we can assume that a > b
    int ret = 0;
    unsigned i;
    for(i=1;i<=b.n;++i) {
        int v = a.ext[a.n-i] - b.ext[b.n-i] - ret;
        if ( v < 0 ) { ret = 1; v += base; } else ret = 0;
        res.ext[res.n-i] = v;
    }
    //
    if ( i <= a.n ) {
        int v = a.ext[a.n-i] - b.val - ret;
        if ( v < 0 ) { ret = 1; v += base; } else ret = 0;
        res.ext[res.n-i] = v;
    
        while ( ++i <= a.n ) {
            v = a.ext[a.n-i] - ret;
            if ( v < 0 ) { ret = 1; v += base; } else ret = 0;
            res.ext[res.n-i] = v;
        }
        
        v = a.val - ret;
        if ( v < 0 ) { ret = 1; v += base; } else ret = 0;
        res.val = v;
    }
    else {
        int v = a.val - b.val - ret;
        if ( v < 0 ) { ret = 1; v += base; } else ret = 0;
        res.val = v;
    }
    // elimination of zeros at the beginning
    res.rm_zeroes_at_beginning();
    
//     res.basic_assert();    
    return res;
}

template<int base, class T, int off, bool owning>
void partial_mac(BigInt<base,T,off> &res,const BigInt<base,T,off,owning> &a,int m,int offset) {
    int ret = 0;
    
    T *r = res.ext + res.n - offset;
    
    // a.ext
    for(unsigned i=1;i<=a.n;++i) {
        int v = a.ext[a.n-i] * m + *r + ret;
        if ( v >= base ) { ret = v / base; v %= base; } else ret = 0;
        *(r--) = v;
    }
    
    // a.val
    int v = abs(a.val) * m + *r + ret;
    if ( v >= base ) { ret = v / base; v %= base; } else ret = 0;
    *(r--) = v;
    
    // ret
    // *(r--) += ret;
    while ( ret ) {
        int v = *r + ret;
        if ( v >= base ) { ret = v / base; v %= base; } else ret = 0;
        *(r--) = v;
    }
}

template<int base, class T, int offset, bool owning>
BigInt<base,T,offset> operator*(BigInt<base,T,offset,owning> a,BigInt<base,T,offset,owning> b) {
//     a.basic_assert();    
//     b.basic_assert();    
    
    if ( a.n + b.n == 0 ) {
        int v = a.val * b.val;
        if ( abs(v) >= base ) {
            BigInt<base,T,offset,owning> res;
            res.val = v / base;
            res.n = 1;
            res.ext = (T *)malloc( sizeof(T) );
            res.ext[0] = abs(v) % base;
            return res;
        }
        // else
        BigInt<base,T,offset> res;
        res.val = v;
        return res;
    }
    bool sgn = ( a.val < 0 ) xor ( b.val < 0 );
    a.val = abs( a.val );
    b.val = abs( b.val );
    
    // we will have to cope with a.ext or b.ext
    BigInt<base,T,offset> res;
    res.n = a.n + b.n + 2; // 
    res.ext = (T *)malloc( sizeof(T *) * res.n );
    res.val = 0;
    for(unsigned i=0;i<res.n;++i) res.ext[i] = 0;
    
    for(unsigned i=1;i<=b.n;++i)
        partial_mac( res, a, b.ext[b.n-i], i );
    partial_mac( res, a, abs(b.val), b.n+1 );
    
    // elimination of zeros at the beginning
    res.rm_zeroes_at_beginning();
    
    if ( sgn )
        res.val = -res.val;
//     res.basic_assert();
    return res;
}
/* a is assumed to be < base */
template<int base, class T, int offset, bool owning>
BigInt<base,T,offset> mul_by_lonely_digit(int a,const BigInt<base,T,offset,owning> &b) {
    if ( b.n == 0 ) {
        int v = a * b.val;
        if ( v >= base ) {
            BigInt<base,T,offset> res;
            res.val = v / base;
            res.n = 1;
            res.ext = (T *)malloc( sizeof(T) );
            res.ext[0] = v % base;
            return res;
        }
        // else
        BigInt<base,T,offset> res;
        res.val = v;
        return res;
    }

    // we will have to cope with a.ext or b.ext
    BigInt<base,T,offset> res;
    res.n = b.n + 2;
    res.ext = (T *)malloc( sizeof(T *) * res.n );
    res.val = 0;
    for(unsigned i=0;i<res.n;++i) res.ext[i] = 0;

    partial_mac( res, b, a, 1 );

    // elimination of zeros at the beginning
    res.rm_zeroes_at_beginning();
    return res;
}


template<int base, class T, int offset>
BigInt<base,T,offset> operator/( BigInt<base,T,offset> a, BigInt<base,T,offset> b ) {
    BigInt<base,T,offset> res;
    // when n == 0 (no extension)
    if ( a.n == 0 and b.n == 0 ) { res.val = a.val / b.val; return res; }
    // < 0
    if ( a.val<0 or b.val<0 ) {
        bool sgn = ( a.val < 0 ) xor ( b.val < 0 );
        a.val = abs( a.val );
        b.val = abs( b.val );
        res = a / b;
        if ( sgn )
            res.val = -res.val;
        return res;
    }
    // Ex : 12 / 123
    if ( a < b ) return res;
    
    // as in school
    typedef BigInt<base,T,offset> B;
    while ( a.n > b.n + 1 ) {
        B tmp = b;
        int o = a.n - tmp.n - 1 + (tmp.val < a.val);
        tmp.mul_by_base( o );
        int mult_tot = 0;
        while ( a >= tmp ) {
            int m = a.val * ( a.n==tmp.n ? 1 : base ) / tmp.val / 2;
            a = a - ( m > 1 ? B(m) * tmp : tmp );
            mult_tot += ( m > 1 ? m : 1 );
        }
        B m = mult_tot; m.mul_by_base( o ); res = res + m;
    }
    while ( a >= b ) {
        int m = a.val * ( a.n==b.n ? 1 : base ) / b.val / 2;
        a = a - ( m > 1 ? B(m) * b : b );
        res = res + B( m > 1 ? m : 1 );
    }
    
    return res;
}

template<int base, class T, int offset>
BigInt<base,T,offset> operator%( BigInt<base, T, offset> a, BigInt<base, T, offset> b ) {
    if ( b.n==0 and b.val==0 ) {
        std::cerr << "Division by zero" << std::endl;
        return b;
    }
    
    // when n == 0 (no extension)
    if ( a.n == 0 and b.n == 0 ) { a.val %= b.val; if (a.val<0) a.val+= b.val; return a; }
    
    // < 0
    bool minus_sgn = ( a.val<0 ) xor ( b.val<0 );
    a.val = abs( a.val );
    b.val = abs( b.val );
    
    // Ex : 12 % 123
    if ( a < b ) {
        if ( minus_sgn ) a.val = -a.val;
        return a;
    }
    
    // as in school
    typedef BigInt<base,T,offset> B;
    while ( a.n > b.n + 1 ) {
        B tmp = b;
        tmp.mul_by_base( a.n - tmp.n - 1 + (tmp.val < a.val) );
        while ( a >= tmp ) {
            int m = a.val * ( a.n==tmp.n ? 1 : base ) / tmp.val / 2;
            a = a - ( m > 1 ? B(m) * tmp : tmp );
        }
    }
    while ( a >= b ) {
        int m = a.val * ( a.n==b.n ? 1 : base ) / b.val / 2;
        a = a - ( m > 1 ? B(m) * b : b );
    }
    
    if ( minus_sgn ) a = b - a;
    
    return a;
}

template<int base, class T, int offset, bool owning>
BigInt<base,T,offset> gcd( BigInt<base,T,offset,owning> a, BigInt<base,T,offset,owning> b ) {
    if ( b.val == 1 and b.n == 0 )
        return 0;
        
    BigInt<base,T,offset> old;
    while ( b ) {
        old = b;
        b = a % b;
        a = old;
    }
    return a;
}


#endif // BIG_INT_H

