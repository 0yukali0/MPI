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
void SetJobOrder();
void SetJobRange(unsigned length);
void SetDefaultA();
void SetDefaultB();

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
  //printf("%d init\n", pid);
  role.work();
  //printf("%d work\n", pid);
  endTime = MPI_Wtime();
  role.get(startTime, endTime);
  //printf("%d end\n", pid);
  MPI_Finalize();
  return 0;
}

void MasterInit() {
  // define process job size
  i = j = PROBLEM_SIZE;
  MPI_Bcast(&i, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(&j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
  SetJobOrder();
  MPI_Bcast(jobSize, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(dispOrder, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  SetDefaultB();
  MPI_Bcast(BVector, j, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  SetDefaultA();
  revBuf = (float*)malloc(jobSize[pid] * sizeof(float));
  MPI_Scatterv(AMatrix, jobSize, dispOrder, MPI_FLOAT, revBuf, jobSize[pid], MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  free(AMatrix);

  SetJobRange(i);
  return;
}

void Work(){
  sendBuf = (float*)malloc(j * sizeof(float));
  int indexI ,indexJ;
  indexJ = 0;
  while(indexJ < j) {
    sendBuf[indexJ] = 0;
    indexJ++;
  }
  
  indexJ = 0;
  while(indexJ < j) {
    int index =  0;
    indexI = 0;
    while(indexI < i) {
      sendBuf[indexI] += BVector[indexJ] *  revBuf[index];
      indexI++;
      index++;
    }
    indexJ++;
  }
  indexJ = 0;
  return;
}
void MasterReport(double start, double end) {
  CVector = (float*)malloc(j * sizeof(float));
  MPI_Reduce(sendBuf, CVector, j, MPI_FLOAT, MPI_SUM, ROOT, MPI_COMM_WORLD);
  free(jobSize);
  free(dispOrder);
  free(BVector);
  free(revBuf);
  free(sendBuf);
  int indexI = 0;
  float result = CVector[0];
  printf("Result %.2f\n", result);
  while(indexI < j) {
    if (result != CVector[indexI]) printf("Dif %.2f\n", CVector[indexI]);
    indexI++;
  }
  printf("\ntime:%.2lf\n", end-start);
  return;
}

void SlaveInit() {
  jobSize = (int*)malloc(group * sizeof(int));
  dispOrder = (int*)malloc(group * sizeof(int));
  MPI_Bcast(&i, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(&j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(jobSize, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(dispOrder, group, MPI_INT, ROOT, MPI_COMM_WORLD);
  BVector = (float*)malloc(j * sizeof(float));
  revBuf = (float*)malloc(jobSize[pid] * sizeof(float));
  MPI_Bcast(BVector, j, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  MPI_Scatterv(NULL, jobSize, dispOrder, MPI_FLOAT, revBuf, jobSize[pid], MPI_FLOAT, ROOT, MPI_COMM_WORLD);

  SetJobRange(i);
  return;
}
void SlaveReport(double start, double end) {
  MPI_Reduce(sendBuf, NULL, j, MPI_FLOAT, MPI_SUM, ROOT, MPI_COMM_WORLD);
  free(jobSize);
  free(dispOrder);
  free(BVector);
  free(revBuf);
  free(sendBuf);
  return;
}

void SetJobOrder() {
  int slaveJobSize, masterJobSize;
  slaveJobSize = (int)(j / group) * i;
  masterJobSize = (int)(j % group) * i;
  if(masterJobSize == 0) {
    masterJobSize = slaveJobSize;
  }

  jobSize = (int*)malloc(group * sizeof(int));
  dispOrder = (int*)malloc(group * sizeof(int));
  jobSize[0] = masterJobSize;
  dispOrder[0] = 0;
  int indexI = 1;
  while(indexI < group) {
    dispOrder[indexI] = dispOrder[indexI - 1] + jobSize[indexI - 1];
    jobSize[indexI] = slaveJobSize;
    indexI++;
  }
  return;
}

void SetDefaultA() {
  int size = i * j;
  AMatrix = (float*)malloc(size * sizeof(float));
  int indexJ = 0;

  while(indexJ < size) {
    AMatrix[indexJ] = DEFAULT_VALUE;
    indexJ++;
  }
  return;
}

void SetDefaultB() {
  BVector = (float*)malloc(j * sizeof(float));
  int indexJ = 0;
  while(indexJ < j) {
    BVector[indexJ] = DEFAULT_VALUE;
    indexJ++;
  }
  return;
}

void SetJobRange(unsigned length) {
  job detail={
    .size = (jobSize[pid] / length)
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
