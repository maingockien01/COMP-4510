#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_WORK_REQ 100

int main (int argc, char *argv[]) {
    int N = NUM_WORK_REQ; //NUM_WORK_REQ
    int q; //processes
    int my_rank;
    int *paper_marks, *scount, *displs, offset, *recvbuf;
    int minPapersPerMarker;
    int sub_total;
    int total, average, *sub_totals;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &q);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    scount = (int *) malloc(sizeof(int) * N);
    displs = (int *) malloc(sizeof(int) * N);
    offset = 0;
    minPapersPerMarker = N/q;

    if (my_rank == 0) {
        //init data
        srand(time(NULL));
        paper_marks = (int *) malloc(sizeof(int) * N);
    
        //randomly fill up marks
        for (int i = 0; i < N; i ++) {
            paper_marks[i] = rand() % 101;
        }

        sub_totals = (int *) malloc(sizeof(int) * q);
    }

    //Calculate distribution

    for (int i = 0; i < N; i ++) {
        scount[i] = minPapersPerMarker;
    }

    int remainingPaper = N%q;
    for (int i = 0; i < remainingPaper; i ++) {
        scount[i]++;
    }

    for (int i = 0; i < N; i ++) {
        displs[i] = offset;
        offset += scount[i];
    }

    recvbuf = (int *) malloc(sizeof(int) * scount[my_rank]);

    //distribute data
    MPI_Scatterv(paper_marks, scount, displs, MPI_INT,
                recvbuf, scount[my_rank], MPI_INT,
                0, MPI_COMM_WORLD);

    //work on computation
    sub_total = 0;
    for (int i = 0; i < scount[my_rank]; i ++) {
        sub_total += recvbuf[i];
    }

    MPI_Gather(&sub_total, 1, MPI_INT, 
                sub_totals, 1, MPI_INT,
                0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        total = 0;
        for (int i = 0; i < q; i ++) {
            total += sub_totals[i];
        }

        average = total / N;
    }

    MPI_Bcast(&average, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("I am process %d with %d number of exam papers. The average is %d for total of %d papers.\n", my_rank, scount[my_rank], average, N);

    MPI_Finalize();
 

    return EXIT_SUCCESS;
}