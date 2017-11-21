/* 	matrix multiplization program
	System Programing
	author	Hoseok Choi & Solbi Choi
	since	2017.11.18
*/


#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#define MAX 4000

long long** C;
int **A;
int **B;
struct timeval start;
struct timeval end;
typedef struct total{
	long long totalsum;
	struct timeval middle2;
	pthread_mutex_t f_lock;
} total;

total total_sum;

typedef struct inputs
{
	int start;
	int colunm;
}inputs;

/*thread worker function*/
void *multi_and_sum(void* arg){
	int a_col, a_row, b_row;
	inputs* st = (inputs*)arg;
	long long sum = 0;
	for(a_col = st->start; a_col < st->colunm; a_col++){
		for(b_row = 0; b_row < MAX; b_row++){
			for(a_row = 0; a_row < MAX; a_row++){
				 sum += (A[a_col][a_row] * B[b_row][a_row]); 
			}	
		}
	}
	
	pthread_mutex_lock(&total_sum.f_lock);  //mutex lock
	total_sum.totalsum += sum;
	pthread_mutex_unlock(&total_sum.f_lock);
	return NULL;
}



void thr_func2(int N){
	int i;
	inputs **arg;
	total_sum.totalsum = 0;
	pthread_mutex_init(&total_sum.f_lock, NULL);
	arg = (inputs**)malloc(sizeof(inputs*)*N);
	pthread_t tid[N];
	int col = MAX / N;

	for(i = 0; i < N; i++){
		arg[i] = (inputs*)malloc(sizeof(inputs*));
	}

	for(i = 0; i < N; i++){
		arg[i]->colunm = (i + 1) * col;
		arg[i]->start = i * col;
		if(i == N - 1){
			arg[i]->colunm = MAX;
		}
	}
	gettimeofday(&start,NULL);
	for(i = 0; i < N; i++){
		pthread_create(&tid[i],NULL,multi_and_sum,(void*)arg[i]);
	}
	for(i = 0; i < N; i++){
		pthread_join(tid[i],NULL);
	}
	
	gettimeofday(&end,NULL);
	printf("total sum: %llu\n",total_sum.totalsum);
	printf("thread number: %d\n", N);
	printf("calculate time: %ld:%ld\n",(end.tv_sec - start.tv_sec) / 60, (end.tv_sec - start.tv_sec) % 60);
	free(arg);
}





int main(int argc, char** argv){
	int col,row;
	int i, sum;
	A = (int**)malloc(sizeof(int*)*MAX);
	B = (int**)malloc(sizeof(int*)*MAX);
	C = (long long**)malloc(sizeof(long long*)*MAX);
	for(i = 0; i < MAX;i++){
		A[i] = (int*)malloc(sizeof(int)*MAX);
		B[i] = (int*)malloc(sizeof(int)*MAX);
		C[i] = (long long*)malloc(sizeof(long long)*MAX);
	}
	
	
	FILE* fd1 = fopen(argv[1],"r+");
	FILE* fd2 = fopen(argv[2],"r+");
	if(fd1 == NULL || fd2 == NULL){
		printf("no files\n");
		return 0;
	}
	for(col = 0; col < MAX; col++){
		for(row = 0; row < MAX; row++){
			fscanf(fd1,"%d",&A[col][row]);
		}
	}
	for(col = 0; col < MAX; col++){ //recieve matrix B data by colunm oriented
		for(row = 0; row < MAX; row++){
			fscanf(fd2,"%d",&B[row][col]);
		}
	}
	fclose(fd1);
	fclose(fd2);

	thr_func2(8);
	free(A);
	free(B);
	free(C);
	printf("\a\n");
	return 0;
}
