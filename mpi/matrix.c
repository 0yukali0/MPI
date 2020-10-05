#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ROOT 0
#define MAX_SIZE 10
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
int pid;
int group;
int** A;
int* AStorage;
int** B;
int* BStorage;
int** C = NULL;
int* CStorage = NULL;
int* recBuffer;
int* computeBuffer;
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
    printf("%d:init\n", pid);
    myJob.init();
    printf("%d:init end\n", pid);
    myJob.work();
    end = MPI_Wtime();
    myJob.show(start, end);
    MPI_Finalize();
    return 0; 
}
void showMaster(double start, double end) {
    printf("pid:%d for time:%.6lf\n",pid, ((end-start)/CLOCKS_PER_SEC));
}
void showWorker(double start, double end) {
    printf("pid:%d for time:%.6lf\n",pid, ((end-start)/CLOCKS_PER_SEC));
    for(int i = 0;i < req_i;i++) {
        putchar('\n');
        for(int k = 0;k < req_k;k++) {
            printf("%-d ", C[k][i]);
        }
    }
    putchar('\n');
}
void computeMaster() {
    for(int k = 0;k < req_k;k++) {
        for(int i = 0, i_limit = slice_order[pid];i < i_limit;i++) {
            computeBuffer[i] = 0;
            for(int j = 0;j < req_j;j++) {
                computeBuffer[i] += A[i][j] * B[j][k];
            }
        }
        MPI_Allgatherv(computeBuffer, slice_order[pid], MPI_INT, recBuffer, req_i, disp_order, MPI_INT, MPI_COMM_WORLD);
        for(int j = 0;j <req_j;j++) {
            C[k][j] =  recBuffer[j];
        }
    }
}
void computeWorker() {
    for(int k = 0;k < req_k;k++) {
        for(int i = 0, i_limit = slice_order[pid];i < i_limit;i++) {
            computeBuffer[i] = 0;
            for(int j = 0;j < req_j;j++) {
                computeBuffer[i] += A[i][j] * B[j][k];
            }
        }
        MPI_Allgatherv(computeBuffer, slice_order[pid], MPI_INT, recBuffer, req_i, disp_order, MPI_INT, MPI_COMM_WORLD);
    }
}
void initMaster() {
    srand(1);
    req_i = rand() % MAX_SIZE + 1;
    req_j = rand() % MAX_SIZE + 1;
    req_k = rand() % MAX_SIZE + 1;
    printf("%d:ijk init\n", pid);
    MPI_Bcast(&req_i, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_k, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    printf("%d:ijk init end\n", pid);
    B = (int**)malloc(req_k * sizeof(int*));
    BStorage = (int*)malloc(req_j * req_k * sizeof(int));
    C = (int**)malloc(req_i *sizeof(int*));
    CStorage = (int*)malloc(req_i * req_k * sizeof(int));
    recBuffer = (int*)malloc(req_i * sizeof(int));
    //B column major
    for(int k = 0, k_start = 0;k < req_k;k++) {
        B[k] = &BStorage[k_start];
        k_start += req_j;
    }
    //C column major
    for(int k = 0, k_start = 0;k < req_k;k++) {
        C[k] = &CStorage[k_start];
        k_start += req_i;
    }
    //assign value to B:j*k (column)
    for(unsigned k = 0, elemelt = 0;k < req_k;k++) {
        for(unsigned j = 0, temp;j < req_j;j++) {
            temp = rand();
            if(temp & 1)
                BStorage[elemelt] = (int)(temp % MAX_SIZE + 1) * -1;
            else
                BStorage[elemelt] = (int)(temp % MAX_SIZE + 1);
            elemelt++;
        }
    }
    printf("%d:BC init\n", pid);
    MPI_Bcast(BStorage, req_j * req_k, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("%d:BC init end\n", pid);
    //slice oredr decision
    slice_order = (int*)malloc(group * sizeof(int));
    disp_order = (int*)malloc(group * sizeof(int));
    disp_order[0] = 0;
    int basic = req_i / group; 
    int plus = req_i % group;
    for(int assign = 0;assign < group;assign++) {
        if(assign < plus)
            slice_order[assign] = basic + 1;
        else
            slice_order[assign] = basic;
        if(assign)
            disp_order[assign] = disp_order[assign - 1] + slice_order[assign];
    }
    printf("%d:order init\n", pid);
    MPI_Bcast(slice_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(disp_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("%d:order init end\n", pid);
    computeBuffer = (int*)malloc(slice_order[pid] * sizeof(int));
    A = (int**)malloc(req_i * sizeof(int*));
    AStorage = (int*)malloc(req_i * req_j * sizeof(int));
    //row major
    for(unsigned i = 0, i_start = 0;i < req_i;i++) {
        A[i] = &AStorage[i_start];
        i_start += req_j;
    }
    //assign value to A:i*j
    for(unsigned i = 0, elemelt = 0;i < req_i;i++) {
        for(unsigned j = 0, temp;j < req_j;j++) {
            temp = rand();
            if(temp & 1)
                AStorage[elemelt] = (int)(temp % MAX_SIZE + 1) * -1;
            else
                AStorage[elemelt] = (int)(temp % MAX_SIZE + 1);
            elemelt++;
        }
    }
    //send
    printf("%d:starting send\n", pid);
    for(int rec_pid = 0,i_start = 0, range;rec_pid < group;rec_pid++) {
        range = slice_order[rec_pid];
        MPI_Send(A[i_start], range * req_j, MPI_INT, rec_pid, range, MPI_COMM_WORLD);
        i_start += range;
    }
    printf("%d:ending send\n", pid);
}

void initWorker() {
    printf("%d:ijk init\n", pid);
    MPI_Bcast(&req_i, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_j, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&req_k, 1, MPI_UNSIGNED, ROOT, MPI_COMM_WORLD);
    printf("%d:ijk init end\n", pid);;
    B = (int**)malloc(req_k * sizeof(int*));
    BStorage = (int*)malloc(req_k * req_j * sizeof(int));
    /*
    C = (int**)malloc(req_i *sizeof(int*));
    CStorage = (int*)malloc(req_i * req_k * sizeof(int));
    */
    recBuffer = (int*)malloc(req_i * sizeof(int));
    //column major
    for(int k = 0, k_start = 0;k < req_k;) {
        B[k] = &BStorage[k_start];
        k_start += req_j;
        k++;
    }
    //column major
    /*
    for(int k = 0, k_start = 0;k < req_k;k++) {
        C[k] = &CStorage[k_start];
        k_start += req_i;
    }
    */
    printf("%d:BC init\n", pid);
    MPI_Bcast(BStorage, req_j * req_k, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("%d:BC init end\n", pid);
    slice_order = (int*)malloc(group * sizeof(int));
    disp_order = (int*)malloc(group * sizeof(int));
    printf("%d:order init\n", pid);
    MPI_Bcast(slice_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(disp_order, group, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("%d:order init end\n", pid);
    computeBuffer = (int*)malloc(slice_order[pid] * sizeof(int));
    int myRange = slice_order[pid];
    A = (int**)malloc(myRange * sizeof(int*));
    AStorage = (int*)malloc(myRange * req_j * sizeof(int));
    //row major
    for(unsigned i = 0, i_start = 0;i < myRange;i++) {
        A[i] = &AStorage[i_start];
        i_start += req_j;
    }
    //recive
    printf("%d:waiting rec\n", pid);
    MPI_Status status;
    MPI_Recv(&A[0], myRange * req_j, MPI_INT, ROOT, myRange, MPI_COMM_WORLD, &status);
    printf("%d:accepting rec\n", pid);
}