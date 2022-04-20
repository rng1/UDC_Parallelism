#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>


int MPI_FlattreeBcast(void *sendbuf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int i;
    int numprocs, rank;
    int totalcount = 0;
    int recAux;

    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank != root){

        MPI_Send(sendbuf, 1, MPI_INT, 0, 0 , comm); 

    }else{

        for (int i = 1; i < numprocs; ++i)

        {
            MPI_Recv(&recAux, 1, MPI_INT, i, MPI_ANY_TAG, comm, &status); 
            
                totalcount += recAux;
        }
    }
    return totalcount;
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

    for (i = 1; i <= ceil(log2(numprocs)); i++)
        if (rank < ipow(2, i-1))
        {
            rec = rank + ipow(2, i-1);
            if (rec < numprocs)
            {
                err = MPI_Send(buf, count, datatype, rec, 0, comm);
                if (err != MPI_SUCCESS) return err;
            }
        } else
        {
            if (rank < ipow(2, i))
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
    int i, done = 0, n, count, totalc;
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

    totalc = MPI_FlattreeBcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Reduce(&pi, &piAux, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0){
        if(totalc < 0){
            printf("There was an error");
        }
        else{
            totalc = totalc + count;
            pi = ((double) totalc/(double) n)*4.0;
            printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
        
    }
        
    MPI_Finalize();
}
