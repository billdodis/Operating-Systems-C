#include "bench.h"
#include <string.h>
#include "../engine/db.h" //include for DB* db
#include <pthread.h> //include for threads
#define DATAS ("testdb")

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

void print_stats(int found, long long time, long int wrthreads, long int rdthreads, long int wrcount, long int rdcount){ //print_stats(foundall , timespend, writethreads , readthreads, writecount , readcount);
	printf(LINE);
	printf("All threads are finished after: %.6f seconds\n", (double)time);
	printf(LINE);
	printf("|Writes (done:%ld): %.6f sec/op; %.1f writes/sec(estimated);cost:%.3f(sec);\n"
		,wrcount, (double)((double)time / wrcount)
		,(double)(wrcount / (double)time)
		,(double)time);
	printf(LINE);
	printf("|Reads (done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
		rdcount, found,
		(double)((double)time / rdcount),
		(double)(rdcount / (double)time),
		(double)time);
	printf(LINE);
}

int main(int argc,char** argv)
{
	pthread_mutex_init(&foundall_cal, NULL);
	pthread_mutex_init(&write_cal, NULL);
	int i;
	long int count;
	long long start,end,timespend;
	long int threadcount;
	int rate ;                		// taking the rate from the terminal on an integer   
	double newrate;                		// make the integer rate (e.g 0,5 for 50)
	long int writecount;			// multiplying our count with the ratio
	long int readcount;		
	long int writethreads;  		// number of threads for writing
	long int readthreads; 			// number of threads for reading
	random_write_newcount = 0;     		//creating some supporting variables for keeping stats of a random write thread and a random read thread
	random_write_cost = 0;			//	 					-//-
	random_read_newcount = 0;		//	 					-//-
	random_read_found = 0;			//	 					-//-
	random_read_cost = 0;
	helpingvar = 0;
	helpingvar1 = 0;
	foundall = 0;     			// for read threads (STATISTICS)
	DB* db;           			//auto pou ekana entos tis kiwi.c sta read kai write to kanw edw, gia auto kai orizw ena database

	srand(time(NULL)); // argc was lower than 3 before our change
	if (argc < 4 && ((strcmp(argv[1], "write") == 0) || (strcmp(argv[1], "read") == 0))) { 			// argc lower than 4 because we want the count of threads
		fprintf(stderr,"Usage: db-bench <write | read> <count> <count of threads> \n");
		exit(1);
	}
	if (argc < 5 && (strcmp(argv[1], "readwrite") == 0)) { 							// argc lower than 5 because we want the count of threads and the rate as you see below in the fprintf
		fprintf(stderr,"Usage: db-bench <readwrite> <count> <count of threads> <rate of write [e.g 50 -for 50 percentage]> \n");
		exit(1);
	}
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;
		struct args a;					//declaring the struct we have on the header file here. 
		count = atoi(argv[2]);
		threadcount = atoi(argv[3]);         		// take the number of threads
		pthread_t id[threadcount];           		// create the id for every thread
		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		db = db_open(DATAS);          			// anoigw thn database prin ksekinhsw ta write
		a.count = count;
		a.r = r;
		a.threadcount = threadcount;
		a.database = db;				//passing values to our struct which are the arguments of our threads.
		start = get_ustime_sec();
		for(i = 0; i<threadcount; i++){
			pthread_create(&id[i], NULL, _write_test, (void *) &a);  
		}
		for(i = 0; i<threadcount; i++){
			pthread_join(id[i], NULL);
		}
		end = get_ustime_sec();
		timespend = end - start;        		//time spent for writing.
		printf("Writing Time:%.6f\n", (double)timespend);
		
		//_write_test(count, r, db);    		//add 3rd argument the db so i can manage it in write_test 
		db_close(db); 					
	} else if (strcmp(argv[1], "read") == 0) {
		int r = 0;
		struct args a;					//declaring the struct we have on the header file here. 
		count = atoi(argv[2]);
		threadcount = atoi(argv[3]);         		// take the number of threads
		pthread_t id[threadcount];           		// create the id for every thread
		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		db = db_open(DATAS);
		a.count = count;
		a.r = r;
		a.threadcount = threadcount;
		a.database = db;				//passing values to our struct which are the arguments of our threads.
		start = get_ustime_sec();
		for(i = 0; i<threadcount; i++){
			pthread_create(&id[i], NULL, _read_test, (void *) &a); 
		}
		for(i = 0; i<threadcount; i++){
			pthread_join(id[i], NULL);
		}
		end = get_ustime_sec();
		timespend = end - start;        		//time spent for reading.
		printf("Reading Time:%.6f\n", (double)timespend);
		printf("reads found is: %ld\n", foundall);
		
		//_read_test(count, r, db);
		db_close(db);
	} else if (strcmp(argv[1], "readwrite") == 0) {
		int r = 0;
		struct args a;					//declaring the struct we have on the header file here. 
		struct args b;					//declaring the struct we have on the header file here. 
		count = atoi(argv[2]);
		threadcount = atoi(argv[3]);         		// take the number of threads and make it integer
		rate = atoi(argv[4]);                		// taking the rate from the terminal on an integer   
		newrate = ((double)rate/(double)100);           // make the integer rate (e.g 0,5 for 50)
		writecount = count*newrate;			// multiplying our count with the ratio
		readcount = count - writecount;		
		writethreads = threadcount * newrate;  		// number of threads for writing
		readthreads = threadcount - writethreads; 	// number of threads for reading
		pthread_t id[threadcount];           		// create the id for every thread
		_print_header(count);
		_print_environment();
		if (argc == 6) 
			r = 1;
		db = db_open(DATAS);          			// open the database before the writing starts
		a.count = writecount;				// passing values to the structs for both write and read test arguments needed for the threads
		a.r = r;
		a.threadcount = writethreads;
		a.database = db;
		b.count = readcount;
		b.r = r;
		b.threadcount = readthreads;
		b.database = db;
		start = get_ustime_sec();
		for(i = 0; i<writethreads; i++){
			pthread_create(&id[i], NULL, _write_test, (void *) &a);   
		}
		for(i = writethreads; i<threadcount; i++){
			pthread_create(&id[i], NULL, _read_test, (void *) &b);   
		}
		for(i = 0; i<writethreads; i++){
			pthread_join(id[i], NULL);
		}
		for(i = writethreads; i<threadcount; i++){
			pthread_join(id[i], NULL);
		}
		end = get_ustime_sec();
		timespend = end - start;        								//time spent for readwriting.
		//printf("reads found is: %ld\n", foundall);
		print_stats(foundall , timespend, writethreads , readthreads, writecount , readcount);		//for printint some stats
		printf("Printing a Random-Write-Thread's stats:\n");
		//printf(LINE);
		printf("|Random-Write thread(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); thread cost:%.3f(sec);\n"
			,random_write_newcount, (double)(random_write_cost / random_write_newcount)
			,(double)(random_write_newcount / random_write_cost)
			,random_write_cost);
		printf(LINE);
		printf("Printing a Random-Read-Thread's stats:\n");
		//printf(LINE);
		printf("|Random-Read thread(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); thread cost:%.3f(sec)\n",
			random_read_newcount, random_read_found,
			(double)(random_read_cost / random_read_newcount),
			(double)(random_read_newcount / random_read_cost),
			random_read_cost);
		printf(LINE);
		db_close(db);						
	} else {
		fprintf(stderr,"Usage: db-bench <write | read> <count> <random>\n or Usage: db-bench <readwrite> <count> <count of threads> <rate of write [e.g 50 -for 50 percentage]> \n");
		exit(1);
	}

	return 1;
}