//
// C++ Interface: lexemnumber
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LEXEMNUMBER_H
#define LEXEMNUMBER_H

#include "bigunsigned.h"
#include "math.h"
#include "config.h"

/**
	@author Grover <hugo@gronordi>
*/
class LexemNumber {
public:
    LexemNumber() { expo=0; recquired_prec=0; has_e = false; }

    Rationnal::BI v;
    int expo;
    unsigned recquired_prec;
    unsigned attributes;
    bool has_e;

    long double to_double() const { return double(v) * pow(Rationnal::BI(10),expo); }
};

#endif
