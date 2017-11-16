/* System Programing Assignment 4-2
    Signal counter
 
    author: Hoseok Choi & Solbi Choi
    since: 2017.11.15
                                         */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>


struct sigaction act, old_act, del;

pid_t pid;         //자식 프로세스 pid
pid_t ppid;        // 최초 프로세스의 pid
int fd;             // sample.txt 의 파일 디스크립터
sigset_t fillset;
int number = 0;
int counter;
char num_to_char[10] = "0";
int len = 1;
char buf[9];

/*프로세스 종료 핸들링 함수
 SIGINT 시그널을 받으면 실행 하게 된다. 최초로 이함수에 들어오는 프로세스는 무조건 init 프로세스이고,
 종료되기 전에 자식 프로세스에게 SIGINT 시그널을 보낸다. 자식이 없을 경우 그냥 종료 된다.             */
static void sig_int(int signo){
	if(pid == 0){
		exit(0);
	}else{
		kill(pid,SIGINT);
		exit(0);
	}
}

/* 시그널 핸들링 함수
 sample.txt 로 부터 number를 읽어와서 counter 와 비교 한뒤
 number 가 counter 보다 작을 경우 number 의 수를 하나 증가시켜서 sample.txt에 저장한다.
 저장이 끝난 후에 자식이 있는 프로세스는 자식 프로세스에게 SIGALRM을 보내고 자식이 없는 프로세스는 최초 프로세스로 SIGALRM 을 보낸다.
 따라서 프로세스의 순서는 최초프로세스 -> 자식프로세스1 -> 자식프로세스2 -> 최초프로세스 가 된다.
 모든 연산이 끝나면 sigsuspend 함수를 이용해서 sleep 상태가 된다. sigsuspend 함수가 호출되기 전에 들어온 SIGALRM 시그널은
 유보 되고, sigsuspend 함수가 호출 되었을때 유보된 SIGALRM 시그널을 접수한다.
 number = counter 가 되면, 다음 순서의 프로세스로 SIGINT를 보내고 프로세스를 종료한다.                                     */
static void sig_alrm(int signo){
	pread(fd,num_to_char,len + 1,0);            // sample.txt 로 부터 숫자를 읽어서 num-to-char 에 저장
	number = atoi(num_to_char);                 // int 형으로 변경
	if(number < counter){        //아직 counter 할게 남은 경우
		number++;
		len = sprintf(num_to_char,"%d",number);
		pwrite(fd,num_to_char,len,0);     //sample.txt 에 +1 된 숫자 입력
		if(pid == 0){
			kill(ppid,SIGALRM);
		}else{
			kill(pid,SIGALRM);
		}
	}else{  //프로세스 종료
		kill(ppid,SIGINT);  //init 프로세스로 SIGINT 를 보낸다.
	}
	if(ppid == getpid() && number <= 3){
		alarm(5);
	}
}




/*main 함수*/
int main(int argc, char** argv){
	fd = open(argv[2], O_CREAT|O_RDWR|O_TRUNC,S_IRUSR|S_IWUSR);  //sample.txt 오픈
	pwrite(fd,num_to_char,len,0);               // sample.txt에 저장된 데이터를 0으로 초기화
	counter = atoi(argv[1]);
	if(counter < 0){    // counter must be bigger than zero
		printf("counter is too few\n");
		return 0;
	}
	
	del.sa_handler = sig_int;
	del.sa_flags = 0;
	sigfillset(&del.sa_mask);
	sigdelset(&del.sa_mask,SIGINT);
	sigaction(SIGINT, &del, NULL); //프로세스 종료 sigaction 함수 설정
	act.sa_handler = sig_alrm;   // 시그널 핸들러 함수 지정
	act.sa_flags = 0;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask,SIGINT);
	sigfillset(&fillset);
	sigdelset(&fillset,SIGINT);
	sigdelset(&fillset,SIGALRM);
	sigaction(SIGALRM, &act, NULL);  //sigation 함수 설정.
	sigprocmask(SIG_SETMASK,&del.sa_mask,NULL);  // SIGALRM block 설정
	
	ppid = getpid();
	pid = fork();
	if(pid == 0){
		pid = fork();

	}
	
	if(ppid == getpid()){
		sleep(1);
		kill(pid, SIGALRM);	
	}
	while(1){   //SIGALRM 의 block을 해제하고 sleep 한다.
		sigsuspend(&fillset);
	}
}

