#include <assert.h>
#include <stdlib.h>

#include "process.h"
#include "utility.h"

process *createProcess(int arrived_time, int run_time, int pid,
                       char parallelisable) {
  process *new_process = (process *)malloc(sizeof(process));
  assert(new_process);
  new_process->pid = pid;
  new_process->cpu_id = NOT_ASSIGNED;
  new_process->arrived_time = arrived_time;
  new_process->run_time = run_time;
  new_process->remaining_time = run_time;
  new_process->parallelisable = parallelisable;
  new_process->next = NULL;
  return new_process;
}

void removeProcess(process *tasks, int pid) {
  process *temp;
  if (tasks != NULL) {
    while (tasks->next->pid != pid)
      tasks = tasks->next;
  }
  temp = tasks->next;
  tasks->next = temp->next;
  free(temp);
}

int checkNumOfProcessors(process *task, int processors) {
  int num_processors = 2;
  for (int i = 2; i <= processors; i++) {
    if (task->remaining_time / i >= 1)
      num_processors = i;
    else
      break;
  }
  return num_processors;
}