#ifndef INTERPRETER_BEHAVIOR_H
#define INTERPRETER_BEHAVIOR_H

#include "profentries.h"

struct InterpreterBehavior {
    InterpreterBehavior() {
        interpretation = false;
        debug = false;
        remake_all = false;
        verbose_level = 1;
    }
    
    bool debug, remake_all;
    int verbose_level;
    bool interpretation;
    
    ProfEntries prof_entries;
};


#endif // INTERPRETER_BEHAVIOR_H
