#ifndef BOOL_H
#define BOOL_H

#include "size_alignement.h"
#include <limits>
// #include "config.h"

struct Bool {
    Bool( char *v, char m ) { val = v; mask = m; }
    
    template<class T> operator T() const {
        return ( *val & mask ? 1 : 0 );
    }
    
    template<class T> void operator=(const T &b) {
        if ( bool(b) )
            *val |= mask;
        else 
            *val &= ~mask;
    }
    
    void operator=(const Bool &b) {
        if ( bool(b) )
            *val |= mask;
        else 
            *val &= ~mask;
    }
    int operator-() const {
        return - bool( *this );
    }
    
    char *val;
    char mask;
};

inline bool operator==(Bool a,Bool b) { return bool(a) == bool(b); }
inline bool operator< (Bool a,Bool b) { return bool(a) <  bool(b); }
inline bool operator> (Bool a,Bool b) { return bool(a) >  bool(b); }
inline bool operator<=(Bool a,Bool b) { return bool(a) <= bool(b); }
inline bool operator>=(Bool a,Bool b) { return bool(a) >= bool(b); }
inline bool operator!=(Bool a,Bool b) { return bool(a) != bool(b); }

inline bool operator+(Bool a,Bool b) { return bool(a) + bool(b); }
inline bool operator-(Bool a,Bool b) { return bool(a) + bool(b); }
inline bool operator*(Bool a,Bool b) { return bool(a) * bool(b); }
inline bool operator/(Bool a,Bool b) { return bool(a) / bool(b); }
inline bool operator%(Bool a,Bool b) { return bool(a) % bool(b); }

template<class T> bool operator==(Bool a,const T &b) { return bool(a) == b; }
template<class T> bool operator==(const T &a,Bool b) { return a == bool(b); }

template<class T> bool operator!=(Bool a,const T &b) { return bool(a) != b; }
template<class T> bool operator!=(const T &a,Bool b) { return a != bool(b); }


inline std::ostream &operator<<(std::ostream &os,Bool b) {
    os << bool( *b.val & b.mask );
    return os;
}

template<class T1,class T2> bool are_different(const T1 &a,const T2 &b) { return a != b; } /// done only to avoid "warning: comparison between signed and unsigned"


template<> struct GetSizeInBitsOf<Bool> { static const unsigned res = 1; };
template<> struct NeededAlignementOf<Bool> { static const unsigned res = 1; };

namespace std {
    template<> struct numeric_limits<Bool> {
        static bool max() { return true; }
        static bool min() { return false; }
    };
}

#endif // BOOL_H
