#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>


int MPI_FlattreeColective(void *sendbuff, void *recbuff, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int err;
    int numprocs, rank, value;

    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    
    if (rank == root){
        value = ((int*)sendbuff)[0];  //puts in value the initial value of sendbuff
        for (int k = 0; k < numprocs; k++){   //loop  <numprocs
            if (k != root){    //Dont receive in the root process
                err = MPI_Recv(sendbuff, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (err != MPI_SUCCESS) return err;   //throws error
                value += ((int*)sendbuff)[0];  //continues summing the values received
            }
            
        }
    } else
    {
        MPI_Send(sendbuff, 1, MPI_INT, root, rank, MPI_COMM_WORLD);
    }
       
    ((int*)recbuff)[0] = value;   //pass the total value to the recbuff
    return MPI_SUCCESS;
}


int ipow(int base, int exp)
{
    int res = 1;
    for (;;)
    {
        if (exp & 1)
            res *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return res;
}

int MPI_BinomialBcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    int err, i;
    int numprocs, rank;
    int rec, send;

    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (i = 1; i <= ceil(log2(numprocs)); i++)   //repeats as many floors the tree has
        if (rank < ipow(2, i-1))        //process < 2^k-1
        {
            rec = rank + ipow(2, i-1);    //chooses the receiver (the next following 2^k-1)
            if (rec < numprocs)     //the rec has to be less than numprocs
            {
                err = MPI_Send(buf, count, datatype, rec, 0, comm);
                if (err != MPI_SUCCESS) return err;
            }
        } else
        {
            if (rank < ipow(2, i))  //the rank does not pass the max values of that "floor"
            {
                send = rank - ipow(2, i-1);
                err = MPI_Recv(buf, count, datatype, send, 0, comm, &status);
                if (err != MPI_SUCCESS) return err;
            }
        }
    return MPI_SUCCESS;
}



int main(int argc, char *argv[])
{
    int i, done = 0, n, count, countAux;
    double PI25DT = 3.141592653589793238462643;
    double pi, x, y, z, piAux;

    int numprocs, rank;

    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);



    if (rank == 0){

        printf("Enter the number of points: (0 breaks) ");
        scanf("%d",&n);

    }

    //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_BinomialBcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (n == 0) return 0;

    count = 0;  

    for (i = rank; i < n; i+=numprocs) {


       srand(i); 
       x = ((double) rand()) / ((double) RAND_MAX);
       y = ((double) rand()) / ((double) RAND_MAX);
       //printf("%f  %f   rank: %d\n", x,y,rank); 
       
       z = sqrt((x*x)+(y*y));

       
       if(z <= 1.0)
            count++;
    }

    MPI_FlattreeColective(&count, &countAux, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Reduce(&pi, &piAux, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0){
        pi = ((double) countAux/(double) n)*4.0;
        printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
    }
        
    MPI_Finalize();
}