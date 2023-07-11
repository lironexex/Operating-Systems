#include <pthread.h>
#include <sys/sem.h>
#include "Barrier.h"
#include <stdio.h>


// function (1/4)
Barrier::Barrier(unsigned int num_of_threads) {
	int result1, result2;
	result1 = pthread_mutex_init(&this->mutex, NULL);
	result2 = sem_init(&this->semaphore, 0, 0);
	this->current_num_of_threads = 0;
	this->barrier_size = num_of_threads;

	if(result1 < 0)
		fprintf(stderr, "Barrier error: mutex initialization failed \n");
	if(result2 < 0)
		fprintf(stderr, "Barrier error: semaphore initialition failed \n");
}

// function (2/4)
void Barrier::wait() {
	pthread_mutex_lock(&this->mutex);
	this->current_num_of_threads++;

	if (this->current_num_of_threads == this->barrier_size) {
		sem_post(&this->semaphore);
	}
	pthread_mutex_unlock(&this->mutex);

	sem_wait(&this->semaphore);
	sem_post(&this->semaphore);
}
	
// function (3/4)
unsigned int Barrier::waitingThreads() {
	pthread_mutex_lock(&this->mutex);
	unsigned int return_val = this->current_num_of_threads;
	pthread_mutex_unlock(&this->mutex);
	return return_val;
}

// function (4/4)
Barrier::~Barrier() { 
	pthread_mutex_destroy(&this->mutex);
	sem_destroy(&this->semaphore);
}
	