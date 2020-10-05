#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define I 100
#define J 200
#define K 300
#define MAX_VAL 10
#define ROOT 0
#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n))
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id)+1)-BLOCK_LOW(id))
unsigned** A;
unsigned** B;
int pid;
int group;
void initRoot();
void initWoker();
void init();

int main(int argc, char** argv) {
    double start, end;
    MPI_Init(&argc, &argv);
    start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &group);
    init();

    return 0;
}

void init() {
    void (*pinit)();
    pinit = initWoker;
    if(pid == ROOT)
        pinit = initRoot;
    (*pinit)();
}

void initRoot() {

}

void initWoker() {
    srand(1);
}