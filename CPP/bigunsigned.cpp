//
// C++ Implementation: bigunsigned
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "bigunsigned.h"
#include <iostream>
#include <stdlib.h>
#include <limits>

BigUnsigned::BigUnsigned() {
    i.next = 0;
    i.v = 0;
}

BigUnsigned::BigUnsigned(const BigUnsigned &b) {
    //
    i.v = b.i.v;
    if ( b.i.next ) {
        i.next = (Item *)malloc( sizeof(Item) );
        for(Item *t=b.i.next, *n=i.next; ; t=t->next, n=n->next ) {
            n->v = t->v;
            if ( t->next )
                n->next = (Item *)malloc( sizeof(Item) );
            else {
                n->next = 0;
                break;
            }
        }
    }
    else
        i.next = NULL;
}

BigUnsigned::~BigUnsigned() {
    for(Item *t=i.next;t;) {
        Item *o = t;
        t = t->next;
        free(o);
    }
}

void self_add_partial(unsigned val,BigUnsigned::Item *e) {
    bool ret = ( val > std::numeric_limits<unsigned>::max() - e->v );
    e->v += val;
    if ( ret ) {
        if ( e->next ) {
            for(BigUnsigned::Item *t=e->next;;t=t->next) {
                ret = ( t->v == std::numeric_limits<unsigned>::max() );
                t->v += 1;
                if ( ret==false )
                    break;
                if ( not t->next ) {
                    t->next = (BigUnsigned::Item*)malloc( sizeof(BigUnsigned::Item) );
                    t->next->v = 1;
                    t->next->next = NULL;
                    break;
                }
            }
        }
        else {
            e->next = (BigUnsigned::Item*)malloc( sizeof(BigUnsigned::Item) );
            e->next->v = 1;
            e->next->next = NULL;
        }
    }
}

void self_add_v1_inf_v2(const BigUnsigned &v1,BigUnsigned &res) {
    self_add_partial( v1.i.v, &res.i );
    for(BigUnsigned::Item *t=v1.i.next,*n=res.i.next;t;t=t->next,n=n->next)
        self_add_partial( t->v, n );
}

void BigUnsigned::operator*=(unsigned coeff) {
    BigUnsigned tmp( *this );
    while ( --coeff ) self_add_v1_inf_v2( tmp, *this );
}

void BigUnsigned::operator+=(unsigned val) {
    self_add_partial( val, &i );
}

long double BigUnsigned::to_double() const {
    long double res = i.v, base = 1;
    for(Item *t=i.next;t;t=t->next) {
        base *= std::numeric_limits<unsigned>::max();
        res += t->v * base;
    }    
    return res;
}

unsigned BigUnsigned::nb_items() const {
    unsigned res = 1;
    for(Item *t=i.next;t;t=t->next,++res);
    return res;
}


