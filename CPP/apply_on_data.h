#ifndef APPLY_ON_DATA
#define APPLY_ON_DATA

#include "globaldata.h"
#include "variable.h"
#include "config.h"

/*
types := split("Unsigned8 Unsigned16 Unsigned32 Unsigned64 Int8 Int16 Int32 Int64 Float32 Float64 Float96 Rationnal Bool")
for c in ( "", "const " )
    print "template<class FUNC,class SO> bool apply_on_data( Variable *v, const FUNC &func, $(c)SO &os ) {"
    for i in types
        d := "*reinterpret_cast<$i *>(v->data)"; if i == "Bool"  d = "Bool( v->data, 1 << (7-v->get_bit_offset()) )"
        print "    if ( v->type == global_data.$i ) { func( $d, os ); return true; }"
    print "    return false;"
    print "}"
    
    print "template<class FUNC,class SO> bool apply_on_data( Variable *v1, Variable *v2, const FUNC &func, $(c)SO &os ) {"
    for i in types
        print "    if ( v1->type == global_data.$i ) {"
        d1 := "*reinterpret_cast<$i *>(v1->data)"; if i == "Bool"  d1 = "Bool( v1->data, 1 << 7-v1->get_bit_offset() )"
        for j in types
            d2 := "*reinterpret_cast<$j *>(v2->data)"; if j == "Bool"  d2 = "Bool( v2->data, 1 << 7-v2->get_bit_offset() )"
            print "        if ( v2->type == global_data.$j ) { func( $d1, $d2, os ); return true; }"
        print "    }"
    print "    return false;"
    print "}"
    
*/              
template<class FUNC,class SO> bool apply_on_data( Variable *v, const FUNC &func, SO &os ) {
    if ( v->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned8 *>(v->data), os ); return true; }
    if ( v->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned16 *>(v->data), os ); return true; }
    if ( v->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned32 *>(v->data), os ); return true; }
    if ( v->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned64 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int8 ) {  func( *reinterpret_cast<Int8 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int16 ) {  func( *reinterpret_cast<Int16 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int32 ) {  func( *reinterpret_cast<Int32 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int64 ) {  func( *reinterpret_cast<Int64 *>(v->data), os ); return true; }
    if ( v->type == global_data.Float32 ) {  func( *reinterpret_cast<Float32 *>(v->data), os ); return true; }
    if ( v->type == global_data.Float64 ) {  func( *reinterpret_cast<Float64 *>(v->data), os ); return true; }
    if ( v->type == global_data.Float96 ) {  func( *reinterpret_cast<Float96 *>(v->data), os ); return true; }
    if ( v->type == global_data.Rationnal ) {  func( *reinterpret_cast<Rationnal *>(v->data), os ); return true; }
    if ( v->type == global_data.Bool ) { Bool b( v->data, 1 << (7-v->get_bit_offset()) ); func( b, os ); return true; }
    return false;
}
template<class FUNC,class SO> bool apply_on_data( Variable *v1, Variable *v2, const FUNC &func, SO &os ) {
    if ( v1->type == global_data.Unsigned8 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned8 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Unsigned16 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned16 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Unsigned32 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned32 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Unsigned64 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned64 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int8 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int8 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int16 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int16 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int32 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int32 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int64 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int64 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Float32 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Float32 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Float64 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Float64 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Float96 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Float96 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Rationnal ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Rationnal *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Bool ) {
        if ( v2->type == global_data.Unsigned8 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() );Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( b1, b2, os ); return true; }
    }
    return false;
}
template<class FUNC,class SO> bool apply_on_data( Variable *v, const FUNC &func, const SO &os ) {
    if ( v->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned8 *>(v->data), os ); return true; }
    if ( v->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned16 *>(v->data), os ); return true; }
    if ( v->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned32 *>(v->data), os ); return true; }
    if ( v->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned64 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int8 ) {  func( *reinterpret_cast<Int8 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int16 ) {  func( *reinterpret_cast<Int16 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int32 ) {  func( *reinterpret_cast<Int32 *>(v->data), os ); return true; }
    if ( v->type == global_data.Int64 ) {  func( *reinterpret_cast<Int64 *>(v->data), os ); return true; }
    if ( v->type == global_data.Float32 ) {  func( *reinterpret_cast<Float32 *>(v->data), os ); return true; }
    if ( v->type == global_data.Float64 ) {  func( *reinterpret_cast<Float64 *>(v->data), os ); return true; }
    if ( v->type == global_data.Float96 ) {  func( *reinterpret_cast<Float96 *>(v->data), os ); return true; }
    if ( v->type == global_data.Rationnal ) {  func( *reinterpret_cast<Rationnal *>(v->data), os ); return true; }
    if ( v->type == global_data.Bool ) { Bool b( v->data, 1 << (7-v->get_bit_offset()) ); func( b, os ); return true; }
    return false;
}
template<class FUNC,class SO> bool apply_on_data( Variable *v1, Variable *v2, const FUNC &func, const SO &os ) {
    if ( v1->type == global_data.Unsigned8 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned8 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned8 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Unsigned16 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned16 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned16 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Unsigned32 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned32 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned32 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Unsigned64 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Unsigned64 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Unsigned64 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int8 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int8 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int8 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int16 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int16 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int16 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int32 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int32 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int32 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Int64 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Int64 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Int64 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Float32 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Float32 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Float32 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Float64 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Float64 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Float64 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Float96 ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Float96 *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Float96 *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Rationnal ) {
        if ( v2->type == global_data.Unsigned8 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) {  func( *reinterpret_cast<Rationnal *>(v1->data), *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( *reinterpret_cast<Rationnal *>(v1->data), b2, os ); return true; }
    }
    if ( v1->type == global_data.Bool ) {
        if ( v2->type == global_data.Unsigned8 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned16 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned32 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Unsigned64 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Unsigned64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int8 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int8 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int16 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int16 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int32 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Int64 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Int64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float32 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Float32 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float64 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Float64 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Float96 ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Float96 *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Rationnal ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() ); func( b1, *reinterpret_cast<Rationnal *>(v2->data), os ); return true; }
        if ( v2->type == global_data.Bool ) { Bool b1( v1->data, 1 << 7-v1->get_bit_offset() );Bool b2( v2->data, 1 << 7-v2->get_bit_offset() ); func( b1, b2, os ); return true; }
    }
    return false;
}


struct Display {
    template<class T> void operator()( const T &data, std::ostream &os ) const { os << data; }
    void operator()( const Rationnal &data, std::ostream &os ) const { os << "Rationnal(" << data.num << "," << data.den << ")"; }
};

struct Assign {
    template<class T> void operator()( const T &data, Variable *ds ) const { init_arithmetic( *reinterpret_cast<T *>(ds->data), data ); }
};

struct GetT2 {
    template<class T,class T2> void operator()( const T &data, T2 &val ) const { val = (T2)data; }
};

struct AssignT2 {
    template<class T,class T2> void operator()( T &data, const T2 &val ) const { init_arithmetic( data, (T)val ); }
    template<class T2> void operator()( Bool &data, const T2 &val ) const { data = bool(val); }
};


template<class FUNC> struct AssignFunc {
    template<class T> void operator()( const T &data, Variable *ds ) const { apply_on_data( ds, AssignT2(), func( data ) ); }
    template<class T1,class T2> void operator()( const T1 &a, const T2 &b, Variable *ds ) const { apply_on_data( ds, AssignT2(), func( a, b ) ); }
    FUNC func;
};

template<int val > struct EqualTo {
    template<class T> void operator()(const T &data,bool &res) const { res = METIL_NS::equal( data, val ); }
};

struct Add       { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::add    ( a, b ) ); } };
struct Sub       { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::sub    ( a, b ) ); } };
struct Mul       { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::mul    ( a, b ) ); } };
struct Div       { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::div    ( a, b ) ); } };
struct Pow       { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::pow    ( a, b ) ); } };
struct Div_int   { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::div_int( a, b ) ); } };
struct Mod       { template<class T1,class T2> bool operator()(const T1 &a,const T2 &b,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::mod    ( a, b ) ); } };


struct Neg       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), -val           ); } };
struct Log       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), log( val )     ); } };
struct Heaviside { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), heaviside(val) ); } };
struct Eqz       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), eqz(val) ); } };
struct Sgn       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), sgn(val) ); } };
struct Exp       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), exp(val)       ); } };
struct Sin       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), sin(val)       ); } };
struct Cos       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), cos(val)       ); } };
struct Tan       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), tan(val)       ); } };
struct Asin      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), asin(val)      ); } };
struct Acos      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), acos(val)      ); } };
struct Atan      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), atan(val)      ); } };
struct Sinh      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), sinh(val)      ); } };
struct Cosh      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), cosh(val)      ); } };
struct Tanh      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), tanh(val)      ); } };
struct Abs       { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), METIL_NS::abs(val) ); } };
struct Cast      { template<class T> bool operator()(const T &val,Variable *ds) const { return apply_on_data( ds, AssignT2(), val            ); } };







#endif // APPLY_ON_DATA
