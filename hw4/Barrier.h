#ifndef BARRIER_H_
#define BARRIER_H_

#include <pthread.h>
#include <sys/sem.h>
#include <semaphore.h>

class Barrier {

	pthread_mutex_t mutex;
	sem_t semaphore;
	int barrier_size;
	int current_num_of_threads;

public:
    Barrier(unsigned int num_of_threads);
    void wait();
	unsigned int waitingThreads();
    ~Barrier();

};

#endif // BARRIER_H_

