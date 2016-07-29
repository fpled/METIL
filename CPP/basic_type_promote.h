#ifndef BASIC_TYPE_PROMOTE_H
#define BASIC_TYPE_PROMOTE_H

template<class T1,class T2,class Op>
struct TypePromote { };

template<class T1,class Op>
struct TypePromote<T1,T1,Op> { typedef T1 T; };

struct Add       ;
struct Sub       ;
struct Mul       ;
struct Div       ;
struct Pow       ;
struct Div_int   ;
struct Mod       ;

                 ;
struct Neg       ;
struct Log       ;
struct Heaviside ;
struct Eqz       ;
struct Sgn       ;
struct Exp       ;
struct Sin       ;
struct Cos       ;
struct Tan       ;
struct Asin      ;
struct Acos      ;
struct Atan      ;
struct Sinh      ;
struct Cosh      ;
struct Tanh      ;
struct Abs       ;
struct Cast      ;


/*
types := ["Bool" "Int8" "Unsigned8" "Int16" "Unsigned16" "Int32" "Unsigned32" "Int64" "Unsigned64" "Float32" "Float64" "Float96" "Rationnal"]
for i in 0 .. types.size
    for j in 0 .. types.size
        print "template<class Op> struct TypePromote<$(types[i]),$(types[j]),Op> { typedef $(types[max(i,j)]) T; };"
*/

template<class Op> struct TypePromote<Bool,Bool,Op> { typedef Bool T; };
template<class Op> struct TypePromote<Bool,Int8,Op> { typedef Int8 T; };
template<class Op> struct TypePromote<Bool,Unsigned8,Op> { typedef Unsigned8 T; };
template<class Op> struct TypePromote<Bool,Int16,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Bool,Unsigned16,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Bool,Int32,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Bool,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Bool,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Bool,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Bool,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Bool,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Bool,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Bool,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Int8,Bool,Op> { typedef Int8 T; };
template<class Op> struct TypePromote<Int8,Int8,Op> { typedef Int8 T; };
template<class Op> struct TypePromote<Int8,Unsigned8,Op> { typedef Unsigned8 T; };
template<class Op> struct TypePromote<Int8,Int16,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Int8,Unsigned16,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Int8,Int32,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int8,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Int8,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int8,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Int8,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Int8,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Int8,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Int8,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Unsigned8,Bool,Op> { typedef Unsigned8 T; };
template<class Op> struct TypePromote<Unsigned8,Int8,Op> { typedef Unsigned8 T; };
template<class Op> struct TypePromote<Unsigned8,Unsigned8,Op> { typedef Unsigned8 T; };
template<class Op> struct TypePromote<Unsigned8,Int16,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Unsigned8,Unsigned16,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Unsigned8,Int32,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Unsigned8,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned8,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Unsigned8,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned8,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Unsigned8,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Unsigned8,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Unsigned8,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Int16,Bool,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Int16,Int8,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Int16,Unsigned8,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Int16,Int16,Op> { typedef Int16 T; };
template<class Op> struct TypePromote<Int16,Unsigned16,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Int16,Int32,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int16,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Int16,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int16,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Int16,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Int16,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Int16,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Int16,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Unsigned16,Bool,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Unsigned16,Int8,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Unsigned16,Unsigned8,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Unsigned16,Int16,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Unsigned16,Unsigned16,Op> { typedef Unsigned16 T; };
template<class Op> struct TypePromote<Unsigned16,Int32,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Unsigned16,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned16,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Unsigned16,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned16,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Unsigned16,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Unsigned16,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Unsigned16,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Int32,Bool,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int32,Int8,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int32,Unsigned8,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int32,Int16,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int32,Unsigned16,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int32,Int32,Op> { typedef Int32 T; };
template<class Op> struct TypePromote<Int32,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Int32,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int32,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Int32,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Int32,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Int32,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Int32,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Unsigned32,Bool,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Int8,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Unsigned8,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Int16,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Unsigned16,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Int32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Unsigned32,Op> { typedef Unsigned32 T; };
template<class Op> struct TypePromote<Unsigned32,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Unsigned32,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned32,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Unsigned32,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Unsigned32,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Unsigned32,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Int64,Bool,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Int8,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Unsigned8,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Int16,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Unsigned16,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Int32,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Unsigned32,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Int64,Op> { typedef Int64 T; };
template<class Op> struct TypePromote<Int64,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Int64,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Int64,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Int64,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Int64,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Unsigned64,Bool,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Int8,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Unsigned8,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Int16,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Unsigned16,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Int32,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Unsigned32,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Int64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Unsigned64,Op> { typedef Unsigned64 T; };
template<class Op> struct TypePromote<Unsigned64,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Unsigned64,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Unsigned64,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Unsigned64,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Float32,Bool,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Int8,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Unsigned8,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Int16,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Unsigned16,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Int32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Unsigned32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Int64,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Unsigned64,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Float32,Op> { typedef Float32 T; };
template<class Op> struct TypePromote<Float32,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float32,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float32,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Float64,Bool,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Int8,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Unsigned8,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Int16,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Unsigned16,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Int32,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Unsigned32,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Int64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Unsigned64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Float32,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Float64,Op> { typedef Float64 T; };
template<class Op> struct TypePromote<Float64,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float64,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Float96,Bool,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Int8,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Unsigned8,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Int16,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Unsigned16,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Int32,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Unsigned32,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Int64,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Unsigned64,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Float32,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Float64,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Float96,Op> { typedef Float96 T; };
template<class Op> struct TypePromote<Float96,Rationnal,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Bool,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Int8,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Unsigned8,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Int16,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Unsigned16,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Int32,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Unsigned32,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Int64,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Unsigned64,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Float32,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Float64,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Float96,Op> { typedef Rationnal T; };
template<class Op> struct TypePromote<Rationnal,Rationnal,Op> { typedef Rationnal T; };


#endif // BASIC_TYPE_PROMOTE_H

