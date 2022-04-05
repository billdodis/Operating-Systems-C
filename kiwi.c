#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb") //anousio kathws tha vgalume to db_open kai close kai tha ta valume sthn main

void *_write_test(void *arg)//(long int count, int r, int threadcount,DB* db) 		
											//changed the arguments from the above to void *arg so we can run this function with the pthread_create
											//Furthermore, we changed the name from _write_test to *_write_test , again so pthread_create can run it.
											// I take as an argument the database because I do the
{											// db = db_open(DATAS) in main function and I need the db in order to exetute the db_add and db_get functions.
	int i;	
	double cost;
	long long start,end;
	Variant sk, sv;
	DB* db; 							
	//we have to get the values from the given struct passed as argument here.
	//so we transfer the values with the following 5 lines.
	struct args *values = (struct args *) arg; 					// passing the struct to a new struct we declare for value-transfer purposes.
	long int count = values->count;
	int r = values->r;
	int threadcount = values->threadcount;
	db = values->database;
	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);
	
	
	//db = db_open(DATAS); 				//we open the database before the thread creation(at main funtion), because they work simultaneously and they might change data at the same time, forcing errors
							//or one thread might close the database while another works in it, so there will be errors

	long int newcount = count/threadcount;   	//we create the newcount because we want to share the whole count number to our threads .
							// We share our counts in our threads. So we make COUNT writes.
	
	start = get_ustime_sec();
	for (i = 0; i < newcount; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}
	//db_close(db); 

	end = get_ustime_sec();
	cost = end -start;
	pthread_mutex_lock(&write_cal); 		//locking write_cal because we want helpingvar safe.
	helpingvar++;
	if(helpingvar==1){
		random_write_newcount = newcount;	//passing values to these variables helping us keep some stat to print on the main function.
		random_write_cost = cost;
	}
	pthread_mutex_unlock(&write_cal); 		//unlocking write_cal.
	return 0;					// because of warning: control reaches end of non-void function [-Wreturn-type]
}

void *_read_test(void *arg)				//(long int count, int r, int threadcount, DB* db) 
							//changed the arguments from the above to void *arg so we can run this function with the pthread_create
							//Furthermore, we changed the name from _read_test to *_read_test , again so pthread_create can run it.
{
	int i;
	int ret;
	int found = 0;
	double cost;
	long long start,end;
	Variant sk;
	Variant sv;
	DB* db; 							
							//we have to get the values from the given struct passed as argument here.
							//so we transfer the values with the following 5 lines.
	struct args *values = (struct args *) arg; 	// passing the struct to a new struct we declare for value-transfer purposes.
	long int count = values->count;
	int r = values->r;
	int threadcount = values->threadcount;
	db = values->database;
	char key[KSIZE + 1];

	//db = db_open(DATAS);				//we open the database before the thread creation(at main funtion), because they work simultaneously and they might change data at the same time, forcing errors
							//or one thread might close the database while another works in it, so there will be errors

	long int newcount = count/threadcount;   	//we create the newcount because we want to share the whole count number to our threads .
							// We share our counts in our threads. So we make COUNT reads.
	start = get_ustime_sec();
	for (i = 0; i < newcount; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	//db_close(db);
	
	end = get_ustime_sec();
	cost = end - start;				//the cost(time) of a thread
	pthread_mutex_lock(&foundall_cal); 		//locking foundall because we want foundall safe. 
							//foundall is a variable which keeps track of all the found values that where read from the threads
	foundall = foundall + found;			//foundall sum.
	helpingvar1++;
	if(helpingvar1 == 1){				//helping vars for stat printing
		random_read_newcount = newcount;
		random_read_found = found;
		random_read_cost = cost;
	}
	pthread_mutex_unlock(&foundall_cal); 		// unlocking foundall.
	return 0;					// because of warning: control reaches end of non-void function [-Wreturn-type]
}