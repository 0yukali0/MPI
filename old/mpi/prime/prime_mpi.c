#include <stdio.h>
#include <time.h>
#include <stdbool.h>
//#include "mpi.h"
#define SIZE 1001
#define ROOT 0
#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id)+1)-BLOCK_LOW(id))
#define BLOCK_OWNER(index,p,n) (((p)*((index)+1)-1)/(n))
int number[SIZE];
int total[SIZE];
void mark(unsigned long index);
void printPrime();

int main(int argc, char** argv) {
    int pid, group;
    double start, end;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &group);
    unsigned long index, limit;
    for(index = 0;index < SIZE;index++) {
        number[index] = 1;
	total[index] = 1;
    }
    number[1] = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    index = BLOCK_LOW(pid, group, SIZE-1);
    if(index < 3)
        index = 3;
    if(index & 1 == 0)
        index++;
    limit = BLOCK_HIGH(pid, group, SIZE-1);
    for(;index <= limit;index+=2) {
        mark(index);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(pid == group - 1)
    	MPI_Bcast(&index, 1, MPI_UNSIGNED_LONG, group-1, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    for(;index < SIZE;index+=2)
        if(BLOCK_OWNER(index,group,SIZE-1) == pid)
            mark(index); 
    MPI_Reduce(number, total, SIZE, MPI_INT, MPI_MIN, ROOT, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(pid == ROOT)
        printPrime();;
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    printf("%d:time:%lf\n", pid, (end-start)/CLOCKS_PER_SEC);
    MPI_Finalize();
    return 0;
}

void mark(unsigned long index) {
    unsigned long test = index * index;
    unsigned long twice = index << 1;
    for(;test < SIZE;test += twice)
            number[test] = 0;
}

void printPrime() {
    unsigned long long count = 0;
    unsigned long long index;
    for(index = 3;index < SIZE;index+=2)
        if(total[index] == 1)
            count++;
    count++;
    printf("***Total count***\ncount: %llu\n",count);
}
