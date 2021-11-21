#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SIZE 2500
#define LENGTH 100
#define MAX_BLOCK 25

	double a[SIZE][SIZE];
	double b[SIZE][SIZE];
	double c[SIZE][SIZE];

int main(void) {
	double start, end;
	int i, j, k;
	
	for(int n=0;n<SIZE;n++)
		for(int m=0;m<SIZE;m++) {
			a[n][m] = b[n][m] =1.1f;
		}
	start = clock();
	for(i=0;i<MAX_BLOCK;i++){
		int limit_i=LENGTH*(i+1);
		for(k=0;k<MAX_BLOCK;k++) {
			int limit_k=LENGTH*(k+1);
			for(j=0;j<MAX_BLOCK;j++) {
				int limit_j=LENGTH*(j+1);
				/*
				* block compute
				*block_ij += block_ik * block_kj
				*/
				int block_i,block_j,block_k;
				for(block_i=i*LENGTH;block_i<limit_i;block_i++) {
					for(block_k=k*LENGTH;block_k<limit_k;block_k++) {
						double temp = a[block_i][block_k];
						for(block_j=j*LENGTH;block_j<limit_j;block_j++) 
							c[block_i][block_j] += temp * b[block_k][block_j];
					}
				}
			}
		}
	}

	end = clock();
	
	printf("%lf",(double)(end - start)/CLOCKS_PER_SEC);
	return 0;
}