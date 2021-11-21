#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#define SIZE 10
#define MAX_LEN 10
#define INFIN 10000
#define ROOT 0
#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n))
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id)+1,p,n)-BLOCK_LOW(id,p,n))
#define BLOCK_OWNER(index,p,n) (((p)*((index)+1)-1)/(n))
/*
typedef struct part {
    struct part* next;
    unsigned index;
} part;
part* header = NULL;
(void)(*otherJob)();
*/
unsigned** graph;
unsigned* graphSize;
unsigned nodeNumber;
unsigned pid_start, pid_end, pid_size;
int pid;
int group;
void graphInit();
void shortPath();
void printAnswer();
void printLocal();
void SR();
/*
void ohter_R();
void ohter_SR();
*/

int main(int argc, char** argv) {
    double start, end; 
    MPI_Init(&argc, &argv);
    start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &group);
    graphInit();
    MPI_Barrier(MPI_COMM_WORLD);
    if(pid == ROOT) {
        MPI_Bcast(&graphSize[0], SIZE*SIZE, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
        printAnswer();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    shortPath();
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    printf("pid:%d compute x:%u~%u for time:%.6lf",pid, pid_start, pid_end-1, ((end-start)/CLOCKS_PER_SEC));
    printLocal();
    MPI_Barrier(MPI_COMM_WORLD);
    if(pid == ROOT)
        printAnswer();
    free(graphSize);
    MPI_Finalize();
    return 0;
}

void printAnswer() {
    unsigned i, j;
    for(i = 0;i < SIZE;i++) {
        printf("\n%u:",i);
        for(j = 0;j < SIZE;j++) {
            unsigned temp;
            temp = graph[i][j];
            if(temp == INFIN)
                printf("IN ");
            else
                printf("%-2u ", temp);
        }
    }
    printf("\n\n");
}

void printLocal() {
    unsigned i, j;
    for(i = pid_start;i < pid_end;i++) {
        printf("\n%u:",i);
        for(j = 0;j < SIZE;j++) {
            unsigned temp;
            temp = graph[i][j];
            if(temp == INFIN)
                printf("IN ");
            else
                printf("%-2u ", temp);
        }
    }
    putchar('\n');
}

void graphInit() {
    srand(1);
    //menmory alloate
    nodeNumber = SIZE;
    graphSize = (unsigned*)malloc(nodeNumber * nodeNumber * sizeof(unsigned));
    graph = (unsigned**)malloc(nodeNumber * sizeof(unsigned*));
    unsigned index;
    for(index = 0;index < nodeNumber;index++)
        graph[index] = &graphSize[index * nodeNumber];
    unsigned i, j, len;
    //graph init
    for(i = 0;i < SIZE;i++) {
        for(j = 0;j < SIZE;j++) {
            if(i == j) {
                graph[i][j] = 0;
                continue;
            }
            len = rand();
            if(len & 1)
                graph[i][j] = (unsigned)((len % MAX_LEN)+1);
            else
                graph[i][j] = INFIN;
        }
    }
    //workpart define
    pid_size = BLOCK_SIZE(pid, group, nodeNumber);
    pid_start = BLOCK_LOW(pid, group, nodeNumber);
    pid_end = BLOCK_HIGH(pid, group, nodeNumber);
    /*
    for(i = BLOCK_SIZE(group, group, nodeNumber);i < SIZE;i++) {
        if(pid == BLOCK_OWNER(i,group,SIZE))
            continue;
        if(header == NULL) {
            header = (part*)malloc(sizeof(part));
            header->next = NULL;
            header->index = i;
        } else {
            part* ptr;
            for(ptr = header;ptr->next != NULL;ptr = ptr->next);
            ptr->next = (part*)malloc(sizeof(part));
            ptr = ptr->next;
            ptr->index = i;
        }
    }
    */
}

void shortPath() {
    unsigned  k, i, j, ik, temp;
    for(k = 0;k < SIZE;k++) {
        if(k) SR();
        for(i = pid_start;i < pid_end;i++) {
            ik = graph[i][k];
            for(j = 0;j < SIZE;j++) {
                temp = ik + graph[k][j];
                if(graph[i][j] >= temp)
                    graph[i][j] = (unsigned)temp;
            }
        }
        /*
        if(header == NULL) continue;
        part* ptr;
        ptr = header;
        for(i = ptr->index;ptr != NULL;ptr = ptr->next) {
            ik = graph[i][k];
            for(j = 0;j < SIZE;j++) {
                temp = ik + graph[k][j];
                if(graph[i][j] >= temp)
                    graph[i][j] = (unsigned)temp;
            }
        }
        */
    }
}

void SR() {
    MPI_Status status;
    unsigned rec;
    for(rec = 0;rec < group;rec++) {
        /*
        part* ptr;
        ptr = header;
        unsigned i;
        for(i = ptr->index;ptr != NULL;ptr = ptr->next)
            MPI_Send(graph[i], SIZE, MPI_UNSIGNED, (int)rec, 0, MPI_COMM_WORLD);
            */
        MPI_Send(&graphSize[pid_start*nodeNumber], (int)pid_size, MPI_UNSIGNED, (int)rec, pid_start, MPI_COMM_WORLD);
    }
    /*
    unsigned start;
    for(start = 0;start < SIZE;start+=size)
        MPI_Recv(graph[start], (int)size, MPI_UNSIGNED, MPI_ANY_SOURCE, start, MPI_COMM_WORLD, &status);
    */
    MPI_Recv(&graphSize[pid_start*nodeNumber], (int)pid_size, MPI_UNSIGNED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
}

