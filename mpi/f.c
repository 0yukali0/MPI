#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SIZE 100
#define MAX_LEN 10
#define INFIN -1
int** graph;
int* graphSize;
unsigned long long nodeNumber;
void graphInit();
void shortPath();

int main(void) {
    double start = clock(), end;
    graphInit();
    shortPath();
    end = clock();
    printf("time %lf\n", (end - start) / CLOCKS_PER_SEC);
    unsigned i,j;
    for(i = 0;i < SIZE;i++) 
        for(j = 0;j < SIZE;j++)
            if(j == SIZE-1)
                printf("%d\n", graph[i][j]);
            else 
                printf("%d ", graph[i][j]); 
    return 0;
}

void graphInit() {
    srand(time(NULL));
    nodeNumber = SIZE;
    graphSize = (int*)malloc(nodeNumber * nodeNumber * sizeof(int));
    graph = (int**)malloc(nodeNumber * sizeof(int*));
    unsigned long long index;
    for(index = 0;index < nodeNumber;index++)
        graph[index] = &graphSize[index * nodeNumber];
    unsigned long long i, j;
    for(i = 0;i < SIZE;i++) {
        for(j = 0;j < SIZE;j++) {
            if(i == j) {
                graph[i][j] = 0;
                continue;
            }
            unsigned long long len;
            len = (rand() % MAX_LEN) + 1;
            if(len < 5)
                graph[i][j] = (int)(((len << 3) % MAX_LEN) + 1);
            else
                graph[i][j] = INFIN;
        }
    }
}

void shortPath() {
	unsigned long long k, i,j;
    for(k = 0;k < SIZE;k++)
        for(i = 0;i < SIZE;i++)
            for(j = 0;j < SIZE;j++) {
                unsigned long long temp, orin;
                temp = graph[i][k] + graph[k][j];
                orin = graph[i][j];
                if(orin == INFIN) {
                    graph[i][j] = (int)temp;
                    continue;
                }
                if(orin >= temp)
                    graph[i][j] = (int)temp;
            }

}