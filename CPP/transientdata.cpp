#include "transientdata.h"
#include "variable.h"
#include "thread.h"

void TransientData::destroy( Thread *th ) {
//     for( Variable *v = variables; v; v=v->prev_named )
//         th->push_var_in_variables_to_del_at_next_end_block_if_to_be_destroyed( v );
}

void reassign_transient_data( Thread *th, TransientData *dest, TransientData *src, Type *type_ret ) {
//     if ( dest )
//        dest->destroy( th );
//     if ( src and dest ) {
//         
//     }
}
