#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <sched.h>

#define PART 1

// The scheduler include SCHED_FIFO, SCHED_RR, and SCHED_DEADLINE
#define SCHEDULING SCHED_FIFO

/* Hardware dependency parameter */
#define CORE_NUM 4

/* Workload parameter */
#define MASK_SIZE 5
#define UTILIZATION_DIVIDER 10000

#define __NR_sched_setattr 314
#define __NR_sched_getattr 315


#endif

