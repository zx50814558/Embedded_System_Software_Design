#include "system.h"

System::System (char* input_file)
{
    loadInput(input_file);

    for (int i = 0; i < numThread; i++)
    {
#if (PART == 1)
        /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
        // For part1, we assign the matrix0 into all threads
        threadSet[i].init(multiResult[0], matrix[0], mask[0]);
        // Set up the caculation range of each thread matrix
        if (i == 0){
            threadSet[i].setStartCalculatePoint((threadSet[i].matrixSize() / 10) * 0);
            threadSet[i].setEndCalculatePoint((threadSet[i].matrixSize() / 10) * 1);
        }
        else if (i == 1){
            threadSet[i].setStartCalculatePoint((threadSet[i].matrixSize() / 10) * 1);
            threadSet[i].setEndCalculatePoint((threadSet[i].matrixSize() / 10) * 3);
        }
        else if (i == 2){
            threadSet[i].setStartCalculatePoint((threadSet[i].matrixSize() / 10) * 3);
            threadSet[i].setEndCalculatePoint((threadSet[i].matrixSize() / 10) * 6);
        }
        else{
            threadSet[i].setStartCalculatePoint((threadSet[i].matrixSize() / 10) * 6);
            threadSet[i].setEndCalculatePoint((threadSet[i].matrixSize() / 10) * 10);
        }
        
        /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#else
        threadSet[i].init(multiResult[i], matrix[i], mask[i]);
            
#endif

#if (PART == 3)
	    /*~~~~~~~~~~~~Your code(PART3)~~~~~~~~~~~*/
        // Set the scheduling policy for thread.
        for (int i = 0; i < numThread; i++)
            threadSet[i].setSchedulingPolicy(SCHEDULING);
            // std::cout << threadSet[i].schedulingPolicy() << std::endl;
	    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
#endif
    }

    /* CPU list Initialize */
    cpuSet = new CPU[CORE_NUM];
    for (int i = 0; i < CORE_NUM; i++)
        cpuSet[i].init (i, numThread);
    
    /* Set up checker for checking correctness */
    check = new Check;
    check->init(CORE_NUM, MASK_SIZE, numThread, multiResult);
    std::cout << "\n===========Generate Matrix Data===========" << std::endl;
    setStartTime();
    for (int i = 0; i < numThread; i++)
    {
        check->setThreadWithIndex (i, &threadSet[i]._thread);
        check->setMatrixSizeWithIndex( i, threadSet[i].matrixSize() );
        check->dataGenerator(i, matrix, mask);
        threadSet[i].setCheck( check );
    }
    setEndTime();
    std::cout << "Generate Date Spend time : " << _timeUse << std::endl;
    
#if (PART == 3)
    if (SCHEDULING == SCHED_FIFO) {
        check->setCheckState(PARTITION_FIFO);
    } else if (SCHEDULING == SCHED_RR) {
        check->setCheckState(PARTITION_RR);
    } else {
        std::cout << "!! Not supported scheduler !!" << std::endl;
        assert(false);
    }
#endif

}

System::~System()
{
    for (int thread_id = 0; thread_id < numThread; thread_id++)
    {
        for (int i = 0; i < MASK_SIZE; i++)
        {
            delete[] mask[thread_id][i];
        }
        delete[] mask[thread_id];
        for (int i = 0; i < threadSet[thread_id].matrixSize(); i++)
        {
            delete[] matrix[thread_id][i];
            delete[] multiResult[thread_id][i];
        }
        delete[] matrix[thread_id];
        delete[] multiResult[thread_id];
    }
    delete[] mask;
    delete[] matrix;
    delete[] multiResult;

    delete cpuSet;
}

void
System::creatAnswer ()
{
    check->creatAnswer(PART);
}

/**
 * Using the pthread function to perform PA exection.
 * 
 * pthread_create( pthread_t* thread, NULL, void* function (void*), void* args );
 * 
 * pthread_join( pthread_t* thread, NULL );
 * 
 */
void
System::globalMultiCoreConv ()
{
    std::cout << "\n===========Start Global Multi-Thread Convolution===========" << std::endl;
    check->setCheckState(GLOBAL);
    setStartTime();

    /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
    // Create thread and join
    for (int i = 0; i < numThread; i++){
        pthread_create (&threadSet[i]._thread, NULL, &threadSet[i].convolution, &threadSet[i]);
    }
    for (int j = 0; j < numThread; j++)
    	pthread_join (threadSet[j]._thread, NULL);
    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    setEndTime();
    std::cout << "Global Multi Thread Spend time : " << _timeUse << std::endl;
    cleanMultiResult();
}

void
System::partitionMultiCoreConv ()
{
#if (PART == 1)
    std::cout << "\n===========Start Partition Multi-Thread Convolution===========" << std::endl;
    check->setCheckState(PARTITION);
    for (int i = 0; i < numThread; i++)
        threadSet[i].setCore(i);
#endif
    setStartTime();

    /*~~~~~~~~~~~~Your code(PART1)~~~~~~~~~~~*/
    // Create thread and join
    for (int i = 0; i < numThread; i++){
    	pthread_create (&threadSet[i]._thread, NULL, &threadSet[i].convolution, &threadSet[i]);
    }
    for (int j = 0; j < numThread; j++)
    	pthread_join (threadSet[j]._thread, NULL);
    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
    
    setEndTime();
    std::cout << "Partition Multi Thread Spend time : " << _timeUse << std::endl;
    cleanMultiResult();
}

void
System::partitionFirstFit ()
{
    std::cout << "\n===========Partition First-Fit Multi Thread Matrix Multiplication===========" << std::endl;
#if (PART == 2)
    check->setCheckState(PARTITION_FF);
#endif
    threadSet -> setcurrent_PID(-1);
    threadSet -> restart_fn(0);
    threadSet -> totoalThread_fn(numThread);
    /*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
    // Implement parititon first-fit and print result.
    for (int i = 0; i < CORE_NUM; i++) 
        cpuSet[i].emptyCPU();
    int usedList[numThread] = {0};
    for (int i = 0; i < CORE_NUM; i++){
        for (int j = 0; j < numThread; j++){
            if (usedList[j] == 1)
                continue;
            else if ((cpuSet[i].utilization() + threadSet[j].utilization()) <= 1){
                cpuSet[i].push_thread(threadSet[j].ID(), threadSet[j].utilization());
                threadSet[j].setCore(i);
                usedList[j] = 1;
            }
        }
    }
    for (int i = 0; i < numThread; i++){
        if (usedList[i] == 0)
            std::cout << "Thread-" << threadSet[i].ID() << " not schedulable." << std::endl;
       }
    for (int i = 0; i < CORE_NUM; i++){
        std::cout << "CPU" << i << " : ";
        cpuSet[i].printCPUInformation();
    }
    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    partitionMultiCoreConv();
    cleanMultiResult();
}
    
void
System::partitionBestFit ()
{
    std::cout << "\n===========Partition Best-Fit Multi Thread Matrix Multiplication===========" << std::endl;
#if (PART == 2)
    check->setCheckState(PARTITION_BF);
#endif
    threadSet -> setcurrent_PID(-1);
    threadSet -> restart_fn(0);
    /*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
    // Implement partition best-fit and print result.
    float utilList[CORE_NUM] = {0};
    int usedList[numThread] = {0};
    int maxCpu = 0;
    float maxValeue = 0;
    for (int i = 0; i < CORE_NUM; i++) 
        cpuSet[i].emptyCPU();
    for (int i = 0; i < numThread; i++){
        for (int j = 0; j < CORE_NUM; j++){
            utilList[j] = cpuSet[j].utilization() + threadSet[i].utilization();
        }
        // std::cout << "utilList = [" << cpuSet[0].utilization() << ", " << cpuSet[1].utilization() << ", " << cpuSet[2].utilization() << ", " << cpuSet[3].utilization() << "]" << std::endl;

        for (int j = 0; j < CORE_NUM; j++){
            if (utilList[j] <= 1){
                if (utilList[j] > maxValeue){
                    maxValeue = utilList[j];
                    maxCpu = j;
                }
            }
        }
        if ((cpuSet[maxCpu].utilization() + threadSet[i].utilization()) <= 1){
            cpuSet[maxCpu].push_thread(threadSet[i].ID(), threadSet[i].utilization());
            threadSet[i].setCore(maxCpu);
            usedList[i] = 1;
        }
        maxValeue = 0;
    }
    for (int i = 0; i < numThread; i++){
        if (usedList[i] == 0)
            std::cout << "Thread-" << threadSet[i].ID() << " not schedulable." << std::endl;
       }
    for (int i = 0; i < CORE_NUM; i++){
        std::cout << "CPU" << i << " : ";
        cpuSet[i].printCPUInformation();
    }
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    partitionMultiCoreConv();
    cleanMultiResult();
}

void
System::partitionWorstFit ()
{
    std::cout << "\n===========Partition Worst-Fit Multi Thread Matrix Multiplication===========" << std::endl;
#if (PART == 2)
    check->setCheckState(PARTITION_WF);
#endif
    threadSet -> setcurrent_PID(-1);
    threadSet -> restart_fn(0);
    /*~~~~~~~~~~~~Your code(PART2)~~~~~~~~~~~*/
    // Implement partition worst-fit and print result.
    float utilList[CORE_NUM] = {0};
    int usedList[numThread] = {0};
    int maxCpu = 0;
    float maxValeue = 1;
    for (int i = 0; i < CORE_NUM; i++) 
        cpuSet[i].emptyCPU();
    for (int i = 0; i < numThread; i++){
        for (int j = 0; j < CORE_NUM; j++){
            utilList[j] = cpuSet[j].utilization() + threadSet[i].utilization();
        }
        // std::cout << "utilList = [" << cpuSet[0].utilization() << ", " << cpuSet[1].utilization() << ", " << cpuSet[2].utilization() << ", " << cpuSet[3].utilization() << "]" << std::endl;

        for (int j = 0; j < CORE_NUM; j++){
            if (utilList[j] <= 1){
                if (utilList[j] < maxValeue){
                    maxValeue = utilList[j];
                    maxCpu = j;
                }
            }
        }
        if ((cpuSet[maxCpu].utilization() + threadSet[i].utilization()) <= 1){
            cpuSet[maxCpu].push_thread(threadSet[i].ID(), threadSet[i].utilization());
            threadSet[i].setCore(maxCpu);
            usedList[i] = 1;
        }
        maxValeue = 1;
    }
    for (int i = 0; i < numThread; i++){
        if (usedList[i] == 0)
            std::cout << "Thread-" << threadSet[i].ID() << " not schedulable." << std::endl;
       }
    for (int i = 0; i < CORE_NUM; i++){
        std::cout << "CPU" << i << " : ";
        cpuSet[i].printCPUInformation();
    }
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
    
    partitionMultiCoreConv();
    cleanMultiResult();
}

void
System::cleanMultiResult ()
{
    for (int thread_id = 0; thread_id < numThread; thread_id++)
    {
        int matrix_size = threadSet[thread_id].matrixSize();
        for (int i = 0; i < matrix_size; i++)
            memset(multiResult[thread_id][i], 0, sizeof(float)*matrix_size);
    }  
}

void 
System::loadInput (char* input_file)
{
    std::ifstream infile(input_file);
    std::string line;

    int read_matrix_size = 0;
    float total_matrix_size = 0;

    if(infile.is_open()) {
        getline(infile, line);
        numThread = atoi(line.c_str());
    
            std::cout << "Input File Name : " << input_file << std::endl;
            std::cout << "numThread : " << numThread << std::endl;

        threadSet = new Thread[numThread];
            multiResult = new float**[numThread];
            matrix = new float**[numThread];
            mask = new float**[numThread];
        
        for (int i = 0; i < numThread; i++)
        {
            getline(infile, line);
            read_matrix_size = atoi(line.c_str());
                std::cout << i << ".Matrix size : " << read_matrix_size << std::endl;
            total_matrix_size += read_matrix_size;
            threadSet[i].setThreadID(i);
            threadSet[i].setMatrixSize(read_matrix_size);
                setUpMatrix(i);

        }
        std::cout << "Workload Utilization : " << total_matrix_size / UTILIZATION_DIVIDER << std::endl;
        
        infile.close();
    } else {
        std::cout << "!! Input file not found !!" << std::endl;
        assert(false);
    }
}

void
System::setEndTime ()
{
	gettimeofday(&end, NULL);
	_timeUse = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
}

void
System::setStartTime ()
{
	gettimeofday(&start, NULL);
}

void
System::setUpMatrix (int thread_id)
{
    int matrix_size = threadSet[thread_id].matrixSize();

    multiResult[thread_id] = new float*[matrix_size];
    for (int i = 0; i < matrix_size; i++)
        multiResult[thread_id][i] = new float[matrix_size];

    matrix[thread_id] = new float*[matrix_size];
    for (int i = 0; i < matrix_size; i++)
        matrix[thread_id][i] = new float[matrix_size];

    if((MASK_SIZE % 2) != 0)
    {
        mask[thread_id] = new float*[MASK_SIZE];
        for (int i = 0; i < MASK_SIZE; i++)
            mask[thread_id][i] = new float[MASK_SIZE];
    } else {
        std::cout << "!! Mask size not odd number !!" << std::endl;
        assert(false);
    }
    
}


