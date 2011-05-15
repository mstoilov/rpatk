#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "rmem.h"


#define NUM_THREADS 10
#define NUM_ITERATIONS 100000

typedef struct _RDummy {
	ruint32 i;
} RDummy;


void *test_busy_work(void *t) {
	ruint32 i;
	RDummy *pDummy;
	long tid = (long)t;

	printf("Thread %ld starting...\n", tid);
	for (i = 0; i < NUM_ITERATIONS; i++) {
		pDummy = (RDummy*)r_malloc(sizeof(*pDummy));
		r_free(pDummy);
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

	printf("Main: program completed. Exiting. Expected Result = %d, Allocated Memory = %d, Max Memory = %d\n",
			0, (int)r_debug_get_allocmem(), (int)r_debug_get_maxmem());
	if (r_debug_get_allocmem() == 0)
		printf("Test PASSED\n");
	else
		printf("Test FAILED\n");
	pthread_exit(NULL);
}

int maine(int argc, char *argv[]) {
	fprintf(stdout, "It works!\n");
	return 0;
}
