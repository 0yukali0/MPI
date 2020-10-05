#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#define ROOT 0
#define MAX_SIZE 30
typedef struct command{
    void (*init)();
    void (*work)();
    void (*show)();
}command;
unsigned req_i;
unsigned req_j;
unsigned req_k;
int* slice_order;
int* disp_order;
int* block_disp_order;
int* block_slice_order;
int pid;
int group;
int** A;
int* AStorage;
int** B;
int* BStorage;
int** C = NULL;
int* CStorage = NULL;
int* recBuffer = NULL;
int* computeBuffer = NULL;
void initMaster();
void initWorker();
void computeMaster();
void computeWorker();
void showMaster(double start, double end);
void showWorker(double start, double end);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    double start, end; 
    start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &group);
    command myJob;
    if(pid == ROOT) {
        myJob.init = initMaster;
        myJob.work = computeMaster;
        myJob.show = showMaster;
    } else {
        myJob.init = initWorker;
        myJob.work = computeWorker;
        myJob.show = showWorker;
    }
    myJob.init();
    myJob.work();
    end = MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    myJob.show(start, end);
    MPI_Finalize();
    return 0; 
}
void showMaster(double start, double end) {
    printf("pid:%d for time:%.6lf\n",pid, ((end-start)/CLOCKS_PER_SEC));
    MPI_Barrier(MPI_COMM_WORLD);
    int i, k;
    for(i = 0;i < req_i;i++) {
        putchar('\n');
        for(k = 0;k < req_k;k++) {
            printf("%02d ", C[k][i]);
        }
    }
    putchar('\n');
}
void showWorker(double start, double end) {
    printf("pid:%d for time:%.6lf\n",pid, ((end-start)/CLOCKS_PER_SEC));
    MPI_Barrier(MPI_COMM_WORLD);
}
void computeMaster() {
    int i, j, k, i_limit;
    i_limit = slice_order[pid];
    recBuffer = (int*)malloc(req_i * sizeof(int));
    computeBuffer = (int*)malloc(i_limit * sizeof(int));
    //printf("order %d:%d\n",pid , i_limit);
    for(k = 0;k < req_k;k++) {
        for(i = 0;i < i_limit;i++) {
            computeBuffer[i] = 0;
            //printf("%d:%d ",pid , i);
            for(j = 0;j < req_j;j++) {
                computeBuffer[i] += A[i][j] * B[j][k];
            }
        }
        //printf("Master:%d for k:%d getting\n",pid , k);
        MPI_Allgatherv(computeBuffer, i_limit, MPI_INT, recBuffer, slice_order, disp_order, MPI_INT, MPI_COMM_WORLD);
        //printf("Master:%d for k:%d get!\n",pid ,k);
        for(j = 0;j <req_j;j++) {
            C[k][j] =  recBuffer[j];
        }
    }
}
void computeWorker() {
    int i, j, k, i_limit;
    i_limit = slice_order[pid];
    //printf("order %d:%d\n",pid , i_limit);
    recBuffer = (int*)malloc(req_i * sizeof(int));
    computeBuffer = (int*)malloc(i_limit * sizeof(int));
    for(k = 0;k < req_k;k++) {
        for(i = 0;i < i_limit;i++) {
            computeBuffer[i] = 0;
            //printf("%d:%d\n",pid , i);
            for(j = 0;j < req_j;j++) {
                computeBuffer[i] += A[i][j] * B[j][k];
            }
        }
        printf("worker:%d for k:%d grouping\n",pid , k);
        MPI_Allgatherv(computeBuffer, i_limit, MPI_INT, recBuffer, slice_order, disp_order, MPI_INT, MPI_COMM_WORLD);
        //printf("worker:%d for k:%d grouping end!\n",pid ,k);
    }
}
void initMaster() {
    srand(1);
    req_i = (MAX_SIZE-1) % MAX_SIZE + 1;
    req_j = (MAX_SIZE-1) % MAX_SIZE + 1;
    req_k = (MAX_SIZE-1) % MAX_SIZE + 1;
    //printf("%d:ijk init\n", pid);
    MPI_Bcast(&req_i, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_k, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    //printf("%d:ijk init end\n", pid);

    B = (int**)malloc(req_k * sizeof(int*));
    BStorage = (int*)malloc(req_j * req_k * sizeof(int));
    C = (int**)malloc(req_i *sizeof(int*));
    CStorage = (int*)malloc(req_i * req_k * sizeof(int));
    //B column major
    int i, j, k, k_start, i_start, element, temp;
    for(k = 0, k_start = 0;k < req_k;k++) {
        B[k] = &BStorage[k_start];
        k_start += req_j;
    }
    //C column major
    for(k = 0, k_start = 0;k < req_k;k++) {
        C[k] = &CStorage[k_start];
        k_start += req_i;
    }
    //assign value to B:j*k (column)
    for(k = 0, element = 0;k < req_k;k++) {
        for(j = 0;j < req_j;j++) {
            temp = rand();
            if(temp & 1)
                BStorage[element] = (int)(temp % MAX_SIZE + 1) * -1;
            else
                BStorage[element] = (int)(temp % MAX_SIZE + 1);
            element++;
        }
    }
    //printf("%d:BC init\n", pid);
    MPI_Bcast(BStorage, req_j * req_k, MPI_INT, ROOT, MPI_COMM_WORLD);
    //printf("%d:BC init end\n", pid);
    //slice oredr decision
    slice_order = (int*)malloc(group * sizeof(int));
    disp_order = (int*)malloc(group * sizeof(int));
    block_slice_order = (int*)malloc(group * sizeof(int));
    block_disp_order = (int*)malloc(group * sizeof(int));
    disp_order[0] = 0;
    block_disp_order[0] = 0;
    int basic = req_i / group; 
    int plus = req_i % group;
    int assign;
    for(assign = 0;assign < group;assign++) {
        if(assign < plus)
            slice_order[assign] = basic + 1;
        else
            slice_order[assign] = basic;
        if(assign)
            disp_order[assign] = disp_order[assign - 1] + slice_order[assign - 1];
    }
    for(assign = 0;assign < group;assign++)
        block_slice_order[assign] = slice_order[assign] * req_k;
    for(;;)
    //printf("%d:order init\n", pid);
    MPI_Bcast(slice_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(disp_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    //printf("%d:order init end\n", pid);
    A = (int**)malloc(req_i * sizeof(int*));
    AStorage = (int*)malloc(req_i * req_j * sizeof(int));
    //row major
    for(i = 0, i_start = 0;i < req_i;i++) {
        A[i] = &AStorage[i_start];
        i_start += req_j;
    }
    //assign value to A:i*j
    for(i = 0, element = 0;i < req_i;i++) {
        for(j = 0;j < req_j;j++) {
            temp = rand();
            if(temp & 1)
                AStorage[element] = (int)(temp % MAX_SIZE + 1) * -1;
            else
                AStorage[element] = (int)(temp % MAX_SIZE + 1);
            element++;
        }
    }
    //send
    printf("%d:starting send\n", pid);
    int rec_pid, range;
    for(rec_pid = 1,i_start = disp_order[rec_pid];rec_pid < group;rec_pid++) {
        temp = disp_order[rec_pid];
        range = slice_order[rec_pid];
        printf("send:%d in %d i_start:%d slice:%d disp:%d\n", rec_pid, group,i_start, range, temp);
        MPI_Send(A[i_start], range * req_j, MPI_INT, rec_pid, range, MPI_COMM_WORLD);
        i_start += range;
    }
    printf("%d:ending send\n", pid);
}

void initWorker() {
    //printf("%d:ijk init\n", pid);
    MPI_Bcast(&req_i, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_k, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    //printf("%d:ijk init end\n", pid);;
    B = (int**)malloc(req_k * sizeof(int*));
    BStorage = (int*)malloc(req_k * req_j * sizeof(int));
    int i, j, k, k_start, i_start, element, temp;
    //column major
    for(k = 0, k_start = 0;k < req_k;) {
        B[k] = &BStorage[k_start];
        k_start += req_j;
        k++;
    }
    //printf("%d:BC init\n", pid);
    MPI_Bcast(BStorage, req_j * req_k, MPI_INT, ROOT, MPI_COMM_WORLD);
    //printf("%d:BC init end\n", pid);
    slice_order = (int*)malloc(group * sizeof(int));
    disp_order = (int*)malloc(group * sizeof(int));
    //printf("%d:order init\n", pid);
    MPI_Bcast(slice_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(disp_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    //printf("%d:order init end\n", pid);
    int myRange = slice_order[pid];
    AStorage = (int*)malloc(myRange * sizeof(int));
    //recive
    printf("%d:waiting rec\n", pid);
    MPI_Status status;
    MPI_Recv(AStorage, myRange, MPI_INT, ROOT, myRange, MPI_COMM_WORLD, &status);
    printf("%d:accepting rec\n", pid);
}