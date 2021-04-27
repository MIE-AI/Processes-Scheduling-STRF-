#ifndef ALLOCATE
#define ALLOCATE 1

#include "cpu_task.h"
#include "process.h"

// Scheduler that handle input file and add process into linklist
void scheduler(int num_processors, char filepath[], bool flag);
// Processes allocate Simulator
void scheduling(process *tasks, int time, int processors, bool flag);
// Assign appropriate task according to shortest-time-remaining algorithm within
// task list to cpu
int seekNewTask(process *tasks, cpuTask *cpu[], int time, int processors);
// Optimize from seekNewTask function to achieve the improvement of Makespan
int optimizeSeekNewTask(process *tasks, cpuTask *cpu[], int time, int processors);

#endif