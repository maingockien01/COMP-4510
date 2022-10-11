#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_MATRIX_SIZE 100

int main (int argc, char *argv[]) {
    int my_rank, comm_size;
    int *scount, *displs, offset;
    int *gather_displs;

    int matrix_size = DEFAULT_MATRIX_SIZE;
    int *A;
    int *b;
    int *a_row;
    int *c;
    int *row_counts;
    int *sub_c;

    srand(1);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    row_counts = (int *) malloc(sizeof(int) * comm_size);
    scount = (int *) malloc(sizeof(int) * comm_size);
    displs = (int *) malloc(sizeof(int) * comm_size);
    gather_displs = (int *) malloc(sizeof(int) * comm_size);

    if (my_rank == 0) {
        //init matrix b
        A = (int *) malloc(sizeof(int) * matrix_size * matrix_size);
        b = (int *) malloc(sizeof(int) * matrix_size);
        c = (int *) malloc(sizeof(int) * matrix_size);

        for (int i = 0; i < matrix_size * matrix_size; i ++) {
            A[i] = rand();
        }
        for (int i = 0; i < matrix_size; i ++) {
            b[i] = rand();
        }
        
    }

    MPI_Request bcast_request;

    MPI_Ibcast(b, matrix_size, MPI_INI, 0, MPI_COMM_WORLD, &bcast_request);

    //Partition matrix A - rows (conceptually)
    for (int i = 0; i < comm_size; i ++) {
        row_counts[i] = matrix_size / comm_size;
    }

    int remaining_rows = matrix_size % comm_size;
    
    for (int i = 0; i < remaining_rows; i ++) {
        row_counts[i]++;
    }

    //Partition matrix A - entries (actually)
    for (int i = 0; i < comm_size; i ++) {
        scount[i] = row_counts[i] * matrix_size;
    }

    //Calculate displ for scatter
    offset = 0;
    for (int i = 0; i < comm_size; i ++) {
        displs[i] = offset;
        offset += scount[i];
    }

    //Calculate displ for gatherv
    offset = 0;
    for (int i = 0; i < comm_size; i ++) {
        gather_displs[i] = offset;
        offset += row_counts[i];
    }


    //distribute data
    a_row = (int *) malloc(sizeof(int) * scount[my_rank]);

    MPI_Scatterv(A, scount, displs, MPI_INT,
                a_row, scount[my_rank], MPI_INT,
                0, MPI_COMM_WORLD);

    //wait for bcast - make sure process receive b
    MPI_Wait(&bcast_request, MPI_STATUS_IGNORE);

    //work on computation    
    sub_c = (int *) malloc(sizeof(int) * row_counts[my_rank]);

    for (int i = 0; i < row_counts[my_rank]; i ++) {
        sub_c[i] = 0;
    }

    for (int i = 0; i < row_counts[my_rank]; i ++) {
        for (int j = 0; j < matrix_size; j ++) {
            sub_c[i] += b[j] * a_row[i * matrix_size + j];
        }
    }


    //Gather data
    MPI_Gatherv(sub_c, row_counts[my_rank], MPI_INT,
                c, row_counts, gather_displs, MPI_INT,
                0, MPI_COMM_WORLD);
    
    if (my_rank == 0) {
        printf("Matrix size is %d. Number of processes is %d.\n", matrix_size, comm_size);
        printf("")
    }

    MPI_Finalize();
 
    return EXIT_SUCCESS;
}