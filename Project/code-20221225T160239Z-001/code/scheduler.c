#include "headers.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "Tree.c"


struct algorithm{
    char name[3];
    bool preemptive;
    int Quantum;
    int process_count;
};
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
    int startadd;
    int endadd;
    int memsize;
};
//--------------------------------------
//priority queue
typedef struct node{
    struct PCB  process;
    struct node * next;
}Node;
typedef struct Pqueue{
    Node*head;
}Pqueue;
Node*newNode(struct PCB newprocess){
    Node*temp = (Node*)malloc(sizeof(Node));
    temp->process = newprocess;
    return temp;
}
bool isEmpty(Node**head){
    return ((*head) == NULL);
}
struct PCB *peek(Node**head){
    return &((*head)->process);
}
void dequeue(Node ** head){
    printf("dequeuing process with id = %d with remaining time %d at process scheduler\n",(*head)->process.id,(*head)->process.remaining_time);
    Node*temp = *head;
    (*head) = (*head)->next;
    temp = NULL;
}
void print_queue(Node**head){
    printf("printing ready queue\n");
    Node *start = *head;
    while((start != NULL)){
            printf("id = %d ->",start->process.id);
            start = start -> next;
        }
        printf("NULL\n");
}
bool count_queue(Node**head,int n){
    printf("counter started\n");
    int l = 0;
    Node *start = *head;
    while((start != NULL)){
            start = start -> next;
            l++;
        }
    printf("counter ended %d\n",(l == n));
    print_queue(head);
    return l == n;
}
void enqueue_priority(Node**head,struct PCB process){
    Node * start = (*head);
    Node * temp = newNode(process);
    if((*head) == NULL){
        (*head) = temp;
        printf("new head process with id = %d with remaining time %d at process scheduler\n",(*head)->process.id,(*head)->process.remaining_time);
        return;
    }
    else if((*head)->process.priority > process.priority){
        temp->next = *head;
        (*head) = temp;
    }
    else{
        while((start->next != NULL) && (start->next->process.priority < process.priority)){
            start = start -> next;
        }
        temp -> next = start -> next;
        start -> next = temp;
    }
    printf("new head process with id = %d with remaining time %d at process scheduler\n",(*head)->process.id,(*head)->process.remaining_time);
}
void enqueue_sjf(Node**head,struct PCB process){
    printf("received new process with id = %d with remaining time %d at process scheduler\n",process.id,process.remaining_time);
    Node * start = (*head);
    Node * temp = newNode(process);
    if((*head) == NULL){
        *head = temp;
        printf("new head process with id = %d with remaining time %d at process scheduler\n",(*head)->process.id,(*head)->process.remaining_time);
        return;
    }
    else if((*head)->process.runningtime > process.runningtime){
        temp->next = *head;
        (*head) = temp;
    }
    else{
        while((start->next != NULL) && (start->next->process.runningtime < process.runningtime)){
            start = start -> next;
        }
        temp -> next = start -> next;
        start -> next = temp;
    }
    printf("new head process with id = %d with remaining time %d at process scheduler\n",(*head)->process.id,(*head)->process.remaining_time);
}
void enqueue_srt(Node**head,struct PCB process){
    Node * start = (*head);
    Node * temp = newNode(process);
    if((*head) == NULL){
        (*head) = temp;
        temp->next = NULL;
        return;
    }
    else if((*head)->process.remaining_time > process.remaining_time){
        temp->next = *head;
        (*head) = temp;
    }
    else{
        while((start->next != NULL) && (start->next->process.remaining_time < process.remaining_time)){
            start = start -> next;
        }
        temp -> next = start -> next;
        start -> next = temp;
    }
}
void enqueue_rr(Node**head,struct PCB process){
    printf("received new process with id = %d with remaining time %d at process scheduler\n",process.id,process.remaining_time);
    Node * start = (*head);
    Node * temp = newNode(process);
    if((*head) == NULL){
        *head = temp;
        printf("new head process with id = %d with remaining time %d at process scheduler --- 1\n",(*head)->process.id,(*head)->process.remaining_time);
        print_queue(head);
        return;
    }
    else{
        while(start->next != NULL){
            start = start -> next;
        }
        temp -> next = start -> next;
        start -> next = temp;
    }
    print_queue(head);
    printf("new head process with id = %d with remaining time %d at process scheduler --- 2\n",(*head)->process.id,(*head)->process.remaining_time);
}
//------------------------------------------
//datastructures required
int last_level;
int time_passed;
int working;
int pid;
struct algorithm algo;
key_t remaining;
key_t receiving;
key_t shm_pid;
Node * ready_queue;
Node * blocked_queue;
Node * finished;
struct PCB* running; //points to the running process
struct PCB received_process;
struct PCB process_from_blocked_queue;
int shmid;
int qid;
int *shmaddr_process;
int shm_pid_id;
int *shmaddr_pid;
Node* MFL_queue [11];
bool is_generator_done;
//------------------------------------------
void myhandler(int);
void myhandler2(int);
void process_generator_end(int);
//------------------------------------------
void SJF(int x, char *argv[]);
void SRT(int x, char *argv[]);
void HPF(int x, char *argv[]);
void HPF_preemptive(int x, char *argv[]);
void RR(int x, char *argv[]);
void MFL(int x, char *argv[]);
//------------------------------------------

struct Tnode* memory;
  FILE*memoryfile;
   

struct Tnode* allocateprocess(struct PCB process)
{
    int start;
    struct Tnode* allocatednode=Allocate(&memory,process.memsize,&start);
    if (allocatednode!=NULL){
        fprintf(memoryfile,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),process.memsize,process.id,allocatednode->start,allocatednode->end);
           process.startadd=allocatednode->start;
           process.endadd=allocatednode->end;
    }
    return allocatednode;
}

void deallocateprocess(struct PCB process,int start,int end)
{
    deallocation(&memory,start,end);
    fprintf(memoryfile,"At time %d freed %d bytes from process %d from %d to %d\n",getClk(),process.memsize,process.id,start,end);
}

//------------------------------------------

int main(int argc, char *argv[])
{
    fopen("memoryfile.txt","w");
    signal(SIGUSR1,process_generator_end);
    signal(SIGUSR2,myhandler2);
    //signal(SIGCHLD,processend);
    //-------------------------------------------
    printf("scheduler started\n");
    working = 0;
    last_level = 0;
    receiving = ftok("lab.txt",1);
    remaining = ftok("lab.txt",22);
    shm_pid = ftok("lab.txt",3);
    shmid = shmget(remaining, sizeof(int), IPC_CREAT | 0644);
    shm_pid_id = shmget(shm_pid, sizeof(int), IPC_CREAT | 0644);
    qid = msgget(receiving,0666 | IPC_CREAT);
    shmaddr_process = (int *)shmat(shmid, NULL, 0);
    shmaddr_pid = (int *)shmat(shm_pid_id, NULL, 0);
    //--------------------------------------------
    is_generator_done = false;
    msgrcv(qid,&algo,sizeof(algo),0,!IPC_NOWAIT);
    printf("algorithm received is %s and process count %d\n",algo.name,algo.process_count);
    int finished_queue_count;
    initClk();
    while(1){
        int x = getClk();
        //--------------------------------------
        if(strcmp(algo.name,"SJF") == 0){

            if(algo.preemptive == false){
            SJF(x,argv);   
            }
            else{
            SRT(x,argv); 
            }
        }
        //--------------------------------------
        else if(strcmp(algo.name,"HPF") == 0){
            if(algo.preemptive == false){
            HPF(x,argv);
            }
            else{
            HPF_preemptive(x,argv);
            }
        }
        //--------------------------------------
        else if(strcmp(algo.name,"RR") == 0){
            RR(x,argv);
        }
        //--------------------------------------
        else if(strcmp(algo.name,"MFL") == 0){
            MFL(x,argv);
        }
        //--------------------------------------
          if(!isEmpty(&finished) && is_generator_done && isEmpty(&ready_queue) && count_queue(&finished,algo.process_count)){
             goto finish;
          }
    }
    finish:
    //destroyClk(true);
    print_queue(&finished);
    //kill(getppid(),SIGUSR1);
    printf("working %d\n",working);
    myhandler2(working);
    system("ipcrm --all");
    return 0;
}
void myhandler(int signum){
    msgctl(receiving,IPC_RMID,NULL);
    msgctl(remaining,IPC_RMID,NULL);
    msgctl(qid,IPC_RMID,NULL);
}
void myhandler2(int signum){
    printf("logs are printing now\n");
    ///////////memory//////////
     

    FILE*logs;
    logs = fopen("process_logs.txt","a");
    fprintf(logs,"  id    |   arrival time    |   start time  |   response time   |   burst time  |   finish  time    |     turnaround time     |     weighted turnaround time      |\n");
    struct PCB *process_under_review;
    int turnaround_time = 0;
    int response_time = 0;
    int total_finish_time = 0;
    int total_turnaround_time = 0;
    int total_response_time =0;
    int total_work_time = getClk();
    int num_of_processes = 0;
    float weighted_turnaround_time =0;
    float total_weighted_turnaround_time =0;
    while(finished != NULL){
        num_of_processes++;
        process_under_review = peek(&finished);
        dequeue(&finished);
        turnaround_time = ((float)process_under_review->finishtime-(float)process_under_review->arrivaltime);
        response_time = ((float)process_under_review->start_time - (float)process_under_review->arrivaltime);
        total_turnaround_time += (float)turnaround_time;
        total_finish_time += (float)process_under_review->finishtime;
        total_response_time += (float)response_time;
        weighted_turnaround_time = ((float)turnaround_time/(float)process_under_review->runningtime);
        fprintf(logs,"  %d        %d        %d      %d       %d      %d        %d       %f\n",process_under_review->id,process_under_review->arrivaltime,process_under_review->start_time,response_time,process_under_review->runningtime,process_under_review->finishtime,turnaround_time,(float)weighted_turnaround_time);
    }
    float average_turnaround = ((float)total_turnaround_time / (float)num_of_processes);
    float average_finish_time = ((float)total_finish_time / (float)num_of_processes);
    float average_response_time = ((float)total_response_time / (float)num_of_processes);
    float average_weighted_turnaround_time = ((float)total_weighted_turnaround_time / (float)num_of_processes);
    float CPU_Utilization = (((float)working/(float)total_work_time)*100);
    fprintf(logs,"average turnaround time = %f\n",(float)average_turnaround);
    fprintf(logs,"average weighted turnaround time = %f\n",(float)average_weighted_turnaround_time);
    fprintf(logs,"average finish time = %f\n",(float)average_finish_time);
    fprintf(logs,"average response time = %f\n",(float)average_response_time);
    fprintf(logs,"CPU Utilization = %f\n",(float)CPU_Utilization);
    fclose(logs); 
    kill(getppid(),SIGUSR1);
}
void process_generator_end(int signum){
        is_generator_done = true;
        printf("process generator is done\n");
}
//----------------------------------------------
//SJF
void SJF(int x, char *argv[]){
    int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        enqueue_sjf(&ready_queue,received_process);
        
    }
     if((running != NULL)){//continue running condition implementation
         if(*shmaddr_process == 0){
             running->finishtime = x;
             enqueue_rr(&finished,*running);
             //deallocate the process from memory
             deallocateprocess(running,running->startadd,running->endadd);
             running = NULL;
             if(!isEmpty(&ready_queue)){
             running = peek(&ready_queue);
             dequeue(&ready_queue);
             running->finishtime = x  + 1;
             printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                //call function allocation of memory
                allocateprocess(running);
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;

           }
         }
     }
    else if(running == NULL){
        if(!isEmpty(&ready_queue)){
            running = (peek(&ready_queue));
            dequeue(&ready_queue);
            running->finishtime = x + running->runningtime + 1;
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
        }
    }
    if(running != NULL){
        working += 1;
    }
    while(x == getClk()){}
}
//----------------------------------------------
//SRT
void SRT(int x, char *argv[]){
    int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        enqueue_sjf(&ready_queue,received_process);
    }
    if((running != NULL)){//continue running condition implementation
        if((*shmaddr_process == 0)){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            //deallocate the process from memory
            running = NULL;
            if(!isEmpty(&ready_queue)){
            if(peek(&ready_queue)->remaining_time != peek(&ready_queue)->runningtime){
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&ready_queue)->remaining_time == peek(&ready_queue)->runningtime){
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                  //call function allocation of memory
                  allocateprocess(running);
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
        
          }
        }
        else if(!isEmpty(&ready_queue) && (*shmaddr_process != 0)){
            running->remaining_time = *shmaddr_process;
        if(peek(&ready_queue)->remaining_time < running->remaining_time){
            if(peek(&ready_queue)->remaining_time != peek(&ready_queue)->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_sjf(&ready_queue,*running);
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            }
            else if(peek(&ready_queue)->remaining_time == peek(&ready_queue)->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_sjf(&ready_queue,*running);
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
        }
      }
    }
    else if(running == NULL){
        if(!isEmpty(&ready_queue)){
            running = (peek(&ready_queue));
            dequeue(&ready_queue);
            running->finishtime = x + running->runningtime + 1;
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
        }
    }
    if(running != NULL){
        working += 1;
    }
    while(x == getClk());
}
//---------------------------------------------
//basic ondition for running process control transfer -> ((running->runningtime - running->remaining_time) %algo.Quantum) == 0
//RR
void RR(int x, char *argv[]){
    int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        enqueue_rr(&ready_queue,received_process);
    }
    if((running != NULL)){//continue running condition implementation
        running->remaining_time = *shmaddr_process;
        if((*shmaddr_process == 0)){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            //deallocate the process from memory
            running = NULL;
            if(!isEmpty(&ready_queue)){
            if(peek(&ready_queue)->remaining_time != peek(&ready_queue)->runningtime){
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            time_passed = getClk();
            }
            else if(peek(&ready_queue)->remaining_time == peek(&ready_queue)->runningtime){
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             time_passed = getClk();
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
        
          }
        }else if(!isEmpty(&ready_queue) &&  ((getClk() % algo.Quantum) == 0)){
            printf("the current time = %d and is mod %d\n",getClk(),(getClk()) % algo.Quantum);
            if(peek(&ready_queue)->remaining_time != peek(&ready_queue)->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_rr(&ready_queue,*running);
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            kill(running->pid,SIGCONT);
            time_passed = getClk();
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&ready_queue)->remaining_time == peek(&ready_queue)->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_rr(&ready_queue,*running);
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
             time_passed = getClk();
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
      }
    }
    else if(running == NULL){
        if(!isEmpty(&ready_queue)){
            running = (peek(&ready_queue));
            dequeue(&ready_queue);
            //running->finishtime = x + running->runningtime + 1;
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            time_passed = getClk();
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
        }
    }
    if(running != NULL){
        working += 1;
    }
    while(x == getClk()){
    };
    print_queue(&ready_queue);
    printf("current time is %d\n",getClk());
}
//-----------------------------------------------
//HPF non preemptive
void HPF(int x, char *argv[]){
    int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        enqueue_priority(&ready_queue,received_process);
        
    }
     if((running != NULL)){//continue running condition implementation
         if(*shmaddr_process == 0){
            printf("process possible dequeue 1\n");
             running->finishtime = x;
             enqueue_rr(&finished,*running);
             //deallocate the process from memory
             running = NULL;
             if(!isEmpty(&ready_queue)){
             running = peek(&ready_queue);
             dequeue(&ready_queue);
             //running->finishtime = x + 1;
             printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
           }
         }
     }
    else if(running == NULL){
        if(!isEmpty(&ready_queue)){
            printf("process possible dequeue 2\n");
            running = (peek(&ready_queue));
            dequeue(&ready_queue);
            running->finishtime = x + running->runningtime + 1;
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
        }
    }
    if(running != NULL){
        working += 1;
    }
    while(x == getClk());
}
//--------------------------------------------------------
//HPF preemptive
void HPF_preemptive(int x, char *argv[]){
    int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        enqueue_priority(&ready_queue,received_process);
    }
    if((running != NULL)){//continue running condition implementation
        if((*shmaddr_process == 0)){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            //deallocate the process from memory
            running = NULL;
            if(!isEmpty(&ready_queue)){
            if(peek(&ready_queue)->remaining_time != peek(&ready_queue)->runningtime){
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&ready_queue)->remaining_time == peek(&ready_queue)->runningtime){
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
        
          }
        }else if(!isEmpty(&ready_queue) && (*shmaddr_process != 0)){
        if(peek(&ready_queue)->priority < running->priority){
            if(peek(&ready_queue)->remaining_time != peek(&ready_queue)->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_priority(&ready_queue,*running);
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            kill(running->pid,SIGCONT);
            }
            else if((peek(&ready_queue)->remaining_time == peek(&ready_queue)->runningtime)){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_priority(&ready_queue,*running);
            running = peek(&ready_queue);
            dequeue(&ready_queue);
            printf("-------------------------------------------------------");
             printf("preemptively started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
        }
      }
    }
    else if(running == NULL){
        if(!isEmpty(&ready_queue)){
            running = (peek(&ready_queue));
            dequeue(&ready_queue);
            running->finishtime = x + running->runningtime + 1;
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
        }
    }
    if(running != NULL){
        working += 1;
    }
    while(x == getClk());
}

void MFL(int x, char *argv[]){
//char *argv = {"process",NULL};
x = getClk();
int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        int p=received_process.priority;        
        enqueue_rr(&MFL_queue[p],received_process);
        printf("process id %d is in  %d\n",received_process.id,p);
    }

   for(int i = 0; i < 11;i++){
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        if(!isEmpty(&MFL_queue[i])){
            if(running != NULL){
            //------------------------------------------------------------------------------------    
            if(*shmaddr_process == 0){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            //deallocate the process from memory
            running = NULL;
            if(peek(&MFL_queue[i])->remaining_time != peek(&MFL_queue[i])->runningtime){
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            last_level = i;
            break;
            }
            else if(peek(&MFL_queue[i])->remaining_time == peek(&MFL_queue[i])->runningtime){
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
            last_level = i; 
            break;
            }
            }
            //-------------------------------------------------------------------
            else if((((*shmaddr_process)!=(running->runningtime)) && (((running->runningtime)-(*shmaddr_process))% algo.Quantum == 0 )) || ((running->remaining_time)==0)){
            //printf("the current time = %d and is mod %d\n",getClk(),(getClk()) % algo.Quantum);
            if(peek(&MFL_queue[i])->remaining_time != peek(&MFL_queue[i])->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;

            //you have to check if last level was 10 or not
            if(last_level<10){
                 enqueue_rr(&MFL_queue[last_level + 1],*running);
            }else {
                 enqueue_rr(&MFL_queue[10],*running);
            }
         
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            last_level = i;
            break;
            }
         else if(peek(&MFL_queue[i])->remaining_time == peek(&MFL_queue[i])->runningtime){
            
             kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            //running = NULL;
            //you have to check if last level was 10 or not
            if(last_level<10){
                 enqueue_rr(&MFL_queue[last_level + 1],*running);
            }else {
                 enqueue_rr(&MFL_queue[10],*running);
            }
           
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            time_passed = getClk();
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
            last_level = i;
            break;
            }
            }
            //--------------------------------------------------------------------------------------------------------------------
            }
            //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            else if(running == NULL){
            if(peek(&MFL_queue[i])->remaining_time != peek(&MFL_queue[i])->runningtime){
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            last_level = i;
            break;
            }
            else if(peek(&MFL_queue[i])->remaining_time == peek(&MFL_queue[i])->runningtime){
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                  //call function allocation of memory
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
            last_level = i; 
            break;
            
            //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
               }
            }
         else if(running != NULL){
            printf("process not pointed to\n");
            if(*shmaddr_process == 0){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            //deallocate the process from memory
            //running = NULL;
            }else if((((*shmaddr_process)!=(running->runningtime)) && (((running->runningtime)-(*shmaddr_process))% algo.Quantum == 0 )) || ((running->remaining_time)==0)){
            //printf("the current time = %d and is mod %d\n",getClk(),(getClk()) % algo.Quantum);
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            running = NULL;
            //you have to check if last level was 10 or not
            if(last_level<10){
                 printf("shifting from level %d to %d\n",last_level+1,last_level);
                 enqueue_rr(&MFL_queue[last_level + 1],*running);
                 //last_level++;
            }else {
                printf("working process id %d in level 10 with remaining time %d\n",running->id,*shmaddr_process);
                enqueue_rr(&MFL_queue[10],*running);
            }
            }
            break;
        }        
        //---------------------------------------------------------------------------------------------------------------------------------
     }
     else if((running != NULL) && (i == 10)){
        if(*shmaddr_process == 0){
        running->finishtime = x;
        enqueue_rr(&finished,*running);
        
        running = NULL;
        break;
        }
     }
   }
   if(running != NULL){
        working += 1;
    }
    while(x == getClk());
}









//-----------------------------------------------
//At beginning: find a way of commmunication between scheduler and process generator to put new processes in the ready queue which will have different structure according to the type of algorithm


    //Notes about processes:  1)Creation of a process: by fork() then if we're in child call the functions in process.c 
    //                        2)Stop a process: a)call function currentstate in process.c b)save the state c)kill that child(that doesn't mean that the process has completed as the scheduler still has its PCB)

    //Notes about any algorithm:  1)any algorithm will be working in a while(1){} where the first line in this loop is to check whether there's a new processs arrived or not (message queue between scheduler and process generator)
    //                            2)when an algorithm selects a new process to run: a)put the PCB of runnning in the ready queue but check if its not null first 
    //                                                                              b)running=PCB of the selected process
    
    
    /*
    int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
if(temp != -1){
        int p=received_process.priority;        
        enqueue_rr(&MFL_queue[p],received_process);
        printf("process id %d is in  %d\n",received_process.id,p);
     }

    for(int i = 0; i < 11;i++){
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        if(!isEmpty(&MFL_queue[i])){
            if(running != NULL){
            //------------------------------------------------------------------------------------    
            if(*shmaddr_process == 0){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            running = NULL;
            if(peek(&MFL_queue[i])->remaining_time != peek(&MFL_queue[i])->runningtime){
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            last_level = i;
            break;
            }
            else if(peek(&MFL_queue[i])->remaining_time == peek(&MFL_queue[i])->runningtime){
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
            last_level = i; 
            break;
            }
            }
            //-------------------------------------------------------------------
            else if((getClk() % algo.Quantum) == 0){
            //printf("the current time = %d and is mod %d\n",getClk(),(getClk()) % algo.Quantum);
            if(peek(&MFL_queue[i])->remaining_time != peek(&MFL_queue[i])->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_rr(&MFL_queue[last_level + 1],*running);
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            kill(running->pid,SIGCONT);
            time_passed = getClk();
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            last_level = i;
            break;
            }
            else if(peek(&MFL_queue[i])->remaining_time == peek(&MFL_queue[i])->runningtime){
            kill(running->pid,SIGSTOP);
            running->remaining_time = *shmaddr_process;
            enqueue_sjf(&MFL_queue[i],*running);
            running = peek(&MFL_queue[i]);
            dequeue(&MFL_queue[i]);
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            time_passed = getClk();
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
            last_level = i;
            break;
            }
            }
            //--------------------------------------------------------------------------------------------------------------------
            }
            //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            else if(running == NULL){
            running = (peek(&MFL_queue[i]));
            dequeue(&MFL_queue[i]);
            //running->finishtime = x + running->runningtime + 1;
            printf("-------------------------------------------------------");
            printf("started new process with id = %d and remaining time = %d at level %d",running->id,running->remaining_time,i);
            printf("-------------------------------------------------------\n");
            char *args[] = {"process",NULL};
            *shmaddr_process = running->remaining_time;
            pid = fork();
            if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
            }
            running->pid = pid;
            running->start_time = x;
            printf("the process pid is %d\n",running->pid);
            last_level = i;
            break;
            }
            //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        }
        else if((running != NULL) && (*shmaddr_process == 0)){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            running = NULL;
        }
        else if(running != NULL && ((getClk() % algo.Quantum) == 0) && ((i+1) == 11)){
            if(last_level == 10){
                continue;
                }
            else{
                printf("shifted from priority %d to priority %d",last_level-1,last_level);
                last_level +=1;
            }
        }
        //---------------------------------------------------------------------------------------------------------------------------------
    }
    if(running != NULL){
        working += 1;
    }
    while(x == getClk());
    */
    
    //TODO: implement the scheduler.

    //TODO: upon termination release the clock resources.
/*
MFL(int x ,char*argv[]){
    x = getClk();
int temp = msgrcv(qid,&received_process,sizeof(received_process),0,IPC_NOWAIT);
    if(temp != -1){
        int p=received_process.priority;
        
         enqueue_rr(&MFL_queue[p],received_process);
         printf("queue %d is %d\n",p,isEmpty(&MFL_queue[p]));
     }

 if((running != NULL)){
        running->remaining_time = *shmaddr_process;//continue running condition implementation
        if(*shmaddr_process == 0){
            running->finishtime = x;
            enqueue_rr(&finished,*running);
            running = NULL;
            if(!isEmpty(&MFL_queue[0])){
            running = peek(&MFL_queue[0]);
            dequeue(&MFL_queue[0]);
            pid = fork();
            if(pid == 0){
                execv("process.c",running->remaining_time);
            }
            running->pid = pid;
            running->start_time = x;
            }
            else if(!isEmpty(&MFL_queue[1])){
            if(peek(&MFL_queue[1])->remaining_time != peek(&MFL_queue[1])->runningtime){
            running = peek(&MFL_queue[1]);
            dequeue(&MFL_queue[1]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[1])->remaining_time == peek(&MFL_queue[1])->runningtime){
            running = peek(&MFL_queue[1]);
            dequeue(&MFL_queue[1]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[2])){
            if(peek(&MFL_queue[2])->remaining_time != peek(&MFL_queue[2])->runningtime){
            running = peek(&MFL_queue[2]);
            dequeue(&MFL_queue[2]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[2])->remaining_time == peek(&MFL_queue[2])->runningtime){
            running = peek(&MFL_queue[2]);
            dequeue(&MFL_queue[2]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[3])){
            if(peek(&MFL_queue[3])->remaining_time != peek(&MFL_queue[3])->runningtime){
            running = peek(&MFL_queue[3]);
            dequeue(&MFL_queue[3]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[3])->remaining_time == peek(&MFL_queue[3])->runningtime){
            running = peek(&MFL_queue[3]);
            dequeue(&MFL_queue[3]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[4])){
            if(peek(&MFL_queue[4])->remaining_time != peek(&MFL_queue[4])->runningtime){
            running = peek(&MFL_queue[4]);
            dequeue(&MFL_queue[4]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[4])->remaining_time == peek(&MFL_queue[4])->runningtime){
            running = peek(&MFL_queue[4]);
            dequeue(&MFL_queue[4]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[5])){
            if(peek(&MFL_queue[5])->remaining_time != peek(&MFL_queue[5])->runningtime){
            running = peek(&MFL_queue[5]);
            dequeue(&MFL_queue[5]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[5])->remaining_time == peek(&MFL_queue[5])->runningtime){
            running = peek(&MFL_queue[5]);
            dequeue(&MFL_queue[5]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[6])){
            if(peek(&MFL_queue[6])->remaining_time != peek(&MFL_queue[6])->runningtime){
            running = peek(&MFL_queue[6]);
            dequeue(&MFL_queue[6]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[6])->remaining_time == peek(&MFL_queue[6])->runningtime){
            running = peek(&MFL_queue[6]);
            dequeue(&MFL_queue[6]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[7])){
            if(peek(&MFL_queue[7])->remaining_time != peek(&MFL_queue[7])->runningtime){
            running = peek(&MFL_queue[7]);
            dequeue(&MFL_queue[7]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[7])->remaining_time == peek(&MFL_queue[7])->runningtime){
            running = peek(&MFL_queue[7]);
            dequeue(&MFL_queue[7]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[8])){
            if(peek(&MFL_queue[8])->remaining_time != peek(&MFL_queue[8])->runningtime){
            running = peek(&MFL_queue[8]);
            dequeue(&MFL_queue[8]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[8])->remaining_time == peek(&MFL_queue[8])->runningtime){
            running = peek(&MFL_queue[8]);
            dequeue(&MFL_queue[8]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[9])){
            if(peek(&MFL_queue[9])->remaining_time != peek(&MFL_queue[9])->runningtime){
            running = peek(&MFL_queue[9]);
            dequeue(&MFL_queue[9]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[9])->remaining_time == peek(&MFL_queue[9])->runningtime){
            running = peek(&MFL_queue[9]);
            dequeue(&MFL_queue[9]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[10])){
            if(peek(&MFL_queue[10])->remaining_time != peek(&MFL_queue[10])->runningtime){
            running = peek(&MFL_queue[10]);
            dequeue(&MFL_queue[10]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[10])->remaining_time == peek(&MFL_queue[10])->runningtime){
            running = peek(&MFL_queue[10]);
            dequeue(&MFL_queue[10]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
           
      

        }else if((getClk() % algo.Quantum) == 0){
        printf("running time = %d and remaining time = %d",running->runningtime,running->remaining_time);
        int number_of_Q=(running->runningtime - running->remaining_time) / algo.Quantum;
        printf("num of Qs = %d",number_of_Q);
        int indicator=number_of_Q+(running->priority);
        printf("indicator = %d",indicator);
        if(running->priority >= 10){
            enqueue_rr(&MFL_queue[10],*running);

        }else {
            printf("shidting process id %d from priority %d to proiority %d\n",running->id,running->priority,indicator);
            enqueue_rr(&MFL_queue[indicator],*running);
            
        }
        kill(running->pid,SIGSTOP);
        running->remaining_time = *shmaddr_process;
        if(!isEmpty(&MFL_queue[0])){
            running = peek(&MFL_queue[0]);
            dequeue(&MFL_queue[0]);
            pid = fork();
            if(pid == 0){
                execv("process.c",running->remaining_time);
            }
            running->pid = pid;
            running->start_time = x;
            }
            else if(!isEmpty(&MFL_queue[1])){
            if(peek(&MFL_queue[1])->remaining_time != peek(&MFL_queue[1])->runningtime){
            running = peek(&MFL_queue[1]);
            dequeue(&MFL_queue[1]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[1])->remaining_time == peek(&MFL_queue[1])->runningtime){
            running = peek(&MFL_queue[1]);
            dequeue(&MFL_queue[1]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[2])){
            if(peek(&MFL_queue[2])->remaining_time != peek(&MFL_queue[2])->runningtime){
            running = peek(&MFL_queue[2]);
            dequeue(&MFL_queue[2]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[2])->remaining_time == peek(&MFL_queue[2])->runningtime){
            running = peek(&MFL_queue[2]);
            dequeue(&MFL_queue[2]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[3])){
            if(peek(&MFL_queue[3])->remaining_time != peek(&MFL_queue[3])->runningtime){
            running = peek(&MFL_queue[3]);
            dequeue(&MFL_queue[3]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[3])->remaining_time == peek(&MFL_queue[3])->runningtime){
            running = peek(&MFL_queue[3]);
            dequeue(&MFL_queue[3]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[4])){
            if(peek(&MFL_queue[4])->remaining_time != peek(&MFL_queue[4])->runningtime){
            running = peek(&MFL_queue[4]);
            dequeue(&MFL_queue[4]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[4])->remaining_time == peek(&MFL_queue[4])->runningtime){
            running = peek(&MFL_queue[4]);
            dequeue(&MFL_queue[4]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[5])){
            if(peek(&MFL_queue[5])->remaining_time != peek(&MFL_queue[5])->runningtime){
            running = peek(&MFL_queue[5]);
            dequeue(&MFL_queue[5]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[5])->remaining_time == peek(&MFL_queue[5])->runningtime){
            running = peek(&MFL_queue[5]);
            dequeue(&MFL_queue[5]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[6])){
            if(peek(&MFL_queue[6])->remaining_time != peek(&MFL_queue[6])->runningtime){
            running = peek(&MFL_queue[6]);
            dequeue(&MFL_queue[6]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[6])->remaining_time == peek(&MFL_queue[6])->runningtime){
            running = peek(&MFL_queue[6]);
            dequeue(&MFL_queue[6]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
            else if(!isEmpty(&MFL_queue[7])){
            if(peek(&MFL_queue[7])->remaining_time != peek(&MFL_queue[7])->runningtime){
            running = peek(&MFL_queue[7]);
            dequeue(&MFL_queue[7]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[7])->remaining_time == peek(&MFL_queue[7])->runningtime){
            running = peek(&MFL_queue[7]);
            dequeue(&MFL_queue[7]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[8])){
            if(peek(&MFL_queue[8])->remaining_time != peek(&MFL_queue[8])->runningtime){
            running = peek(&MFL_queue[8]);
            dequeue(&MFL_queue[8]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[8])->remaining_time == peek(&MFL_queue[8])->runningtime){
            running = peek(&MFL_queue[8]);
            dequeue(&MFL_queue[8]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[9])){
            if(peek(&MFL_queue[9])->remaining_time != peek(&MFL_queue[9])->runningtime){
            running = peek(&MFL_queue[9]);
            dequeue(&MFL_queue[9]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[9])->remaining_time == peek(&MFL_queue[9])->runningtime){
            running = peek(&MFL_queue[9]);
            dequeue(&MFL_queue[9]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
            else if(!isEmpty(&MFL_queue[10])){
            if(peek(&MFL_queue[10])->remaining_time != peek(&MFL_queue[10])->runningtime){
            running = peek(&MFL_queue[10]);
            dequeue(&MFL_queue[10]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[10])->remaining_time == peek(&MFL_queue[10])->runningtime){
            running = peek(&MFL_queue[10]);
            dequeue(&MFL_queue[10]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
        }
          
        
    }
}

     else if( running == NULL){
        //printf("creating new \n");
        if(!isEmpty(&MFL_queue[0])){
            running = peek(&MFL_queue[0]);
            dequeue(&MFL_queue[0]);
            *shmaddr_process = running->remaining_time;
            char *args[] = {"process",NULL};
            pid = fork();
            if(pid == 0){
                execv("process.c",args);
            }
            running->pid = pid;
            running->start_time = x;
            }
            else if(!isEmpty(&MFL_queue[1])){
            if(peek(&MFL_queue[1])->remaining_time != peek(&MFL_queue[1])->runningtime){
            running = peek(&MFL_queue[1]);
            dequeue(&MFL_queue[1]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 1",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[1])->remaining_time == peek(&MFL_queue[1])->runningtime){
            running = peek(&MFL_queue[1]);
            dequeue(&MFL_queue[1]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 1",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[2])){
            if(peek(&MFL_queue[2])->remaining_time != peek(&MFL_queue[2])->runningtime){
            running = peek(&MFL_queue[2]);
            dequeue(&MFL_queue[2]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 2",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[2])->remaining_time == peek(&MFL_queue[2])->runningtime){
            running = peek(&MFL_queue[2]);
            dequeue(&MFL_queue[2]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 2",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
            else if(!isEmpty(&MFL_queue[3])){
            if(peek(&MFL_queue[3])->remaining_time != peek(&MFL_queue[3])->runningtime){
            running = peek(&MFL_queue[3]);
            dequeue(&MFL_queue[3]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 3",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[3])->remaining_time == peek(&MFL_queue[3])->runningtime){
            running = peek(&MFL_queue[3]);
            dequeue(&MFL_queue[3]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 3",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[4])){
            if(peek(&MFL_queue[4])->remaining_time != peek(&MFL_queue[4])->runningtime){
            running = peek(&MFL_queue[4]);
            dequeue(&MFL_queue[4]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 4",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[4])->remaining_time == peek(&MFL_queue[4])->runningtime){
            running = peek(&MFL_queue[4]);
            dequeue(&MFL_queue[4]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 4",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[5])){
            if(peek(&MFL_queue[5])->remaining_time != peek(&MFL_queue[5])->runningtime){
            running = peek(&MFL_queue[5]);
            dequeue(&MFL_queue[5]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 5",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[5])->remaining_time == peek(&MFL_queue[5])->runningtime){
            running = peek(&MFL_queue[5]);
            dequeue(&MFL_queue[5]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 5",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[6])){
            if(peek(&MFL_queue[6])->remaining_time != peek(&MFL_queue[6])->runningtime){
            running = peek(&MFL_queue[6]);
            dequeue(&MFL_queue[6]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 6",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[6])->remaining_time == peek(&MFL_queue[6])->runningtime){
            running = peek(&MFL_queue[6]);
            dequeue(&MFL_queue[6]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 6",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[7])){
            if(peek(&MFL_queue[7])->remaining_time != peek(&MFL_queue[7])->runningtime){
            running = peek(&MFL_queue[7]);
            dequeue(&MFL_queue[7]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 7",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[7])->remaining_time == peek(&MFL_queue[7])->runningtime){
            running = peek(&MFL_queue[7]);
            dequeue(&MFL_queue[7]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 7",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[8])){
            if(peek(&MFL_queue[8])->remaining_time != peek(&MFL_queue[8])->runningtime){
            running = peek(&MFL_queue[8]);
            dequeue(&MFL_queue[8]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 8",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[8])->remaining_time == peek(&MFL_queue[8])->runningtime){
            running = peek(&MFL_queue[8]);
            dequeue(&MFL_queue[8]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 8",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[9])){
            if(peek(&MFL_queue[9])->remaining_time != peek(&MFL_queue[9])->runningtime){
            running = peek(&MFL_queue[9]);
            dequeue(&MFL_queue[9]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 9",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[9])->remaining_time == peek(&MFL_queue[9])->runningtime){
            running = peek(&MFL_queue[9]);
            dequeue(&MFL_queue[9]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 9",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }
             else if(!isEmpty(&MFL_queue[10])){
            if(peek(&MFL_queue[10])->remaining_time != peek(&MFL_queue[10])->runningtime){
            running = peek(&MFL_queue[10]);
            dequeue(&MFL_queue[10]);
            *shmaddr_process = running->remaining_time;
            kill(running->pid,SIGCONT);
            printf("the process pid is %d\n",running->pid);
            printf("-------------------------------------------------------");
            printf("restarted process with id = %d and remaining time = %d at 10",running->id,running->remaining_time);
            printf("-------------------------------------------------------\n");
            }
            else if(peek(&MFL_queue[10])->remaining_time == peek(&MFL_queue[10])->runningtime){
            running = peek(&MFL_queue[10]);
            dequeue(&MFL_queue[10]);
            printf("-------------------------------------------------------");
             printf("started new process with id = %d and remaining time = %d at 10",running->id,running->remaining_time);
             printf("-------------------------------------------------------\n");
             char *args[] = {"process",NULL};
             *shmaddr_process = running->remaining_time;
             pid = fork();
             if(pid == 0){
                 int i = execv("./process",args);
                 printf("process status is %d\n",i);
             }
             running->pid = pid;
             running->start_time = x;
             printf("the process pid is %d\n",running->pid); 
            }
            }






    }


//while(x == getClk()){}
 sleep(1);

}
*/