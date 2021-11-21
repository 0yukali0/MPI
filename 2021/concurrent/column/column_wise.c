#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define ROOT 0
#define PROBLEM_SIZE 30
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
  role.work();
  endTime = MPI_Wtime();
  role.get(startTime, endTime);
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
  revBuf = (float*)malloc(j * sizeof(float));
  MPI_Scatterv(AMatrix, jobSize, dispOrder, MPI_FLOAT, revBuf, jobSize[pid], MPI_FLOAT, ROOT, MPI_COMM_WORLD);

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
    int index =  indexJ * i;
    indexI = 0;
    while(indexI < i) {
      sendBuf[indexI] += BVector[indexJ] *  revBuf[index];
      indexI++;
      index++;
    }
    indexJ++;
  }
  return;
}
void MasterReport(double start, double end) {
  CVector = (float*)malloc(j * sizeof(float));
  MPI_Reduce(sendBuf, CVector, j, MPI_FLOAT, MPI_SUM, ROOT, MPI_COMM_WORLD);
  int indexI = 0;
  float result = CVector[0];
  printf("Result %.f\n", result);
  while(indexI < j) {
    if (result != CVector[indexI]) printf("Dif %.2f\n", CVector[indexI])
    indexI++;
  }
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
  revBuf = (float*)malloc(j * sizeof(float));
  MPI_Scatterv(NULL, jobSize, dispOrder, MPI_FLOAT, revBuf, jobSize[pid], MPI_FLOAT, ROOT, MPI_COMM_WORLD);

  SetJobRange(i);
  return;
}
void SlaveReport(double start, double end) {
  MPI_Reduce(sendBuf, NULL, j, MPI_FLOAT, MPI_SUM, ROOT, MPI_COMM_WORLD);
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
  revBuf = (float*)malloc(masterJobSize * sizeof(float));
  int indexI = 1;
  while(indexI < group) {
    dispOrder[indexI] = dispOrder[indexI - 1] + jobSize[indexI - 1];
    jobSize[indexI] = slaveJobSize;
    indexI++;
  }
  return;
}

void SetDefaultA() {
  float* rows = NULL;
  int size = i * j;
  row = (float*)malloc(size * sizeof(float));
  AMatrix = (float*)malloc(size * sizeof(float));
  int indexI = 0;
  int indexJ = 0;
  while(indexI < i) {
    indexJ = 0;
    int index = indexJ * j;
    while(indexJ < j) {
      index += indexJ;
      row[index] = DEFAULT_VALUE;
      indexJ++;
    }
    indexI++;
  }

  while(indexI < i) {
    indexJ = 0;
    int index = indexJ * j;
    while(indexJ < j) {
      index += indexJ;
      AMatrix[indexJ * i + indexI] = row[index];
      indexJ++;
    }
    indexI++;
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
