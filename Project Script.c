#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
///// Note that if we want to compare the algorithms then we have to delete all the displays with printf because printf() takes a lot of time to execute.

///// all the algorithms are explaned in details in the rapport


int N=100; /// N is the number of element in array (message)
// let's mention that these variables are initialized at this level because we will use them Max the time.
int rank,size,nbr_iteration;
MPI_Status status;


int LOG2(int a) // to calculate the number of steps
{
    int ans=0;
    while(a>1)
    {
        ans++;
        a/=2;
    }
    return ans;
}
void print_message(int *A)  // we have to change %d to %f in the case that the type if our message is double.
{
    for (int i=0;i<N;i++)
        printf("%d ",A[i]);
    printf("\n");
}

int relative(int rank, int root, int size) // to calculate the relative rank 
{
    int relative_rank;
    if (rank>=root) 
        relative_rank=rank-root;
    else 
        relative_rank=rank-root+size;
    return relative_rank;
}

int relative_inverse(int relative_rank, int root, int size) // to calculate the inverse of the relative rank.
{
    int rank;
    if (relative_rank + root<size)
        rank=relative_rank + root;
    else 
        rank=relative_rank + root - size;
    return rank;
}
void MPI_Binomial_Tree(void *buf, int cnt, MPI_Datatype dat, int root, MPI_Comm comm)
{
    
    nbr_iteration=LOG2(size); /// number of iteration needed to send the message for Max process
    int relative_rank=relative(rank,root,size); // calculate the relative rank 
    if (relative_rank==0) /// (rank==root)
    {
        void *message=buf;
        printf("I'm process rank : %d \n",rank);
        //find all the distination of the relative rank which is relative_destination
        for (int i = 0;i <= nbr_iteration ;i++) 
        {
            /// for each iteration rank 0 send the message for Max process with rank_dest=pow(2,i)+rank. where i is the ith iteration. 
            int relative_destination=(1<<i) + relative_rank;  /// In my program I used (1<<i) which is pow(2,i) ( faster than pow(2,i) ).
            if (relative_destination>=size) break;
            // find the real destination which is the inverse of the relative destination
            int destination=relative_inverse(relative_destination,root,size);
            MPI_Send(message, cnt, dat, destination , 0, comm);  
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination);
            print_message(message);
        }
    }
    else 
    {
        void *message=(void *)malloc(N*sizeof(dat));
        int relative_source=relative_rank-(1<< LOG2(relative_rank)); // find the source of the relative rank which is th relative_source
        int source=relative_inverse(relative_source,root,size); // find the real source which is the inverse of the relative_source
        MPI_Recv(message, cnt, dat, source, 0, comm, &status );
        printf("I'm process rank : %d \n",rank);
        /// each node receive the message from rank-pow(2,log2(rank)) and the send it to its list */
        printf("Rank %d: Received the message from rank %d with values : ",rank,source);
        print_message(message);
        for (int i=LOG2(relative_rank) + 1 ;i <= nbr_iteration; i++)
        {
            int relative_destination=(1<<i) + relative_rank;
            if (relative_destination>=size) break;  /// destination must be between 0 and size-1;
            int destination=relative_inverse(relative_destination,root,size);
            MPI_Send(message, cnt, dat, destination, 0, comm);
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination);
            print_message(message);
        }
    }
}

void MPI_Binary_Tree(void *buf, int cnt, MPI_Datatype dat, int root, MPI_Comm comm)
{
    
    int relative_rank=relative(rank,root,size);  
    if (relative_rank==0) /// (rank==root)
    {
        void *message=buf;
        printf("I'm process rank : %d \n",rank);
        int relative_destination1=2*relative_rank+1,relative_destination2=2*relative_rank+2;
        if (relative_destination1<size)
        {
            int destination1=relative_inverse(relative_destination1,root,size);
            MPI_Send(message, cnt, dat, destination1 , 0, comm);  
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination1);
            print_message(message);
        }
        if (relative_destination2<size)
        {
            int destination2=relative_inverse(relative_destination2,root,size);
            MPI_Send(message, cnt, dat, destination2 , 0, comm);  
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination2);
            print_message(message);
        }
    }
    else 
    {
        void *message=(void *)malloc(N*sizeof(dat));
        int relative_source=(relative_rank - 1) /2;
        int source=relative_inverse(relative_source,root,size);
        MPI_Recv(message, cnt, dat, source, 0, comm, &status );
        printf("I'm process rank : %d \n",rank);
        /// each node receive the message from (rank-1)/2 and the send it to the two nodes 2*rank+1 ( left child) and 2*rank+2 (right child)
        printf("Rank %d: Received the message from rank %d with values : ",rank,source);
        print_message(message);
        int relative_destination1= 2 * relative_rank + 1  ,  relative_destination2= 2 * relative_rank + 2;
        if (relative_destination1<size)
        {
            int destination1=relative_inverse(relative_destination1,root,size);
            MPI_Send(message, cnt, dat, destination1 , 0, comm);  
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination1);
            print_message(message);
        }
        if (relative_destination2<size)
        {
            int destination2=relative_inverse(relative_destination2,root,size);
            MPI_Send(message, cnt, dat, destination2 , 0, comm);  
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination2);
            print_message(message);
        }
    }
}

void MPI_Kchain(void *buf, int cnt, MPI_Datatype dat, int root, MPI_Comm comm, int k)
{
    
    int relative_rank=relative(rank,root,size);
    if (relative_rank==0) /// (rank==root)
    {
        void *message=buf;
        printf("I'm process rank : %d \n",rank);
        /// first of Max, the root send the message to k node which are {root+1, root+2, .. root +k }
        for (int i=1;i<=k;i++)
        {
            int relative_destination=i;
            if (relative_destination<size)
            {
                int destination=relative_inverse(relative_destination,root,size);
                MPI_Send(message, cnt, dat, destination , 0, comm);  
                printf("Rank %d : sended the message to rank %d with values : ",rank,destination);
                print_message(message);   
            }
            else break;
        }
    }
    else 
    {
        void *message=(void *)malloc(N*sizeof(dat));
        int relative_source;
        // if rank<=k then the source is 0
        if (relative_rank<=k) 
            relative_source=0;
        else 
            relative_source=relative_rank-k;
        int source=relative_inverse(relative_source,root,size);
        MPI_Recv(message, cnt, dat, source, 0, comm, &status );
        printf("I'm process rank : %d \n",rank);
        /// each node receive the message from rank-k and the send it to the node rank+k
        printf("Rank %d: Received the message from rank %d with values : ",rank,source);
        print_message(message);
        // each node send the message to the node with rank : rank + k
        int relative_destination=relative_rank + k;
        if (relative_destination<size)
        {
            int destination=relative_inverse(relative_destination,root,size);
            MPI_Send(message, cnt, dat, destination , 0, comm);  
            printf("Rank %d : sended the message to rank %d with values : ",rank,destination);
            print_message(message);
        }

    }
}

int main(int argc, char** argv)
{

    /////// Basic MPI Initializations
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    /// prepare the message : in this test the message is an array of integer
    int *buf=(int *)malloc(N*sizeof(int));   /// N number of elements in the array
    for (int i=0;i<N;i++) 
        buf[i]=i;
    int root=4,k=20;
    double time_Bcast,time_BinomialTree,time_Kchain,time_BinaryTree ,Max_time_Bcast,Max_time_BinomialTree,Max_time_Kchain,Max_time_BinaryTree;

    MPI_Barrier(MPI_COMM_WORLD); //Synchronizes all processes of the communicator
    time_BinomialTree -=MPI_Wtime(); 
    MPI_Binomial_Tree(buf,N,MPI_INT,root,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD); // Synchronizes all processes of the communicator
    time_BinomialTree += MPI_Wtime();
    // in other ways, time_BinomialTree= end_time-start_time
    MPI_Reduce(&time_BinomialTree, &Max_time_BinomialTree, 1, MPI_DOUBLE,MPI_SUM, root, MPI_COMM_WORLD); // calculate the sum of the time for all processes 
                                                                                                        // we will divide it by size (number of processes)
                                                                                                        // to obtain the average time spended by all the processes


    MPI_Barrier(MPI_COMM_WORLD);
    time_Bcast -=MPI_Wtime();
    MPI_Bcast (buf, N,MPI_INT , root, MPI_COMM_WORLD );
    MPI_Barrier(MPI_COMM_WORLD);
    time_Bcast += MPI_Wtime();
    MPI_Reduce(&time_Bcast, &Max_time_Bcast, 1, MPI_DOUBLE,MPI_SUM, root, MPI_COMM_WORLD);


    MPI_Barrier(MPI_COMM_WORLD);
    time_BinaryTree -=MPI_Wtime();
    MPI_Binary_Tree(buf,N,MPI_INT,root, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    time_BinaryTree += MPI_Wtime();
    MPI_Reduce(&time_BinaryTree, &Max_time_BinaryTree, 1, MPI_DOUBLE,MPI_SUM, root, MPI_COMM_WORLD);


    MPI_Barrier(MPI_COMM_WORLD);
    time_Kchain -=MPI_Wtime();
    MPI_Kchain(buf, N, MPI_INT, root,MPI_COMM_WORLD, k);  
    MPI_Barrier(MPI_COMM_WORLD);
    time_Kchain += MPI_Wtime();
    
    MPI_Reduce(&time_Kchain, &Max_time_Kchain, 1, MPI_DOUBLE,MPI_SUM, root, MPI_COMM_WORLD);
    // the time is printed in millisecond
    if (rank==root) {
        printf("Binomial Tree time en ms = %f\n", 1000*(Max_time_BinomialTree/size)) ; // millisecond 
        printf("Standard Broadcast time  en ms= %f\n", 1000*(Max_time_Bcast/size)) ;
        printf("Binary Tree time en ms = %f\n",1000*(Max_time_BinaryTree /size)) ;
        printf("K-chain time en ms = %f\n",1000*(Max_time_Kchain/size )) ;
    }
    MPI_Finalize ();

    return (0);
}
