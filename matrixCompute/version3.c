#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SIZE 2500
#define ARRAY_SIZE 62500

typedef struct node {
	double element[SIZE];
}column;

double a[SIZE][SIZE];
column b[SIZE];
double c[SIZE][SIZE];

int main(void) {
	double start, end;
	int i,j,k;
	double temp;
	for(int n=0;n<SIZE;n++)
		for(int m=0;m<SIZE;m++) {
			a[n][m] = b[m].element[n] =1.1f;
		}
	start = clock();
	for(i=0;i<SIZE;i++){
		for(j=0;j<SIZE;j++) {
			for(k=0,temp=0;k<SIZE;k++) {
				temp += a[i][k] * b[j].element[k];
			}
			c[i][j] = temp;
		}
	}
	end = clock();
	printf("%lf",(double)(end - start)/CLOCKS_PER_SEC);
	return 0;
}