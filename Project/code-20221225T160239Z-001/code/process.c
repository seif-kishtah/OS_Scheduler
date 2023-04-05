#include "headers.h"

/* Modify this file as needed*/
void myhandler1(int);
void myhandler2(int);
key_t remainingtime;
int finishtime;
key_t remaining;
int mid;

int main(int argc,char*argv[]) //savedremaining is equal to bursttime at first then it will be the remaining time when state saved
{
    int *shmaddr_process;
    remaining = ftok("lab.txt",22);
    mid = shmget(remaining, 4, IPC_CREAT | 0644);
    shmaddr_process = (int *)shmat(mid, NULL, 0);
    printf("the process remaining time  is %d and key is %d\n",*shmaddr_process,mid);
    signal(SIGUSR1,myhandler1);
    signal(SIGUSR2,myhandler2);
    initClk(); //to be able to read the shared memory(clock)
    remainingtime=*shmaddr_process;

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    int x;
    while (remainingtime > 0)
    {
        x=getClk(); //get current time
        // while(x==getClk()){
        // } //busy wait till time is incremented one
        *shmaddr_process = remainingtime;
        while(x == getClk());
        printf("the current time inside process  is %d\n",remainingtime);
        remainingtime=remainingtime-1; //Here we're sure that time is incremented
        *shmaddr_process = remainingtime;

    }
    
    //finishtime=x+1;
    destroyClk(false);
    printf("i terminated with remaining time = %d\n",*shmaddr_process);
    exit(0);
}

int currentstate(){   //this function will be used to save the current state 
    return remainingtime;
}

int finish_time(){
    return finishtime;
}

void myhandler1(int signum){
    *shmaddr = getClk();
}
void myhandler2(int signum){
    
}
