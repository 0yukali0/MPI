#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <time.h>
#define LIMIT 65536
#define A 0x0001
#define B 0x0002
#define C 0x0004
#define D 0x0008
#define E 0x0010
#define F 0x0020
#define G 0x0040
#define H 0x0080
#define I 0x0100
#define J 0x0200
#define K 0x0400
#define L 0x0800
#define M 0x1000
#define N 0x2000
#define O 0x4000
#define P 0x8000

typedef struct bits {
	unsigned bit: 16;
} bits;

bool isOutput(bits input);
void printBits(bits answer);

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	bits input;
	double start, end, sumTime;
	unsigned int limit = 0 , answerNumber = 0, sumNumber = 0;
	int processNumber, groupSize;
	MPI_Comm_rank(MPI_COMM_WORLD, &processNumber);
	MPI_Comm_size(MPI_COMM_WORLD, &groupSize);
	input.bit = 0;
	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();
	do {
		if(limit % groupSize == processNumber) {
			if(isOutput(input))
				answerNumber++;
		}
		input.bit++;
		limit++;
	} while(limit < LIMIT);
	MPI_Reduce(&answerNumber, &sumNumber, 1, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	end = MPI_Wtime();
	if(processNumber == 0)
		printf("processor:%lf of total:%d\n", processNumber, groupSize);
	MPI_Finalize();
	return 0;
}

bool isOutput(bits input) {
	int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
	a = input.bit & A;
	b = input.bit & B;
	c = input.bit & C;
	d = input.bit & D;
	e = input.bit & E;
	f = input.bit & F;
	g = input.bit & G;
	h = input.bit & H;
	i = input.bit & I;
	j = input.bit & J;
	k = input.bit & K;
	l = input.bit & L;
	m = input.bit & M;
	n = input.bit & N;
	o = input.bit & O;
	p = input.bit & P;
	if((a || b) && (!b || !d) && (c || d) &&
		(!d || !e) && (e || !f) && (f || !g) &&
		(f || g) && (h || !i) && (i || j) && 
		(i || !j) && (!j || !k) && (k || l) &&
		(l || j) && (m || n) && (o || p) &&
		(g || !p) && (!o || n) && (!h || !n)
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