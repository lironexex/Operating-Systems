#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <sys/sem.h>
#include <semaphore.h>
#include "Barrier.h"

Barrier global_barrier(10);
int sum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int ids[10] = {1,2,3,4,5,6,7,8,9,10};

void* thread_workload(void* threadID) {
	
	int* p_val = (int*)threadID;
	pthread_mutex_lock(&mutex);
	sum += *p_val;
	pthread_mutex_unlock(&mutex);
	global_barrier.wait();
	
	if (sum == 55) {
		printf ("sum == 55\n");
	}
	else {
		printf ("ERROR: sum == %d\n", sum);
	}
	return 0;
}

int main() {
	pthread_t t;
	int i;
	for (i = 0; i < 10; ++i) {
		pthread_create(&t, NULL, thread_workload, (void*)(ids+i));
		//printf ("Thread #%d was created.\n", i);
		
		//global_barrier.wait();
		//printf ("current number of waiting threads is: %d\n",global_barrier.waitingThreads());
	}

	printf ("line 36: sum == %d\n", sum);
	printf ("Test completed.\n");
	
	return 0;
}