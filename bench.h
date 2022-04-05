#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../engine/db.h" //include for DB* db

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

struct args {  							//struct for passing argument on write test and read test via the thread
	long int count;
	int r;
	int threadcount;
    DB* database;
};

long int foundall;
long int random_write_newcount;     	//creating some supporting variables for printing stats of a random write thread and a random read thread
double random_write_cost;				//	 					-//-
long int random_read_newcount;			//	 					-//-
int random_read_found;					//	 					-//-
double random_read_cost;				//	 					-//-
int helpingvar;
int helpingvar1;
pthread_mutex_t foundall_cal;       	//using this to count all the found values from all the reads
pthread_mutex_t write_cal;				//using this to get some random stat for printing.
long long get_ustime_sec(void);
void _random_key(char *key,int length);
void *_write_test(void *arg);
void *_read_test(void *arg);
