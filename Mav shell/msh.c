/* 
 * Name: Graciela Aguilar
 * ID #: 1000717478
 * Programming Assignment 1
 * Description: Mav shell; program takes commands and forks into a 
 *              child process, executing those commands using execvp()
 *              for the exec family. also chdir() is used for making
 *              directories or changing directories, etc. program exits
 *              when user types exit/quit otherwise runs in a loop.
 */



/* The MIT License (MIT)
   Copyright (c) 2016, 2017 Trevor Bakker
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE. */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define WHITESPACE " \t\n"    /* We want to split our command line up into tokens
                                 so we need to define what delimits our tokens.
                                 In this case, white space
                                 Will separate the tokens on our command line */
 
#define MAX_COMMAND_SIZE 255  /* The maximum command-line size*/

#define MAX_NUM_ARGUMENTS 11  /* Mav shell only supports 11 arguments (path with max of
                                 ten arguements*/


/* functions are declared before main to be recognized by main but written below main
   to not have so much over head. */
 
/* showpids() function: The function prints the pids array backwards so it can
   print most recent to least recent (pid). It takes an array as an arguement. 
   Because there can possibly be less than ten pids stored (created), if the pid 
   value equals zero that means that particular element has not been modified. Therefore, 
   the loop breaks out once and if it comes across a value of zero. the function is 
   written below main*/

void showpids(int array[]);

/* addpid() function: This function takes an array as an arguement along with an int value
   that needs to be stored in the array. The function shifts everything to the left one position
   and assigns the last element to the new int value desired to store. the elements are hard coded
   to ten values therefore theres no need for a size arguement. */

void addpid (int array [],int  temp);

int main(int argc, char *argv[]){

    /* pids array is hard coded to ten elements all initialized to zero for storing purposes
       of the pids. also as soon as the program is ran it will grab the first pid and store
       it to the last element to begin. */
    int pids [10] = {0};
    pids[9] = (int) getpid ();
    char * cmd_str = (char*) malloc (MAX_COMMAND_SIZE);
 
    /* this while loop will continue to run program until user decides to quit */
    while(1){

        /* Print out the msh prompt */
        printf("msh> ");

        /* Read the command from the commandline.  The
           maximum command that will be read is MAX_COMMAND_SIZE
           This while command will wait here until the user
           inputs something since fgets returns NULL when there
           is no input */
    
        while (!fgets (cmd_str, MAX_COMMAND_SIZE, stdin));

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;
        /* Pointer to point to the token
           parsed by strsep */
        char *arg_ptr;

        char *working_str = strdup(cmd_str);

        /* we are going to move the working_str pointer so
           keep track of its original value so we can deallocate
           the correct amount at the end */
        char *working_root = working_str;
    
        /* Tokenize the input stringswith whitespace used as the delimiter */
        while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) && 
            (token_count<MAX_NUM_ARGUMENTS)){

            token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
           
            if(strlen(token[token_count]) == 0){
                token[token_count] = NULL;
            }

            token_count++;
        }
        /* If user clicks enter it will "continue" and start the loop over and prompt the shell again */
        if(token[0] == NULL){
            continue;
        }
        /* if user types show pid it will call the show pids function and display them, then continue
           to the top of the loop and prompt shell again*/
        else if(strcmp("showpid",token[0]) == 0){
            showpids(pids);
     
            continue;
        }
        /* if user decides to exit program they will type exit or quit leading progam to exit */
        else if((strcmp("exit", token[0]) == 0) || (strcmp("quit", token[0]) == 0)){

            exit(0);
        }
     
        /* if user doesn't want to exit program nor view pids or such, then this is where fork is actually
           needed so we will call fork where a parent pid will spawn a child process  */
        
        pid_t pid = fork();

        /* if fork fails it will return negative number, it will spit out error and exit program. */
        
        if (pid < 0){
            perror("Fork failed.\nExiting...\n");
            exit(1);
        }
        
        /* if the pid is greater than 0 then that means we are in the parent process, we will need to 
           use chdir() function in the parent process because it will not work in the child process.
           chdir() function changes the current directory to the desired one. if successful it will 
           return 0, else -1. If the user wants to change directories and uses the command cd, it will
           take the second token and attempt to move there if it doesn't exist it will throw an error
           message that directory is not found and resume the program leading back to shell prompt. The
           wait(NULL0 in the parent process waits on the child to process, if this is not done, they can 
           run either way which can cause conflict for the conditionals designed in this program. After  
           parent waits we will add the newly spawned pid to our array calling the addpid function passing
           the pid along with the array to store it in. */
         

        else if(pid > 0){
            if(strcmp("cd", token[0]) == 0){
                int ret = chdir(token[1]);
                if (ret == -1){
                    printf("%s: This directory is not found.\n", token[1]);
                }
            }
      
            wait(NULL);
 
            addpid(pids, (int)pid );
  
        }
        
        /* if the pid equals zero, that means we are the child process. execvp() is  going to execute the
           command desired. v stands for taking an array as the second arguement and p stands for allowing
           to search the natural path in the shell. when execvp executes successfully it exits, thats why we
           fork and run it in the child process, because once it kills the child process we can continue in 
           our orginial program. If excevp() fails it will continue to run the code after it. so if it exits 
           we check id the command was cd, if so exit process else, their command was intended for execvp()
           and we need to notify user that it is an invalid command. */
        else if(pid == 0){

            execvp(token[0], token);
            
            if(strcmp("cd", token[0]) == 0){
        
                exit(1);
            }

            printf( "%s: command not found.\n", token[0]);
   
            
            exit(1);
            
        }
        
        /* fflush(NULL) clears the stdin and starts from fresh */

        free(working_root);
        fflush(NULL);
    }
    return 0;
}

void showpids(int array []){
    int i = 0;
    for (i = 9; i >= 0; i--){
        if(array[i] == 0){
            return;
        }
        printf("%d\n", array[i]);
    }
}
void addpid(int array [], int temp){
    int j = 0;
    
    for(j = 0; j < 10; j++){
        array[j] = array[j+1];
    }
    array[9] = temp;
    
}

