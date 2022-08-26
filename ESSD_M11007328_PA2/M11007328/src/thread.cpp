#include "thread.h"

void
Thread::init (int _threadID, float **conv_mask, float **input_matrix, float **multi_result)
{
    ID          = _threadID;
    mask        = conv_mask;
    matrix      = input_matrix;
    multiResult = multi_result;

    startCalculatePoint = 0;
    endCalculatePoint   = MATRIX_SIZE;
}

void*
Thread::convolution (void* args)
{
  Thread *obj = (Thread*) args;

  obj->setUpCPUAffinityMask (obj->core);
  obj->printThreadInfo ();
  
  /*~~~~~~~~~~~~Your code(Part1~3)~~~~~~~~~~~*/
  // Consider the functionality and the fol- //
  // -lowing for-loop description, complete  //
  // the multi-level convolution code by add //
  // up the resource protector and synchron- //
  // -izer.                                  //
  // ...
  // ...
  int shift = (MASK_SIZE-1)/2;
  for (int round = 0; round < CONVOLUTION_TIMES; round++) {
    obj->synchronize ();
    //sem_wait ( sem );   //art2若需要比對 Reentrant 與 non-Reentrant 才使用
    for (int i = obj->startCalculatePoint; i < obj->endCalculatePoint; i++) {
      for (int j = 0; j < MATRIX_SIZE; j++) {

#if (PART != 2)
        obj->enterCriticalSection ();
        sharedSum = 0;
        for (int k = -shift; k <= shift; k++) {
          for (int l = -shift; l <= shift; l++) {
            if ( i + k < 0 ||  i + k >= MATRIX_SIZE || j + l < 0 ||  j + l >= MATRIX_SIZE)
              continue;
            sharedSum += obj->matrix [i + k][j + l] * obj->mask [k + shift][l + shift];
          } // for (int l...
        } // for (int k...
        obj->multiResult [i][j] = sharedSum;
        obj->exitCriticalSection ();
#else
        /*~~~~~~~~~~~~Your code(Part2)~~~~~~~~~~~*/
        // Turn this non-reentrant function into //
        // reentrant function which means no re- //
        // sources contention issue happen.      //
        obj->loc = 0;
        for (int k = -shift; k <= shift; k++) {
          for (int l = -shift; l <= shift; l++) {
            if ( i + k < 0 ||  i + k >= MATRIX_SIZE || j + l < 0 ||  j + l >= MATRIX_SIZE)
              continue;
            obj->loc += obj->matrix [i + k][j + l] * obj->mask [k + shift][l + shift];
          } // for (int l...
        } // for (int k...
        obj->multiResult [i][j] = obj->loc;
        /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

#endif
      } // for (int j...
    } // for (int i...
#if (CONVOLUTION_TIMES > 1)  
    /*~~~~~~~~~~~~~~~~~HINT~~~~~~~~~~~~~~~~*/
    // We use the previous convolution re- //
    // -sult to be the next round's input. //
    // So here we copy the result to input //
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    //sem_post ( sem );  // Part2若需要比對 Reentrant 與 non-Reentrant 才使用
    obj->synchronize ();
    for (int i = obj->startCalculatePoint; i < obj->endCalculatePoint; i++)
        memcpy (obj->matrix [i], obj->multiResult [i], MATRIX_SIZE * sizeof (float));
  } // for (int round...
#endif  
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
  return 0;
}

void 
Thread::enterCriticalSection ()
{
#if _ProtectType == MUTEX
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
  // Implement your mutex ()
  pthread_mutex_lock ( criMutex );

	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#else
  /*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
  // Implement your spinlock
  pthread_spin_lock ( lock );

	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#endif
}

void
Thread::exitCriticalSection ()
{
#if _ProtectType == MUTEX
	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
  // Implement your mutex ()
  pthread_mutex_unlock ( criMutex );

	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#else
  /*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
  // Implement your spinlock
  pthread_spin_unlock ( lock );

	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#endif
}

void
Thread::synchronize ()
{
#if _SynType == BARRIER
  /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
  // Implement your barrier
  pthread_barrier_wait ( barr );
  
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#else
  pthread_mutex_lock (ioMutex);
    std::cout << "Synchronize method not supported." << std::endl;
	pthread_mutex_unlock (ioMutex);
#endif
}

void
Thread::printThreadInfo ()
{
  cur_core = sched_getcpu ();
  PID      = syscall (SYS_gettid);

  pthread_mutex_lock ( ioMutex );

    std::cout << "Thread ID : " << ID ;
    std::cout << "\tPID : " << PID;
    std::cout << "\tCore : " << cur_core;
    std::cout << std::endl;

  pthread_mutex_unlock ( ioMutex );
}

void
Thread::setCore (int _core)
{
  core = _core;
}

void
Thread::setUpCPUAffinityMask (int core_num)
{
	if (core_num == -1) {
        return;
    } else {
		cpu_set_t set;
		CPU_ZERO (&set);
		CPU_SET (core_num, &set);
		sched_setaffinity (0, sizeof(set), &set);
		cur_core = -1;
	}
}


void 
Thread::setEndCalculatePoint (int endPoint)
{
  endCalculatePoint = endPoint;
}

void 
Thread::setStartCalculatePoint (int startPoint)
{
  startCalculatePoint = startPoint;
}

void 
Thread::setloc (int locc)
{
  loc = locc;
}