#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 10

int main (int argc, char *argv[]) {
    int my_rank;
    int comm_size;
    int recv_rank;
    
    int n = ARRAY_SIZE;
    int* arrace;

    int *recvbuf;

    double start, end[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    recvbuf = malloc(n * sizeof(int));


    if (my_rank == 0) {

        arrace = (int *) malloc(n * sizeof(int));
        for (int i = 0; i < n; i ++) {
            arrace[i] = i;
        }
        MPI_Barrier(MPI_COMM_WORLD); // Wait for process 0 ready to start
        //Start timer
        start = MPI_Wtime();
        //Start races
        MPI_Request request1;
        MPI_Request request2;

        MPI_Isend(arrace, n, MPI_INT, 1, 0, MPI_COMM_WORLD, &request1);
        MPI_Isend(arrace, n, MPI_INT, 2, 0, MPI_COMM_WORLD, &request2);

        //Wating for receive
        MPI_Request recv1;
        MPI_Request recv2;
        MPI_Irecv(recvbuf, n, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recv1);
        MPI_Irecv(recvbuf, n, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recv2);

        MPI_Status status1;
        MPI_Status status2;

        MPI_Wait(&request1, &status1); //First
        end[status1.MPI_SOURCE%2] = MPI_Wtime();
        MPI_Wait(&request1, &status1); //Second
        end[status1.MPI_SOURCE%2] = MPI_Wtime();

        MPI_Barrier(MPI_COMM_WORLD); // Wait for all processes end
        printf("q = %d\n", comm_size);
        printf("n = %d\n", n);
        double even_time = end[0]-start;
        double odd_time = end[1]-start;
        printf("Even team time:%f\n",even_time);
        printf("Odd team time:%f\n",odd_time);
        if (even_time < odd_time) {
            printf("Even team wins!\n");
        } else {
            printf("Odd team wins!\n");

        }

    } else {
        int prev_process = my_rank <=2 ? 0 : (my_rank-2+comm_size) % comm_size;
        int next_process = my_rank >= (comm_size-2) ? 0 : (my_rank+2) % comm_size;

        MPI_Barrier(MPI_COMM_WORLD); // Wait for all processes ready to start

        //Wait for receive
        MPI_Status status_recv;
        MPI_Recv(recvbuf, n, MPI_INT, prev_process, MPI_ANY_TAG, MPI_COMM_WORLD, &status_recv);
        //Send to next one
        MPI_Send(recvbuf, n, MPI_INT, next_process, 0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD); // Wait for all processes end

    }


    MPI_Finalize();
 

    return EXIT_SUCCESS;
}