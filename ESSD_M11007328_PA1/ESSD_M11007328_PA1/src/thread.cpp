#include "thread.h"

Thread::~Thread()
{
  for (int i = 0; i < MASK_SIZE; i++)
    delete[] mask[i];
  
  for (int i = 0; i < _matrixSize; i++)
  {
    delete[] matrix[i];
    delete[] multiResult[i];	
  }
  delete[] mask;
  delete[] matrix;
  delete[] multiResult;

  delete check;
}

void
Thread::init (float **multi_result, float **input_matrix, float **conv_mask)
{
  _utilization = float( _matrixSize / float(UTILIZATION_DIVIDER) );

    startCalculatePoint = 0;
    endCalculatePoint = _matrixSize;

    multiResult = multi_result;
    matrix = input_matrix;
    mask = conv_mask;
}

void*
Thread::convolution(void* args)
{
  Thread *obj = (Thread*) args;

#if (PART == 3)
    obj->setUpScheduler();
#endif
  /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
  // Set up the affinity mask
  obj->setUpCPUAffinityMask(obj->core);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

  /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
  // Edit the function into partial multiplication.
  // Hint : Thread::startCalculatePoint & Thread::endCalculatePoint
  int curPID = syscall (SYS_gettid);
  obj->cur_core = sched_getcpu();
  obj->Get_PID(curPID);
  pthread_mutex_lock( &count_Mutex );
    obj->printThreadInfo();
  pthread_mutex_unlock( &count_Mutex );
  int shift = (MASK_SIZE-1)/2;
  // std::cout << "startCalculatePoint " << obj->startCalculatePoint << " to " << obj->endCalculatePoint << std::endl;
  for (int i = obj->startCalculatePoint; i < obj->endCalculatePoint; i++)
  {
    for (int j = 0; j < obj->_matrixSize; j++)
    {
      for (int k = -shift; k <= shift; k++)
      {
        for (int l = -shift; l <= shift; l++)
        {
          if( i + k < 0 ||  i + k >= obj->_matrixSize || j + l < 0 ||  j + l >= obj->_matrixSize)
            continue;
          obj->multiResult[i][j] += obj->matrix[i+k][j+l] * obj->mask[k+shift][l+shift];
        }
      }
      /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
      // Observe the thread migration
      int newCpu = sched_getcpu();
      if (obj->cur_core != newCpu){
        std::cout << "The thread " << obj->_ID;
        std::cout << " PID " << obj->PID;
        std::cout << " is moved from CPU " << obj->cur_core;
        std::cout << " to " << newCpu << std::endl;
        obj->cur_core = newCpu;
      }
      /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
    }
#if (PART == 3)
    /*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
    /* Obaserve the execute thread on core-0 */
    // std::cout << "totoalThread: " << totoalThread << std::endl;
    pthread_mutex_lock( &count_Mutex );
    if (total != totoalThread-1){
      total++;
      pthread_cond_wait(&count_Total, &count_Mutex);
    }
    else{
      
      pthread_cond_broadcast(&count_Total);
    }
    pthread_mutex_unlock( &count_Mutex );

    if (obj->cur_core == 0 && current_PID == -1){
      current_PID = obj->PID;
      std::cout << "Core" << obj->cur_core << " start PID-" << current_PID << std::endl;
    }
    else if (obj->cur_core == 0 && current_PID != -1 && current_PID != obj->PID){
      std::cout << "Core" << obj->cur_core << " context switch from PID-" << current_PID << " to PID-" << obj->PID << std::endl;
      current_PID = obj->PID;
    }
    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#endif
  }
  /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

  pthread_mutex_lock( &count_Mutex );
    obj->check->checkCorrectness();
	pthread_mutex_unlock( &count_Mutex );

  return 0;
}

void
Thread::setUpCPUAffinityMask (int core_num)
{
 	/*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
  // Pined the thread to core.
  cpu_set_t set;
  CPU_ZERO (&set);
  CPU_SET (core_num, &set);
  sched_setaffinity (0, sizeof(set), &set);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
}

void
Thread::setUpScheduler()
{
	/*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
  // Set up the scheduler for current thread
  struct sched_param param;
  param.sched_priority = sched_get_priority_max(schedulingPolicy());
  sched_setscheduler(0, schedulingPolicy(), &param);
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
}

void
Thread::printThreadInfo()
{
    std::cout << "Thread ID : " << _ID ;
    std::cout << "\tPID : " << PID;
    std::cout << "\tCore : " << cur_core;
#if (PART != 1)
    std::cout << "\tUtilization : " << _utilization;
    std::cout << "\tMatrixSize : " << _matrixSize;	
#endif
    std::cout << std::endl;
}

void
Thread::setCheck (Check* _check)
{
  check = _check;
}

void
Thread::setCore (int _core)
{
  core = _core;
}

void 
Thread::setMatrixSize (int matrixSize)
{
  _matrixSize = matrixSize;
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
Thread::setSchedulingPolicy (int schedulingPolicy)
{
  _schedulingPolicy = schedulingPolicy;
}

void
Thread::setThreadID (int id)
{
  _ID = id;
}

//////////////////////////////////////
void
Thread::Get_PID (int pid)
{
  PID = pid;
}

void 
Thread::setcurrent_PID (int tmp){
  current_PID = tmp;
}

void
Thread::totoalThread_fn (int tmp2){
  totoalThread = tmp2;
}

void
Thread::restart_fn (int tmp3){
  total = tmp3;
}

