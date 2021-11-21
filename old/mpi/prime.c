#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#define SIZE 100000001
#define range 100
bool number[SIZE];
void mark(unsigned long index);
void printPrime();
int main(void) {
    double start = clock(), end;
    for(unsigned long index = 0;index < SIZE;index++)
        number[index] = true;
        number[1] = false;
    for(unsigned long index = 3;index < SIZE; index+=2) {
        mark(index);
    }
    printPrime();
    end = clock();
    printf("\ntime:%lf\n", (end - start)/CLOCKS_PER_SEC);
    return 0;
}

void mark(unsigned long index) {
    unsigned long temp = index << 1;
    for(unsigned long test = index * index;test < SIZE;test += temp) {
        number[test] = false;
    }
}

void printPrime() {
    unsigned long count = 0;
    for(unsigned long index = 3;index < SIZE;index+=2)
        if(number[index])
            count++;
    count++;
    printf("***Total count***\ncount: %lu",count);
}