#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#define LIMIT 65536
#define INPUT_SIZE 16
#define ROOT 0
#define DATA_SIZE 1
#define a 0
#define b 1
#define c 2
#define d 3
#define e 4
#define f 5
#define g 6
#define h 7
#define i 8
#define j 9
#define k 10
#define l 11
#define m 12
#define n 13
#define o 14
#define p 15

typedef struct bits {
        unsigned bit: 16;
} bits;

bool isOutput(bits input, unsigned processor);
void printBits(bits answer);

int main(int argc, char** argv) {
        bits input;
        double start, end;
        unsigned int limit, answerNumber, id, totalOfAnswer;
        int groupSize;
        MPI_Init(&argc, &argv); //processors created
        MPI_Comm_rank(MPI_COMM_WORLD, &id);     //get processor id
        MPI_Comm_size(MPI_COMM_WORLD, &groupSize); //processor family size
        totalOfAnswer = 0;
        answerNumber = 0;
        input.bit = id;
        limit = id;

        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        do {
                if(limit % groupSize == id)
                        if(isOutput(input, id))
                                answerNumber++;
                input.bit += groupSize;
                limit += groupSize;
        }while(limit  < LIMIT);
        MPI_Reduce((void *)&answerNumber,(void *)&totalOfAnswer,
                 DATA_SIZE, MPI_UNSIGNED, MPI_SUM, ROOT, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        if(id == ROOT)
                printf("Number:%u\n", totalOfAnswer);
        printf("Time:%lf\n", (end - start)/CLOCKS_PER_SEC);
        MPI_Finalize();
        return 0;
}

bool isOutput(bits input, unsigned processor) {
        int Bit[INPUT_SIZE];
        int index;
        bits temp;
        temp.bit = 1;

        for(index = 0;index < INPUT_SIZE;index++) {
                Bit[index] = temp.bit & input.bit;
                temp.bit <<= 1;
        }

        if((Bit[a] || Bit[b])&&(!Bit[b] || !Bit[d])&&(Bit[c] || Bit[d])&&
        (!Bit[d] || !Bit[e])&&(Bit[e] || !Bit[f])&&(Bit[f] || !Bit[g])&&
        (Bit[f] || Bit[g])&&(Bit[h] || !Bit[i])&&(Bit[i] || Bit[j])&&
        (Bit[i] || !Bit[j])&&(!Bit[j] || !Bit[k])&&(Bit[k] || Bit[l])&&
        (Bit[l] || Bit[j])&&(Bit[m] || Bit[n])&&(Bit[o] || Bit[p])&&
        (Bit[g] || !Bit[p])&&(!Bit[o] || Bit[n])&&(!Bit[h] || !Bit[n])
        ) {
                printf("%u)", processor);
                printBits(input);
                return true;
        }
        return false;
}

void printBits(bits answer) {
        int index;
        for(index = 0;index < INPUT_SIZE;index++) {
                switch(answer.bit & 1) {
                        case 0:
                        putchar('0');
                        break;
                        case 1:
                        putchar('1');
                        break;
                }
                answer.bit >>= 1;
        }
        putchar('\n');
}

