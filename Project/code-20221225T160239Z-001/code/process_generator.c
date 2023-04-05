#include "headers.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

void clearResources(int);

struct PCB
{
    int arrivaltime;
    int priority;
    int runningtime;
    int remaining_time;
    int id;
    int finishtime;
    int pid;
    int start_time;
    int memsize;
};
struct algorithm{
    char name[3];
    bool preemptive;
    int Quantum;
    int process_count;
};
int done;
int main(int argc, char *argv[])
{
    signal(SIGUSR1,clearResources);
    signal(SIGINT, clearResources);
    key_t to_scheduler;
    to_scheduler = ftok("lab.txt",1);
    done = msgget(to_scheduler, 0666 | IPC_CREAT);
    struct algorithm algo;
    //-------------------------------------- 
    FILE * ptr;
    ptr = fopen("./processes_scheduler.txt","r");
    //----------------------------------------------------------------------------------------------------------------------------------------------------------------
    char ch;
    int count = 0;
    while((ch=fgetc(ptr)) != EOF){
        if(ch == '\n'){
            count += 1;
        }
    }
    rewind(ptr);
    //----------------------------------------------------------------------------------------------------------------------------------------------------------------
    fscanf(ptr,"%s",algo.name);
    printf("%s\n",algo.name);
    if(strcmp(algo.name,"SJF") == 0){
        fscanf(ptr,"%d",&algo.preemptive);
        printf("%d\n",algo.preemptive);
    }
    else if(strcmp(algo.name,"HPF") == 0){
        fscanf(ptr,"%d",&algo.preemptive);
    }
    else if(strcmp(algo.name,"RR") == 0){
        fscanf(ptr,"%d",&algo.Quantum);
    }
    else if(strcmp(algo.name,"MFL") == 0){
        fscanf(ptr,"%d",&algo.Quantum);
    }
    struct PCB *all_processes[count - 2];
    for(int i =0; i < count-2 ; i++)
    all_processes[i] = malloc(sizeof(struct PCB));
    int i = 0;
    while(fscanf(ptr,"%d  %d  %d  %d  %d",&all_processes[i]->id,&all_processes[i]->arrivaltime,&all_processes[i]->runningtime,&all_processes[i]->priority,&all_processes[i]->memsize) != EOF){
        all_processes[i]->remaining_time = all_processes[i]->runningtime;
        printf("id %d   arrival time %d   running time %d   priorty %d   remianing time %f   memsize %d \n",all_processes[i]->id,all_processes[i]->arrivaltime,all_processes[i]->runningtime,all_processes[i]->priority,all_processes[i]->remaining_time,all_processes[i]->memsize);
        i++;
    }
    //----------------------------------------------------------------------------------------------------------------------------------------------------------------
    fclose(ptr);
    initClk();
    int pid = fork();
    int pidClk;
    if(pid == 0){
         char*args[] = {"scheduler",NULL};
         execv("./scheduler",args);
     }else{
         pidClk=fork();
         if(pidClk==0){
            char*args[] = {"clk",NULL};
             execv("./clk",args);
             printf("clock started\n");
         }
    }
    algo.process_count = count - 2;
     int sending = msgsnd(done,&algo,sizeof(algo),!IPC_NOWAIT);
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
    struct PCB process;
    for(int i = 0 ; i < count-2 ; i++){
        printf("Current Time is %d\n", getClk());
        while(getClk() < all_processes[i]->arrivaltime){
            
        }
        printf("process id %d arrived at %d and is sent to scheduler\n",all_processes[i]->id,getClk());
        int s = msgsnd(done,all_processes[i],sizeof(*all_processes[i]),IPC_NOWAIT);
        if(s == -1){
            i--;
        }
    }
    kill(pid,SIGUSR1);
    while(wait(NULL) > 0){
        continue;
    }
    destroyClk(false);
    //kill(0,SIGINT);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    exit(0);
}
