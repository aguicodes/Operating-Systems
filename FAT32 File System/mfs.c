/*  Name: Arianne Silvestre & Graciela Aguilar
 *  ID #: 1000921178, 1000717478
 *  Programming Assignment 3
 *  Description: Implement a user space shell application that is capable of
 *  interpreting a FAT32 file system image.
 */


// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11


bool allow = false; //boolean flag that tells whether the file is currently open

//This are the variables of the info that is going to be retrieved
char BS_OEMName[8];
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATS;
int16_t BPB_RootEntCnt;
char  BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

//cd previous will keep track of the cd behaviors
int cd_previous[10] = {0,0,0,0,0,0,0,0,0,0};
int prev = 9; //prev holds the index in the array of the previous directory
int start = 0; //start sets the conditional to see if the array started storing


//Record Struct
struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

struct DirectoryEntry dir[16];


//CWD keeps track of the Current working directory, and PWD keeps track of the previous
int CWD = 0;
int PWD = 0;
//functions are declared in overhead so main can recognize them
void info(FILE *fp);
void stat(FILE *fp, char* input);
void ls(FILE *fp);
void cd(char *s, FILE *fp);
void Read(char *s1, char *s2, char *s3, FILE *fp);
int LBAToOffset(int32_t sector);
void listPrevious(FILE *fp);

//file is made global so any scope can access it
FILE *fp = NULL;

int main(int argc, char *argv[])
{
    char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
    
    while( 1 )
    {
        // Print out the msh prompt
        printf ("mfs> ");
        
        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
        
        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];
        
        int   token_count = 0;
        
        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;
        
        char *working_str  = strdup( cmd_str );
        
        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;
        
        // Tokenize the input stringswith whitespace used as the delimiter
        while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
               (token_count<MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
        }
        
        /*-------------------------------------------------------------------------------------*/
        //If user types a blank line, shell continues and prints another prompt for new
        // line of input.
        if(token[0] == NULL)
            continue;
        // program will terminate if user inputs exit or quit
        if(strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
        {
            exit(0);
        }
        
        //If another command is entered and allow is currently set to false
        //meaning the file isn't open. Print error message.
        if(strcmp(token[0], "open") != 0 && allow == false)
        {
            printf("Error: File image must be opened first.\n");
            continue;
        }
        
        //If open command is entered and allow is currently set to false, then change
        //the state of allow to true to say that the file is now open and open the file
        else if(strcmp(token[0], "open") == 0 && allow == false)
        {
            //open file
            allow = true;
            
            fp = fopen(token[1], "r");
            
            if(!fp)
            {
                allow = false;
            }
            
            // only if file opens, we'll collect data
            if(fp)
            {
                
                //Parse file
                fseek(fp, 3, SEEK_CUR);
                fread(&BS_OEMName, 8, 1, fp);
                fread(&BPB_BytesPerSec, 2, 1, fp);
                fread(&BPB_SecPerClus, 1, 1, fp);
                fread(&BPB_RsvdSecCnt, 2, 1, fp);
                fread(&BPB_NumFATS, 1, 1, fp);
                fread(&BPB_RootEntCnt, 2, 1, fp);
                fseek(fp, 36, SEEK_SET);
                fread(&BPB_FATSz32, 4, 1, fp);
                fseek(fp, 44, SEEK_SET);
                fread(&BPB_RootClus, 4, 1, fp);
                fseek(fp, 71, SEEK_SET);
                fread(BS_VolLab, 11, 1, fp);
                
                // calculates cluster to start at root directory
                RootDirSectors = (BPB_NumFATS * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);
                CWD = RootDirSectors;
                PWD = CWD;
                
                fseek(fp, RootDirSectors, SEEK_SET);
                
                //Populate struct array with records in directory
                int i;
                for(i = 0; i < 16; i++)
                {
                    fread(&dir[i],32, 1, fp);
                }
                
            }
            else
                printf("Error: Unable to open file.\n");
        }
        // file closes when user iputs close
        else if(strcmp(token[0], "close") == 0 && allow == true)
        {
            printf("closed\n");
            allow = false;
            fclose(fp);
        }
        
        //if file is opened start seaerching for matching commands
        if(allow)
        {
            if(strcmp(token[0], "info") == 0)
                info(fp);
            
            if(strcmp(token[0], "stat") == 0)
                stat(fp, token[1]);
            
            if(strcmp(token[0], "ls") == 0)
            {
                //if ls .., print previous dir struct
                if(token[1] != NULL && strcmp(token[1], "..") == 0 )
                    listPrevious(fp);
                else
                    ls(fp);
            }
            /*volume will be typically untitled, name was changed to error check that
             * it was displaying in case of volume name*/
            if(strcmp(token[0],"volume") == 0)
            {
                if((BS_VolLab[0]) == '\0'){
                    printf("Error: volume name not found\n");
                }
                else{
                    printf("%s\n",BS_VolLab);
                }
            }
            if(strcmp(token[0], "cd") == 0)
            {
                cd(token[1],fp);
            }
            if(strcmp(token[0], "read") == 0)
            {
                /*its going to run read function and when it returns its
                 * going to fseek back to the original spot it was before*/
                int i;
                
                Read(token[1], token[2], token[3], fp);
                
                fseek(fp, CWD, SEEK_SET);
                
                for(i = 0; i < 16; i++)
                {
                    fread(&dir[i],32, 1, fp);
                    
                }
                
            }
        }
        free( working_root );
    }
    return 0;
}

/*
 * Function: info
 * Parameters: File pointer
 * Returns: void
 * Description: it prints the info initally retrieved from opening file
 */

void info(FILE *fp)
{
    printf("BPB_BytesPerSec \n  Hex: 0x%X\n  Dec: %d \n", BPB_BytesPerSec, BPB_BytesPerSec);
    printf("BPB_SecPerClus \n  Hex: 0x%X\n  Dec: %d \n", BPB_SecPerClus, BPB_SecPerClus);
    printf("BPB_RsvdSecCnt \n  Hex: 0x%X\n  Dec: %d \n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
    printf("BPB_NumFATS \n  Hex: 0x%X\n  Dec: %d \n", BPB_NumFATS, BPB_NumFATS);
    printf("BPB_FATSz32 \n  Hex: 0x%X\n  Dec: %d \n", BPB_FATSz32, BPB_FATSz32);
    
}

/*
 * Function: stat
 * Parameters: File pointer, and input (second token)
 * Returns: void
 * Description: it loops through dir structure, if it finds the file the user inputed,
 * it will print the details from that stuct, otherwise prints file not found.
 */

void stat(FILE *fp, char* input)
{
    //Convert filename
    char temp_string[12];
    memset(temp_string, 0, 12);
    strncpy(temp_string, "           ", 11);
    char * token = strtok(input, ".");
    strncpy(temp_string, token, strlen(token));
    
    token = strtok(NULL, ".");
    
    //If token does not equal null, there are more characters following ".", meaning it has a file
    //extension, otherwise it's a folder name
    if(token != NULL)
        strncpy(&temp_string[8], token, 3);
    
    int i, j;
    for(i = 0; i < strlen(temp_string); i++)
    {
        temp_string[i] = toupper(temp_string[i]);
    }
    
    //Loop through each record in current directory
    for(j = 0; j < 16; j++)
    {
        char* str = (char *)malloc(12);
        memset(str, 0, 12);
        strncpy(str,dir[j].DIR_Name, 11);
        //If argument matches filename
        if(strcmp(temp_string, str) == 0)
        {
            printf("%s\tAttr: %x\tHigh: %d\tLow: %d\tSize: %d bytes\n", str, dir[j].DIR_Attr, dir[j].DIR_FirstClusterHigh, dir[j].DIR_FirstClusterLow, dir[j].DIR_FileSize);
            return;
        }
        free(str);
    }
    printf("File not found\n");
    
}


/*
 * Function: ls
 * Parameters: File pointer
 * Returns: void
 * Description: prints the files in the directory only if they're not deleted or hidden.
 */

void ls(FILE *fp)
{
    int i;
    for(i = 0; i < 16; i++)
    {
        char *str = (char *)malloc (12);;
        memset(str, 0, 12);
        strncpy(str, dir[i].DIR_Name, 11);
        
        //Archive flag (Slide 14 in FAT32 PDF)
        if(dir[i].DIR_Attr == 16 || dir[i].DIR_Attr == 32)
        {
            //Hides system and hidden files
            if( (str[0] != 46) && (str[0] != -27) )
            {
                char* str = (char *)malloc(12);
                memset(str, 0, 12);
                strncpy(str,dir[i].DIR_Name, 11);
                printf("%s\t%d bytes\n", str, dir[i].DIR_FileSize);
                free(str);
            }
        }
    }
}

/*
 * Function: cd
 * Parameters: string literal and File pointer
 * Returns: void
 * Description: it cds into directory requested or goes back a level depending on second input.
 */

void cd(char *s, FILE *fp)
{
    int i;
    char input[12];
    memset(input, 0, 12);
    strncpy(input,s, 11);
    char *tok;
    
    for(i = 0; i < strlen(input); i++)
    {
        input[i] = toupper(input[i]);
    }
    
    //If second argument is "..", change the directory to the previous folder
    if((strcmp("..",input) == 0) || (strcmp("../",input) == 0))
    {
     
        //if we are not at the end of the array that holds the directories
        if(prev != 9)
            prev++;
        
        if(start == 0)
        {
            start++;
            fseek(fp, CWD, SEEK_SET);
        }
        else
            fseek(fp, cd_previous[prev], SEEK_SET);
        
        for(i = 0; i < 16; i++)
        {
            fread(&dir[i],32, 1, fp);
        }
        
        
        return;
    }
    
    for(i = 0; i < 16; i++)
    {
        //Archive flag (Slide 14 in FAT32 PDF)
        if(dir[i].DIR_Attr == 16 || dir[i].DIR_Attr == 32)
        {
            char* str = (char *)malloc(12);
            memset(str, 0, 12);
            strncpy(str,dir[i].DIR_Name, 11);
            
            tok = strtok(str, " ");
            
            if(strcmp(tok,input) == 0)
            {
                PWD = CWD;
                
                cd_previous[prev] = CWD;
                if(prev != 0){
                    prev--;
                }
                CWD = LBAToOffset(dir[i].DIR_FirstClusterLow);
                fseek(fp, CWD, SEEK_SET);
                
                for(i = 0; i < 16; i++)
                {
                    fread(&dir[i], 32, 1, fp);
                }
                return;
            }
            free(str);
        }
    }
    printf("Error: directory does not exist\n");
}
/*
 * Function: listPrevious
 * Parameters: File pointer
 * Returns: void
 * Description: it fseeks to previous directory and prints and fseeks back to current then returns
 */

void listPrevious(FILE *fp){
    int i;
    fseek(fp, PWD, SEEK_SET);
    for(i = 0; i < 16; i++)
    {
        fread(&dir[i],32, 1, fp);
        
    }
    for(i = 0; i < 16; i++)
    {
        char *str = (char *)malloc (12);;
        memset(str, 0, 12);
        strncpy(str, dir[i].DIR_Name, 11);
        
        //Archive flag (Slide 14 in FAT32 PDF)
        if(dir[i].DIR_Attr == 16 || dir[i].DIR_Attr == 32)
        {
            if( (str[0] != 46) && (str[0] != -27) )
            {
                char* str = (char *)malloc(12);
                memset(str, 0, 12);
                strncpy(str,dir[i].DIR_Name, 11);
                printf("\n%s\t\nAttr: %d\nbytes: %d\n\n", str, dir[i].DIR_Attr, dir[i].DIR_FileSize);
                free(str);
            }
        }
    }
    fseek(fp, CWD, SEEK_SET);
    for(i = 0; i < 16; i++)
    {
        fread(&dir[i],32, 1, fp);
        
    }
}
/*
 * Function: LBAToOffset
 * Parameters: 32 bit integer number
 * Returns: void
 * Description: it returns the position to fseek into the desired file location
 */

int LBAToOffset(int32_t sector)
{
    
    return( (sector - 2)* BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) +
    (BPB_BytesPerSec * BPB_NumFATS * BPB_FATSz32);
}
/*
 * Function: Read
 * Parameters: 3 string literals and a file pointer
 * Returns: void
 * Description: it reads the file name, offsets a position, and reads that number of bytes
 */


void Read(char *s1, char *s2, char *s3, FILE *fp)
{
    if(s1 == NULL || s2 == NULL || s3 == NULL)
    {
        printf("Error: invalid number of commands\n");
        return;
    }
    
    char temp_string[12];
    memset(temp_string, 0, 12);
    strncpy(temp_string, "           ", 11);
    char * token = strtok(s1, ".");
    strncpy(temp_string, token, strlen(token));
    int temp_wd;
    int position = atoi(s2);
    int bytes = atoi(s3);
    int8_t byte_values = 0;
    
    token = strtok(NULL, ".");
    
    //If token does not equal null, there are more characters following ".", meaning it has a file
    //extension, otherwise it's a folder name
    if(token != NULL)
        strncpy(&temp_string[8], token, 3);
    
    int i, j;
    for(i = 0; i < strlen(temp_string); i++)
    {
        temp_string[i] = toupper(temp_string[i]);
    }
    //If argument matches filename
    for(j = 0; j < 16; j++)
    {
        char* str = (char *)malloc(12);
        memset(str, 0, 12);
        strncpy(str,dir[j].DIR_Name, 11);
        if(strcmp(temp_string, str) == 0)
        {
            temp_wd =  LBAToOffset(dir[j].DIR_FirstClusterLow);
            
            fseek(fp, temp_wd, SEEK_SET);
            fseek(fp, position, SEEK_CUR);
            
            for(i = 0; i < bytes; i++){
                fread(&byte_values, 1, 1, fp);
                printf("0x%X\n", byte_values);
            }
            
            return;
        }
        free(str);
    }
    printf("Error: filename not found\n");
    return;
}
