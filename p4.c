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
  int *sendcounts, *displs;
  float rec_buf[N*N];
  int a = 0;
  int b = 3;

  int *rows;


  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(i=0; i<N; i++){
    result[i]=0;
  }


  rows = malloc(sizeof(int)*N);
  displs = malloc(sizeof(int)*N);


  //MIRAR ESTOO
  
  float rem = N%numprocs;

  for (int i = 0; i < numprocs; i++) {
      rows[i] = N/numprocs;
      if (rem > 0) {
          rows[i]++;
          //printf("%d", rows[i]);
          rem--;
      }

      displs[i] = sum;
      sum += rows[i]*N;
      rows[i] = rows[i]*N;
  } 



  //Esto coge y asigna N/numprocs filas a cada proceso en rows [rank],
    //y mete el remanente (rem) ordenadamente desde la primera

  if (rank == 0){

    for(i=0;i<N;i++) {
      vector[i] = i;
      for(j=0;j<N;j++) {
        matrix[i][j] = i+j;
        printf("matrix[%d][%d] = %.f\n", i,j,matrix[i][j]);
      } 
    }
  }

  if (0 == rank) {
      for (int i = 0; i < numprocs; i++) {
          printf("rows[%d]*N = %d\tdispls[%d] = %d\n", i, rows[i], i, displs[i]);
      }
  }

  MPI_Scatterv(&matrix, rows, displs, MPI_FLOAT, &rec_buf, N*rows[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&vector, N, MPI_FLOAT, 0, MPI_COMM_WORLD);


  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < rows[rank]; i++) {
    printf("rank: %d, received: %.f\n", rank,rec_buf[i]);

  //  result[rank] += rec_buf[i];
  //  printf("rank = %d, result = %f\n",rank, result[rank]);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  for(i=0; i<N;i++){
    //printf("vector= %.f\n", vector[i]);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  for (int i = 0; i < (rows[rank]/N); i++){
    for(j=i*N; j<(i+1)*N; j++){
      //printf("rank:%d, i:%d, j:%d\n",rank, i, j);
      result[i] += rec_buf[j] * vector[j-(i*N)];

      //printf("rank[%d], %f * %f, result[%d]: %f\n", rank, rec_buf[j], vector[j-(i*N)],  i, result[i]);
    }
  }



  for(i=0; i<rows[rank]; i++){
    printf("rank = %d, RESULT[%d] = %f\n",rank, i, result[i]);
  }

  //printf("\n");
  MPI_Barrier(MPI_COMM_WORLD);


  //MPI_Gatherv(&result, numprocs, MPI_FLOAT, resultAux, &b, &a, MPI_FLOAT, 0, MPI_COMM_WORLD);

  //MPI_Reduce(&result, &resultAux, numprocs, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

  //MPI_Barrier(MPI_COMM_WORLD);
  //if (rank == 0){
  //  for (int i = 0; i < numprocs; i++)
  //    {
  //      //printf("result[%d]: %f\n",i, resultAux[i]);
  //    }
  //}

  MPI_Finalize();
  free(rows);
  free(displs);


  return 0;
}
