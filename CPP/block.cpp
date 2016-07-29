//
// C++ Implementation: definition
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "block.h"
#include "thread.h"
#include "sourcefile.h"
#include "errorlist.h"

void Block::init( Variable *last_named_, const void *tok_, SourceFile *current_sourcefile_, Variable *self_, DefTrial *def_trial_during_creation_ ) {
    last_named = last_named_;
    tok = tok_;
    sourcefile = current_sourcefile_;
    self = self_;
    def_trial_during_creation = def_trial_during_creation_;
    names.init();
}

void Block::init( Block *b ) {
    last_named = b->last_named;
    tok = b->tok;
    sourcefile = b->sourcefile;
    self = b->self;
    def_trial_during_creation = b->def_trial_during_creation;
    names.init();
    names = b->names;
}
    
void Block::reassign( Block *b, Thread *th, Variable * &sp ) {
    std::cout << "reassign block" << std::endl;
}

const void *destroy( Block &d, Thread *th, Variable * &sp ) {
    d.names.destroy();
    return tok_end_def_block;
}

std::ostream &operator<<(std::ostream &os, const Block &b) {
    if ( b.sourcefile and b.sourcefile->sar_text() and b.tok ) {
        ErrorList::Provenance p( b.sourcefile->sar_text() + reinterpret_cast<const unsigned *>(b.tok)[1], b.sourcefile->provenance );
        os << p.provenance << ":" << p.line << ":" << p.col << ": (";
    }
    for(unsigned i=0;i<b.names.size();++i)
        os << " " << b.names[i];
    os << " )";
    return os;
}

