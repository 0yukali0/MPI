#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define DEFAULT_VALUE 1.1
#define DEFAULT_SIZE 3000
// time record
time_t startTime;
time_t outTime;
// matrix: matrix*vector=result
float** matrix;
float* vector;
float* result;

// matrix size
int size;

// function list
void init();

int main(int argc, char** argv) {
  // init matrix
  size = atoi(argv[1]);
  init();
  time(&startTime);
  // row-wise compute
  int i, j;
  i = 0;
  while(i < size) {
    float sum = 0;
    j = 0;
    while(j < size) {
      sum += matrix[i][j] * vector[j];
      j++;
    }
    result[i] = sum;
    i++;
  }
  time(&outTime);
  // show result
  i = 0;
  //printf("result\n");
  /*
  while(i < size) {
    printf("%.3f ", result[i]);
    i++;
  }
  */
  printf("\nExecution time is %d\n",outTime - startTime);
  return 0;
}

void init(){
  //printf("size is %d\n",size);
  int i, j;

  // row malloc
  matrix = (float**)malloc(size * sizeof(float*));

  // column malloc
  i = 0;
  while(i < size) {
    matrix[i] = (float*)malloc(size * sizeof(float));
    i++;
  }

  // set 1.1 to elements
  i = 0;
  while(i < size) {
    j = 0;
    while(j < size) {
      matrix[i][j] = DEFAULT_VALUE;
      //printf("%.2f ", matrix[i][j]);
      j++;
    }
    //printf("\n");
    i++;
  }

  // one column
  vector = (float*)malloc(size * sizeof(float));
  result = (float*)malloc(size * sizeof(float));
  j = 0;
  //printf("\n vector\n");
  while(j < size) {
    vector[j] = DEFAULT_VALUE;
    result[j] = DEFAULT_VALUE;
    //printf("%.2f ", result[j]);
    j++;
  }
  //printf("\n");

  return;
}
