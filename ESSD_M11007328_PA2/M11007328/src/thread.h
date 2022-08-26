#ifndef _THREAD_H_
#define _THREAD_H_

#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/syscall.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "config.h"
#include "../libs/check.h"

/* Global Resource */
extern float sharedSum;
extern pthread_mutex_t* ioMutex;
extern pthread_barrier_t* barr;
extern pthread_mutex_t* criMutex;
extern sem_t* sem;
extern pthread_spinlock_t* lock;

class Thread
{
  public:

    /* Constructrue */
    void init (int, float**, float**, float**);

    /* Baseline */
    static void* convolution (void*);      // Perform convolution

    /* Part 1~3*/
    void enterCriticalSection ();
    void exitCriticalSection ();
    void synchronize ();

    /* Other */
    void printThreadInfo ();

    void setCore (int);                   // Set the core where thread to be executed
    void setUpCPUAffinityMask (int);      // Pined the thread to core 
    void setEndCalculatePoint (int);      // Convolution start point
    void setStartCalculatePoint (int);    // Convolution end point
    void setloc (int);    // setloc

  public:
    pthread_t _thread;

  public:
    int ID;

    int core = -1;      // The core want to assign to
    int cur_core;       // Run-time core executing this thread
    int PID;

    int startCalculatePoint;
    int endCalculatePoint;

    float **mask;
    float **matrix;
    float **multiResult;	 

    float loc = 0;

};

#endif
