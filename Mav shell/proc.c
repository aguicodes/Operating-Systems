/*
 * Name: Graciela Aguilar
 * ID #: 1000717478
 * Programming Assignment 1
 * Description: Proc.c; program displays specific data from the proc
 *              virtual files on linux. proc contains information
 *              about the OS.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void){
    
    /* token will hold the value needed to be printed, it will be overwritten if needed to 
       until it holds the string needed. buffer will hold the current line and stop at \n i
       serves as a loop variable to get to the line desired. file will point to the file
       desired to open. */
    
    char *token;
    char buffer[256];
    int i = 0;
    FILE * file;
    
    
    
    /* The file variable will be overwritten everytime we desire to open a different
       file. */
    
    file = fopen( "/proc/cpuinfo" , "r");
    if (file) {
        while(fgets(buffer, sizeof(buffer), file) != NULL){
            if(i == 4){
                
                /* if file is open, it will loop to the 4th line where the model type is
                   stored at and tokenize to the semi colon to get rid of title of information
                   and then we'll tokenize again for a delimiter of empty space so it won't
                   print the spaces before the information. after it prints it actually tokenizes
                   to only the first word because of the space delimiter. we will have to 
                   print the rest of the buffer and this time tokenizing by the newline to 
                   get the rest of the line. then we break out because we no longer need to 
                   check the rest of the file */
                
                token = strtok(buffer, ":");
                token = strtok(NULL, " ");
                printf("%s", token);
                token = strtok(NULL, "\n");
                printf("%s\n", token);
                break;
            }
            i++;
        }
        fclose(file);
    }
    file = fopen( "/proc/version" , "r");


    /* fgets opens the file and stores current line in buffer, information
       needed is in first line so it prints and breaks out of loop, no need
       to continue reading file. */

    if (file){
        
        while(fgets(buffer, sizeof(buffer), file) != NULL){

            token = strtok(buffer, "(");
            printf("%s\n", token);
            break;
        }
        fclose(file);
    }

    file = fopen( "/proc/meminfo" , "r");
    
    /* fgets opens the file and stores current line in buffer, information
     needed is in first line so it prints and breaks out of loop, no need
     to continue reading file. */

    if (file){
        while(fgets(buffer, sizeof(buffer), file) != NULL){
            printf("%s", buffer);
            break;
        }

        fclose(file);
    }
    
    file = fopen( "/proc/uptime" , "r");
    
    /* fgets opens the file and stores current line in buffer, information
     needed is in first line so it prints and breaks out of loop, no need
     to continue reading file. it needs to be tokenized by a space because
     we only need the time from last boot */
    
    if (file){
        while(fgets(buffer, sizeof(buffer), file) != NULL){
            token = strtok(buffer, " ");
            printf("%s\n", token);
            break;
        }
        fclose(file);
    }
    
    /* if unable to open any of the files it will eventually hit this conditional 
       state something when wrong and exit program */
    else{
        printf("Something went wrong... Exiting\n");
        exit(1);
    }

    return 0;
}
