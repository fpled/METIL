#include "mpi_wrap.h"
#include <assert.h>

void metil_mpi_init( int argc, char **argv ) {
}

int metil_mpi_size() {
    return 0;
}

int metil_mpi_rank() {
    return 0;
}


int metil_mpi_send( void *buf, int count, int dest, int tag ) {
    assert(0);
}

int metil_mpi_recv( void *buf, int count, int src, int tag ) {
    assert(0);
}

void metil_mpi_finalize() {
}

