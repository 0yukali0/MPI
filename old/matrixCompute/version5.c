#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SIZE 2500
#define LENGTH  100
#define MAX_BLOCK 25

typedef struct node {
	double element[SIZE];
}column;


	double a[SIZE][SIZE];
	double b[SIZE][SIZE];
	double c[SIZE][SIZE];
	column right_min[LENGTH];
	double left_min[LENGTH][LENGTH];
	
int main(void) {
	double start, end;
	int i, j, k;
	
	for(int n=0;n<SIZE;n++)
		for(int m=0;m<SIZE;m++) {
			a[n][m] = b[n][m] =1.1f;
		}
	start = clock();
	//double load_s,load_result=0;
	for(i=0;i<MAX_BLOCK;i++){
		int limit_i=LENGTH*(i+1);
		for(k=0;k<MAX_BLOCK;k++) {
			int limit_k=LENGTH*(k+1);
			for(j=0;j<MAX_BLOCK;j++) {
				int limit_j=LENGTH*(j+1);
				int block_i=i*LENGTH,block_j=j*LENGTH,block_k=k*LENGTH;
				
				//load_s = clock();
				for(block_i=i*LENGTH;block_i<limit_i;block_i++) {
					int load_i=block_i%LENGTH;
					for(block_k=k*LENGTH;block_k<limit_k;block_k++) {
						int load_k=block_k%LENGTH;
						left_min[load_i][load_k] = a[block_i][block_k];
						for(block_j=j*LENGTH;block_j<limit_j;block_j++) {
							int load_j=block_j%LENGTH;
							right_min[load_j].element[load_k]= b[block_k][block_j];
						}
					}
				}
				//load_result+=clock()-load_s;
				/*
				* block compute
				*block_ij += block_ik * block_kj
				*/
				for(block_i=i*LENGTH;block_i<limit_i;block_i++) {
					int load_i=block_i%LENGTH;
					for(block_j=j*LENGTH;block_j<limit_j;block_j++) {
						int load_j=block_j%LENGTH;
						double temp=0;
						for(block_k=k*LENGTH;block_k<limit_k;block_k++) {
							int load_k=block_k%LENGTH;
							temp += left_min[load_i][load_k] * right_min[load_j].element[load_k];
						}
							c[block_i][block_j] += temp ;
					}
				}
			}
		}
	}
	end = clock();
	printf("%lf\n",(double)(end - start)/CLOCKS_PER_SEC);
	//printf("compute time:%lf loading time:%lf",(end-start-load_result)/CLOCKS_PER_SEC,load_result/CLOCKS_PER_SEC);
	return 0;
}