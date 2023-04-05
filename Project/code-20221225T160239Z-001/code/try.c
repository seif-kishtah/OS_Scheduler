#include "headers.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

int main(){
    float  x = ((float)8/(float)9);
    printf("%.5f\n",x);
    return 0;
}