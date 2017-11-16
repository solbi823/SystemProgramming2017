#include <stdio.h>
#include <signal.h>
#include <unistd.h>

struct sigaction act;
struct sigaction act_old;

void sig_fn(int signo){
printf("Ctrl-C is pressed. Try Again.\n");
}

int main(){
act.sa_handler=sig_fn;
sigemptyset(&act.sa_mask);

sigaction(SIGINT, &act, &act_old);

while(1){
printf("keep going....\n");
sleep(1);
}
}

