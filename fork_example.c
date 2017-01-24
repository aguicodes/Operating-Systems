#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main (int argc, char *argv[]){
    printf("I am: %d\n", (int) getpid());
    pid_t pid = fork();
    return 0;
}

