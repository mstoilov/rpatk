#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "rspinlock.h"

#define NUM_THREADS 10
#define NUM_ITERATIONS 100000

static rspinlock g_lock;
static ruint32 g_result;


void *test_busy_work(void *t) {
	ruint32 r;
	ruint32 i;
	long tid = (long)t;

	printf("Thread %ld starting...\n", tid);
	for (i = 0; i < NUM_ITERATIONS; i++) {
		r_spinlock_lock(&g_lock);
		r = g_result;
		r = r + 1;
		g_result = r;
		r_spinlock_unlock(&g_lock);
	}
	printf("Thread %ld done. \n", tid);
	pthread_exit((void*) t);
}


int main(int argc, char *argv[]) {
	pthread_t thread[NUM_THREADS];
	pthread_attr_t attr;
	int rc;
	long t;
	void *status;

	/* Initialize the global varibales */
	r_spinlock_init(&g_lock);
	g_result = 0;

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (t = 0; t < NUM_THREADS; t++) {
		printf("Main: creating thread %ld\n", t);
		rc = pthread_create(&thread[t], &attr, test_busy_work, (void *) t);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);
	for (t = 0; t < NUM_THREADS; t++) {
		rc = pthread_join(thread[t], &status);
		if (rc) {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		printf("Main: completed join with thread %ld having a status of %ld\n",
				t, (long) status);
	}

	printf("Main: program completed. Exiting. Expected Result = %d, Actual Result = %d\n",
			NUM_ITERATIONS * NUM_THREADS, g_result);
	if (g_result == NUM_ITERATIONS * NUM_THREADS)
		printf("Test PASSED\n");
	else
		printf("Test FAILED\n");
	pthread_exit(NULL);
}

int maine(int argc, char *argv[]) {
	fprintf(stdout, "It works!\n");
	return 0;
}
