#include "mpi_wrap.h"
#include <mpi.h>

void metil_mpi_init( int argc, char **argv ) {
    MPI_Init(&argc, &argv);
}

int metil_mpi_size() {
    int my_rank;
    MPI_Comm_size( MPI_COMM_WORLD, &my_rank );
    return my_rank;
}

int metil_mpi_rank() {
    int my_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
    return my_rank;
}


int metil_mpi_send( void *buf, int count, int dest, int tag ) {
    return MPI_Send( buf, count, MPI_BYTE, dest, tag, MPI_COMM_WORLD );
}

int metil_mpi_recv( void *buf, int count, int src, int tag ) {
    MPI_Status stat;
    MPI_Recv( buf, count, MPI_BYTE, src, tag, MPI_COMM_WORLD, &stat );
    #ifdef MPICH_NAME
    return stat.count;
    #else
    return stat.st_length;
    #endif
}

void metil_mpi_finalize() {
    MPI_Finalize();
}

