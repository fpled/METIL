#ifndef TYPE_CONFIG_H
#define TYPE_CONFIG_H

#include <limits>
#include <sys/types.h>

typedef float Float32;
typedef double Float64;
typedef long double Float96;

typedef __int8_t  Int8;
typedef __int16_t Int16;
typedef __int32_t Int32;
typedef __int64_t Int64;

typedef __uint8_t  Unsigned8;
typedef __uint16_t Unsigned16;
typedef __uint32_t Unsigned32;
typedef __uint64_t Unsigned64;

template<int N> struct SizeTypeChoice {};
template<> struct SizeTypeChoice<4> { typedef Int32 T; };
template<> struct SizeTypeChoice<8> { typedef Int64 T; };

typedef SizeTypeChoice<sizeof(void *)>::T SizeType;

// struct Type;
// typedef Type *TypePtr;

#define FLOAT64_MIN_VAL (-std::numeric_limits<Float64>::max())
#define FLOAT64_MAX_VAL (std::numeric_limits<Float64>::max())

#define INT32_MIN_VAL (-std::numeric_limits<Int32>::max())
#define INT32_MAX_VAL (std::numeric_limits<Int32>::max())


/*
for i in [8 16 32 64]
    for u in [8 16 32 64]
        if u >= i
            print "inline bool are_different(Int$i a,Unsigned$u b) { return b > (Unsigned$u)std::numeric_limits<Int$i>::max() or a != (Int$i)b; } /// done only to avoid 'warning: comparison between signed and unsigned'"
            print "inline bool are_different(Unsigned$u a,Int$i b) { return a > (Unsigned$u)std::numeric_limits<Int$i>::max() or (Int$i)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'"
        else
            print "inline bool are_different(Int$i a,Unsigned$u b) { return a != (Int$i)b; } /// done only to avoid 'warning: comparison between signed and unsigned'"
            print "inline bool are_different(Unsigned$u a,Int$i b) { return (Int$i)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'"
*/
inline bool are_different(Int8 a,Unsigned8 b) { return b > (Unsigned8)std::numeric_limits<Int8>::max() or a != (Int8)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned8 a,Int8 b) { return a > (Unsigned8)std::numeric_limits<Int8>::max() or (Int8)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int8 a,Unsigned16 b) { return b > (Unsigned16)std::numeric_limits<Int8>::max() or a != (Int8)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned16 a,Int8 b) { return a > (Unsigned16)std::numeric_limits<Int8>::max() or (Int8)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int8 a,Unsigned32 b) { return b > (Unsigned32)std::numeric_limits<Int8>::max() or a != (Int8)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned32 a,Int8 b) { return a > (Unsigned32)std::numeric_limits<Int8>::max() or (Int8)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int8 a,Unsigned64 b) { return b > (Unsigned64)std::numeric_limits<Int8>::max() or a != (Int8)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned64 a,Int8 b) { return a > (Unsigned64)std::numeric_limits<Int8>::max() or (Int8)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int16 a,Unsigned8 b) { return a != (Int16)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned8 a,Int16 b) { return (Int16)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int16 a,Unsigned16 b) { return b > (Unsigned16)std::numeric_limits<Int16>::max() or a != (Int16)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned16 a,Int16 b) { return a > (Unsigned16)std::numeric_limits<Int16>::max() or (Int16)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int16 a,Unsigned32 b) { return b > (Unsigned32)std::numeric_limits<Int16>::max() or a != (Int16)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned32 a,Int16 b) { return a > (Unsigned32)std::numeric_limits<Int16>::max() or (Int16)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int16 a,Unsigned64 b) { return b > (Unsigned64)std::numeric_limits<Int16>::max() or a != (Int16)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned64 a,Int16 b) { return a > (Unsigned64)std::numeric_limits<Int16>::max() or (Int16)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int32 a,Unsigned8 b) { return a != (Int32)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned8 a,Int32 b) { return (Int32)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int32 a,Unsigned16 b) { return a != (Int32)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned16 a,Int32 b) { return (Int32)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int32 a,Unsigned32 b) { return b > (Unsigned32)std::numeric_limits<Int32>::max() or a != (Int32)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned32 a,Int32 b) { return a > (Unsigned32)std::numeric_limits<Int32>::max() or (Int32)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int32 a,Unsigned64 b) { return b > (Unsigned64)std::numeric_limits<Int32>::max() or a != (Int32)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned64 a,Int32 b) { return a > (Unsigned64)std::numeric_limits<Int32>::max() or (Int32)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int64 a,Unsigned8 b) { return a != (Int64)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned8 a,Int64 b) { return (Int64)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int64 a,Unsigned16 b) { return a != (Int64)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned16 a,Int64 b) { return (Int64)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int64 a,Unsigned32 b) { return a != (Int64)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned32 a,Int64 b) { return (Int64)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Int64 a,Unsigned64 b) { return b > (Unsigned64)std::numeric_limits<Int64>::max() or a != (Int64)b; } /// done only to avoid 'warning: comparison between signed and unsigned'
inline bool are_different(Unsigned64 a,Int64 b) { return a > (Unsigned64)std::numeric_limits<Int64>::max() or (Int64)a != b; } /// done only to avoid 'warning: comparison between signed and unsigned'

#endif // TYPE_CONFIG_H
