#ifndef TRANSIENT_DATA_H
#define TRANSIENT_DATA_H

#include "thread.h"

struct TransientData {
    void init() { variables = NULL; }
    void destroy( Thread *th );
    
    Variable *variables;
};

void reassign_transient_data( Thread *th, TransientData *dest, TransientData *src, Type *type_ret = NULL );

#endif // TRANSIENT_DATA_H
