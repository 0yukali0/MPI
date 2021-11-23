#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define SIZE 10000
#define VALUE 1.1

float* a = NULL;
float* b = NULL;
float* c = NULL;

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  double start, end;
  a = (float*)malloc(SIZE * SIZE * sizeof(float));
  b = (float*)malloc(SIZE * sizeof(float));
  c = (float*)malloc(SIZE * sizeof(float));

  int i, j;
  int aIndex;
  aIndex = i = 0;
  while(i < SIZE) {
    b[i] = VALUE;
    c[i] = 0;
    j = 0;
    while(j < SIZE) {
      a[aIndex] = VALUE;
      j++;
      aIndex++;
    }
    i++;
  }
  start = MPI_Wtime();
  i = 0;
  float* ptr = a;
  float* bptr;
  float* cptr = c;
  while(i < SIZE){
    j = 0;
    bptr = b;
    float sum = 0;
    while(j < SIZE) {
      sum += (*ptr)  * (*bptr);
      j++;
      ptr++;
      bptr++;
    }
    *cptr = sum;
    i++;
    cptr++;
  }

  end = MPI_Wtime();
  printf("result %f\n", c[0]);
  printf("%lf\n", end - start);
  MPI_Finalize();
  return 0;
}
