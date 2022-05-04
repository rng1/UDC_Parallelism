#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define DEBUG 1

#define N 5

int main(int argc, char *argv[] ) {

  int i, j, sum=0;
  float matrix[N][N];
  float vector[N];
  float result[N], resultAux[N];

  int numprocs, rank;
  int *displs;
  double rec_buf[N*N];

  int *rows;

  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


  if(numprocs>N){
    if(rank == 0){
      printf("Numprocs cant be more than %d\n", N);

    }
    MPI_Finalize();
    return 0;
  }

  for(i=0; i<N; i++){
    result[i]=0;
  }

  rows = malloc(sizeof(int)*N);
  displs = malloc(sizeof(int)*N);

  float rem = N%numprocs;

  for (int i = 0; i < numprocs; i++) {
      rows[i] = N/numprocs;
      if (rem > 0) {
          rows[i]++;
          rem--;
      }

      displs[i] = sum;
      sum += rows[i]*N;
      rows[i] = rows[i]*N;
  } 

  if (rank == 0){

    for(i=0;i<N;i++) {
      vector[i] = i;
      for(j=0;j<N;j++) {
        matrix[i][j] = i+j;
      } 
    }
  }

  MPI_Scatterv(&matrix, rows, displs, MPI_FLOAT, &rec_buf, N*rows[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&vector, N, MPI_FLOAT, 0, MPI_COMM_WORLD);

  for (int i = 0; i < (rows[rank]/N); i++){
    for(j=i*N; j<(i+1)*N; j++){
      result[i] += rec_buf[j] * vector[j-(i*N)];
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  for(i = 0; i < numprocs; i++){
    rows[i] = rows[i] / N;
    displs[i] = displs [i] / N;
  }

  MPI_Gatherv(&result, rows[rank], MPI_FLOAT, resultAux, rows, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  if(rank == 0){
    if (DEBUG){
      for(i=0;i<N;i++) {
        printf("ANSWER: %.f \n",resultAux[i]);
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();

  free(rows);
  free(displs);


  return 0;
}
