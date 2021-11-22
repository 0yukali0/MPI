#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define ROOT 0
#define PROBLEM_SIZE 10000
#define DEFAULT_VALUE 1.1

// Role function
void MasterInit();
void MasterReport(double start, double end);
void SlaveInit();
void SlaveReport(double start, double end);
void Work();

// Setting or search
void SetJobRange();

// Debug
void GetDisp();
void GetJobSize();
void GetSendBuf();

typedef struct jobDetail {
  unsigned size;
}job;

typedef struct command {
  job  context;
  void (*init)();
  void (*work)();
  void (*get)(double, double);
}command;

command role;
unsigned i, j, pid, group;
int* jobSize = NULL;
int* dispOrder = NULL;
float** A = NULL;
float* AMatrix = NULL;
float* BVector = NULL;
float* CVector = NULL;
float* revBuf = NULL;
float* sendBuf = NULL;

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &group);
  double startTime, endTime;
  startTime = MPI_Wtime();

  // defualt role is slave
  role.init = SlaveInit;
  role.work = Work;
  role.get = SlaveReport; 
  if(pid == ROOT) {
    role.init = MasterInit;
    role.get = MasterReport;
  }
  
  // work stage: init -> work -> report
  role.init();
  role.work();
  endTime = MPI_Wtime();
  role.get(startTime, endTime);
  MPI_Finalize();
  return 0;
}

void MasterInit() {
  // define process job size
  i = j = PROBLEM_SIZE;
  int slaveJobSize, masterJobSize;
  slaveJobSize = (int)(i / group) * j;
  masterJobSize = (int)(i % group) * j;
  if(masterJobSize == 0) {
    masterJobSize = slaveJobSize;
  }

  jobSize = (int*)malloc(group * sizeof(int));
  dispOrder = (int*)malloc(group * sizeof(int));
  jobSize[0] = masterJobSize;
  dispOrder[0] = 0;
  revBuf = (float*)malloc(masterJobSize * sizeof(float));
  int indexI = 1;
  while(indexI < group) {
    dispOrder[indexI] = dispOrder[indexI - 1] + jobSize[indexI - 1];
    jobSize[indexI] = slaveJobSize;
    indexI++;
  };

  // GetDisp();
  // GetJobSize();

  int size = i * j;
  AMatrix = (float*)malloc(size * sizeof(float));
  BVector = (float*)malloc(j * sizeof(float));
  indexI = 0;
  while(indexI < j) {
    BVector[indexI] = DEFAULT_VALUE;
    indexI++;
  }
  indexI = 0;
  while(indexI < size) {
    AMatrix[indexI] = DEFAULT_VALUE;
    indexI++;
  }

  // jobSize, j, dispOrder, BVector, revBuf sync
  MPI_Bcast(jobSize, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(&j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(dispOrder, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(BVector, j, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  MPI_Scatterv(AMatrix, jobSize, dispOrder, MPI_FLOAT, revBuf, jobSize[pid], MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  free(AMatrix);

  SetJobRange();
  return;
}

void Work(){
  int ILen = role.context.size;
  sendBuf = (float*)malloc(ILen * sizeof(float));
  int IIndex = 0;
  int matrixIndex = 0;
  while(IIndex < ILen) {
    float sum = 0;
    int JIndex = 0;
    while(JIndex < j) {
      sum += revBuf[matrixIndex] * BVector[JIndex];
      JIndex++;
      matrixIndex++;
    }
    sendBuf[IIndex] = sum;
    // printf("pid %d of %d, sum:%.2f v.s buf:%.2f\n", pid, IIndex, sum, sendBuf[IIndex]);
    IIndex++;
  }
  free(revBuf);
  return;
}
void MasterReport(double start, double end){
  CVector = (float*)malloc(j * sizeof(float));
  int IIndex = 0;
  while(IIndex < group) {
    jobSize[IIndex] /= j;
    IIndex++;
  }
  IIndex = 1;
  while(IIndex < group) {
    dispOrder[IIndex] = dispOrder[IIndex-1] + jobSize[IIndex-1];
    IIndex++;
  }
  // GetJobSize();
  // GetDisp();;
  MPI_Gatherv(sendBuf, role.context.size, MPI_FLOAT, CVector, jobSize, dispOrder, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  // printf("master checking...\n");
  free(BVector);
  free(jobSize);
  free(dispOrder);
  free(sendBuf);
  printf("time %lf\n", end - start);
  IIndex = 0;
  float isSame = CVector[0];
  printf("Result %.3f\n", isSame);
  while(IIndex < j) {
    if(isSame != CVector[IIndex]) printf("%d:%d Result dif %.3f\n", i, IIndex, CVector[IIndex]);
    IIndex++;
  }
  return;
}

void SlaveInit() {
  // jobSize, j ,dispOrder, BVector, revBuf sync
  jobSize = (int*)malloc(group * sizeof(int));
  dispOrder = (int*)malloc(group * sizeof(int));
  MPI_Bcast(jobSize, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(&j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(dispOrder, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  BVector = (float*)malloc(j * sizeof(float));
  MPI_Bcast(BVector, j, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  revBuf = (float*)malloc(jobSize[pid] * sizeof(float));
  MPI_Scatterv(NULL, jobSize, dispOrder, MPI_FLOAT, revBuf, jobSize[pid], MPI_FLOAT, ROOT, MPI_COMM_WORLD);

  SetJobRange();
  return;
}
void SlaveReport(double start, double end) {
  // printf("Slave return result\n");
  MPI_Gatherv(sendBuf, role.context.size, MPI_FLOAT, NULL, NULL, NULL, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  free(sendBuf);
  free(jobSize);
  free(dispOrder);
  free(BVector);
  // printf("slave end\n");
  return;
}

void SetJobRange() {
  job detail={
    .size = (jobSize[pid] / j)
  };
  role.context = detail;
  return;
}

void GetDisp() {
  int IIndex = 0;
  printf("\nDisp\n");
  while(IIndex < group) {
    printf("%d ", dispOrder[IIndex]);
    IIndex++;
  }
  printf("\n");
}

void GetJobSize() {
  int IIndex = 0;
  printf("\nJobSize\n");
  while(IIndex < group) {
    printf("%d ", jobSize[IIndex]);
    IIndex++;
  }
  printf("\n");
}

void GetSendBuf() {
  int size = role.context.size;
  int IIndex = 0;
  printf("\nSendBuf\n");
  while(IIndex < size) {
    printf("%d ", sendBuf[IIndex]);
    IIndex++;
  }
  printf("\n");
}
