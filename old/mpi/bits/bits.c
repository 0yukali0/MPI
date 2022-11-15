#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <time.h>
#define LIMIT 65536
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

bool isOutput(bits input);
void printBits(bits answer);

int main(void) {
	bits input;
	double start = clock(), end;
	unsigned int limit = 0 , answerNumber = 0;
	input.bit = 0;
	do {
		if(isOutput(input))
			answerNumber++;
		input.bit++;
		limit++;
	} while(limit < LIMIT);
	end = clock();
	printf("number:%u\n", answerNumber);
	printf("time:%lf\n",(end - start) / CLOCKS_PER_SEC);
	return 0;
}

bool isOutput(bits input) {
	int Bit[16];
	bits temp;
	temp.bit = 1;
	for(int index = 0;index < 16;index++) {
		Bit[index] = temp.bit & input.bit;
		temp.bit <<= 1;
	}
	if((Bit[a] || Bit[b]) && (!Bit[b] || !Bit[d]) && (Bit[c] || Bit[d]) &&
	   (!Bit[d] || !Bit[e]) && (Bit[e] || !Bit[f]) && (Bit[f] || !Bit[g]) &&
	   (Bit[f] || Bit[g]) && (Bit[h] || !Bit[i]) && (Bit[i] || Bit[j]) && 
	   (Bit[i] || !Bit[j]) && (!Bit[j] || !Bit[k]) && (Bit[k] || Bit[l]) &&
	   (Bit[l] || Bit[j]) && (Bit[m] || Bit[n]) && (Bit[o] || Bit[p]) &&
	   (Bit[g] || !Bit[p]) && (!Bit[o] || Bit[n]) && (!Bit[h] || !Bit[n])
	) {
		printBits(input);
		return 1;
	}
	return 0;
}

void printBits(bits answer) {
	for(int index = 0;index < 16; index++) {
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