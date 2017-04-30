/*  Name: Arianne Silvestre & Graciela Aguilar
 *  ID #: 1000921178, 1000717478
 *  Programming Assignment 4
 *  Description: TBC
 *  
 */
// The MIT License (MIT)
//
// Copyright (c) 2017 Trevor Bakker
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

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h> 

// gcc ksf.c -o ksf -pthread
// in ubuntu

int fuelages, tanks, engines;

sem_t e_parts;
sem_t f_parts;
sem_t t_parts;
int counter = 0;
sem_t launchpad;

bool resources_available();
void *SpaceFacility(void *thread_id);

/* ideally, bool function should not be used, pthread_cond_t should be preferably implemented.
 * Also, (ideally) each thread should have 8-9 states (variable states; int, or long, etc.)
 * depending on which state, it'll print to screen what it's doing (maybe switch statement ana-
 * lzing the states?) at the moment. 
 *
 * The concept is here, but i don't think its the style he wants this to be implemented with.
 * based on his lectures...
 */
int main(int argc, char* argv[])
{
	//i used longs bc he did in an example, don't think they're required, could be just ints
	long fuelages_total = 0;
	long tanks_total = 0;
	long engines_total = 0;
	int kerbals;

	engines_total = (long) atoi(argv[1]);
    	fuelages_total = (long)atoi(argv[2]);
    	tanks_total = (long)atoi(argv[3]);
    	kerbals = atoi(argv[4]);

	int rc;
	long i;

    	sem_init(&e_parts, 0, engines_total);
	sem_init(&f_parts, 0, fuelages_total);
	sem_init(&t_parts, 0, tanks_total);

	sem_init(&launchpad, 0 , 1); //initalizes launchpad to 1 (meaning not in use)	

	pthread_t threads[kerbals];
	
	for(i = 1; i <= kerbals; i++)
	{
		rc = pthread_create(&threads[i], NULL, SpaceFacility, (void*) i);
		if(rc)
		{
			printf("Error; return code in pthread_create() is %d\n", rc);
		}
		//threads need to be joined at the same time after created; separate loop
	}
	for(i = 1; i <= kerbals; i++)
	{
		pthread_join(threads[i], NULL);
	}

	//this will never run when loop is applied

	pthread_exit(NULL);
	sem_destroy(&e_parts);
 	sem_destroy(&f_parts);
	sem_destroy(&t_parts);
	sem_destroy(&launchpad);
     
}
void *SpaceFacility(void *thread_id)
{
	long t_id;
	t_id = (long) thread_id;

	int value;
	sem_getvalue(&launchpad, &value);

	//TODO print if waiting for resources 

	//ideally should be in an endless while loop

	if(resources_available()){

		printf("Kerbal %ld: Enters into the assembly facility\n", t_id);

		sem_wait(&e_parts);
		sem_wait(&f_parts);
		sem_wait(&t_parts);
		
		
		printf("Kerbal %ld: Assembles Engine %d, ",t_id, engines);
		printf("Fuelage %d, Fuel Tank %d\n", fuelages, tanks);
		sleep(5);
		sem_post(&e_parts);
		sem_post(&f_parts);
		sem_post(&t_parts);
		
	}
    	
	//bug: if not assembled, thread should not head to launching pad
	//doesn't disassemble yet
    	if(counter)//if first time waiting, does not need to occur
		printf("Thread %ld: Waiting for launch pad\n", t_id);
  
	sem_wait(&launchpad); /* down semaphore */ 

    	/* START CRITICAL REGION */
    	printf("Kerbal %ld: Arriving at launch pad\n", t_id);
	
	counter = 10;
	while(counter){
		printf("Kerbal %ld: Launch countdown %d . . .\n", t_id, counter);
		counter--;
		sleep(1);
	}
	counter++;
    	printf("Kerbal %ld: Lift off!\n", t_id);
	
    	/* END CRITICAL REGION */    
    	sem_post(&launchpad);       /* up semaphore */
	sleep(5);
	printf("Kerbal %ld: Landed!\n", t_id);
    
    	//thread will technically never exit once loop is included so exit will never be executed
	pthread_exit(NULL);
}
bool resources_available(){
	
	sem_getvalue(&e_parts, &engines);
	sem_getvalue(&f_parts, &fuelages);
	sem_getvalue(&t_parts, &tanks);

	if(engines && fuelages && tanks)
		return true;
	return false;

}

