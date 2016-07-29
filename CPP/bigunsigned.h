//
// C++ Interface: bigunsigned
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BIGUNSIGNED_H
#define BIGUNSIGNED_H

/**
	@author Grover <hugo@gronordi>
*/
class BigUnsigned {
public:
    BigUnsigned();
    BigUnsigned(const BigUnsigned &b);
    ~BigUnsigned();
        
    struct Item {
        unsigned v;
        Item *next;
    };
    Item i;

    void operator*=(unsigned coeff);
    void operator+=(unsigned val);
    long double to_double() const;
    unsigned nb_items() const;
};

#endif
