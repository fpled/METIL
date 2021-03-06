#ifndef METIL_MPI_WRAP_H
#define METIL_MPI_WRAP_H

/** simple wrapper */

void metil_mpi_init( int argc, char **argv );
void metil_mpi_finalize();

int metil_mpi_size(); 
int metil_mpi_rank(); 

int metil_mpi_send( void *buf, int count, int dest, int tag );
int metil_mpi_recv( void *buf, int count, int dest, int tag );

#endif // METIL_MPI_WRAP_H

