#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SIZE 2500

	double a[SIZE][SIZE];
	double b[SIZE][SIZE];
	double c[SIZE][SIZE];

int main(void) {
	double start, end;
	long long i, j, k;
	double temp;
	
	for(int n=0;n<SIZE;n++)
		for(int m=0;m<SIZE;m++) {
			a[n][m] = b[n][m] =1.1f;
		}
	
	start = clock();
	for(i=0;i<SIZE;i++){
		for(k=0,temp=0;k<SIZE;k++) {
			temp = a[i][k];
			 for(j=0;j<SIZE;j++) {
				c[i][j] += temp * b[k][j];
			}
		}	
	}
	end = clock();
	printf("%lf",(double)(end - start)/CLOCKS_PER_SEC);
	return 0;
}