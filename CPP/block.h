//
// C++ Interface: definition
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BLOCK_H
#define BLOCK_H

#include "splittedvec.h"
#include "nstring.h"

struct Variable;
struct Thread;
struct SourceFile;
struct DefTrial;
/**
	@author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct Block {
    void init( Variable *last_named_, const void *tok_, SourceFile *current_sourcefile_, Variable *self_, DefTrial *def_trial_during_creation_ );
    void init( Block *b );
    void reassign( Block *b, Thread *th, Variable * &sp );
    Variable *last_named;
    const void *tok;
    SourceFile *sourcefile;
    Variable *self;
    DefTrial *def_trial_during_creation;
    SplittedVec<Nstring,4> names;
    bool want_tuple;
};

const void *destroy( Block &d, Thread *th, Variable * &sp );
std::ostream &operator<<(std::ostream &os, const Block &b);

#endif // BLOCK_H

