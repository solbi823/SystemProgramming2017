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
struct timeval middle1;
//struct timeval middle2;
struct timeval middle3;
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

long long add(int start, int colunm){
	int col, row;
	long long sum = 0;
	for(col = start; col < colunm; col++){
		for(row = 0; row < MAX; row++){
			sum += C[col][row];
		}
	}
	return sum;
}

void *multi(void* arg){
	int a_col, a_row, b_row;
	long long sum;
	inputs* st = (inputs*)arg;
	struct timeval thr_start;
	struct timeval thr_end;
	gettimeofday(&thr_start,NULL);
	for(a_col = st->start; a_col < st->colunm; a_col++){
		for(b_row = 0; b_row < MAX; b_row++){
			sum = 0;
			for(a_row = 0; a_row < MAX; a_row++){
				 sum += (A[a_col][a_row] * B[a_row][b_row]);
			}
			C[a_col][b_row] = sum;
		}
	}
	gettimeofday(&thr_end,NULL);
	return NULL;
}

void *multi2(void* arg){
	int a_col, a_row, b_row;
	inputs* st = (inputs*)arg;
	long long sum = 0;
	struct timeval thr_start;
	gettimeofday(&thr_start,NULL);
	for(a_col = st->start; a_col < st->colunm; a_col++){
		for(b_row = 0; b_row < MAX; b_row++){
			for(a_row = 0; a_row < MAX; a_row++){
				 sum += (A[a_col][a_row] * B[b_row][a_row]);
			}
		}
	}
	pthread_mutex_lock(&total_sum.f_lock);
	total_sum.totalsum += sum;

	pthread_mutex_unlock(&total_sum.f_lock);
	return NULL;
}



void thr_func(int N){
	int i;
	inputs **arg;
	arg = (inputs**)malloc(sizeof(inputs*)*N);
	pthread_t tid[N];
	int col = MAX / N;
	int dividelse = MAX % N;
	long long sum;

	gettimeofday(&start,NULL);
	for(i = 0; i < N; i++){
		arg[i] = (inputs*)malloc(sizeof(inputs*));
		arg[i]->colunm = (i + 1) * col + dividelse;
		arg[i]->start = i * col + dividelse;
		if(i == 0){
			arg[i]->start = 0;
		}
		pthread_create(&tid[i],NULL,multi,(void*)arg[i]);
	}
	for(i = 0; i < N; i++){
		pthread_join(tid[i],NULL);
	}
	sum = add(0,MAX);
	gettimeofday(&end,NULL);
	printf("thread number: %d\n",N);
	printf("total time: %ld:%ld\n",(end.tv_sec - start.tv_sec) / 60, (end.tv_sec - start.tv_sec) % 60);
	free(arg);
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

	gettimeofday(&start,NULL);
	for(i = 0; i < N; i++){
		arg[i]->colunm = (i + 1) * col;
		arg[i]->start = i * col;
		if(i == N - 1){
			arg[i]->colunm = MAX;
		}
		pthread_create(&tid[i],NULL,multi2,(void*)arg[i]);
	}
	for(i = 0; i < N; i++){
		pthread_join(tid[i],NULL);
	}
	
	gettimeofday(&end,NULL);
	printf("%llu\n",total_sum.totalsum);
	printf("thread number: %d\n", N);
	printf("total time: %ld:%ld\n",(end.tv_sec - start.tv_sec) / 60, (end.tv_sec - start.tv_sec) % 60);
	free(arg);
}





int main(int argc, char** argv){
	int col,row;
	int i, sum;
	int thread_num;
	int 
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
	for(col = 0; col < MAX; col++){
		for(row = 0; row < MAX; row++){
			fscanf(fd2,"%d",&B[col][row]);
		}
	}
	fclose(fd1);
	fclose(fd2);

	printf("please enter the thread number: ");
	scanf("%d",&thread_num);
	thr_func(thread_num);
	free(A);
	free(B);
	free(C);
	return 0;
}

