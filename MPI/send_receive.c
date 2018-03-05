// This program implements the most basic version
// of send recevie using MPI. 

// Process with rank 0 sends a number to process with rank 1.

#include<mpi.h>
#include<stdio.h>

int main(){
        MPI_Init(NULL,NULL);

        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD,&world_size);

        int world_rank;
        MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);

        int number;
        if(world_rank == 0){ //Sender
                number = -1;
                MPI_Send(&number,1,MPI_INT,1,0,MPI_COMM_WORLD);
        }
        else if(world_rank == 1){ // Receiver
                MPI_Recv(&number,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                printf("Number received %d from process 0\n",number);
        }

        MPI_Finalize();

}
