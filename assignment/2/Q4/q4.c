#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[]) {
    int my_rank;
    int comm_size;
    int recv_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int next_process = (my_rank+1) % comm_size;
    int prev_process = (my_rank-1+comm_size) % comm_size;

    MPI_Request send_request;
    MPI_Request recv_request;
    MPI_Isend(&my_rank, 1, MPI_INT, next_process, 0, MPI_COMM_WORLD, &send_request);
    MPI_Irecv(&recv_rank, 1, MPI_INT, prev_process, 0, MPI_COMM_WORLD, &recv_request);

    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

    printf("I, process %d, received a rank from my neighbouring process %d.\n", my_rank, recv_rank);

    MPI_Finalize();
 

    return EXIT_SUCCESS;
}