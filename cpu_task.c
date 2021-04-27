#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cpu_task.h"
#include "utility.h"

int getCpuWithSRT(cpuTask *cpu[], int processors) {
  cpuTask *temp;
  int total_time = 0;
  int current_cpu = 0;
  for (int i = 0; i < processors; i++) {
    int total_temp_time = 0;
    temp = cpu[i]->next;
    while (temp != NULL) {
      total_temp_time += temp->remaining_time;
      temp = temp->next;
    }
    if (total_time == 0) {
      total_time = total_temp_time;
      current_cpu = i;
    } else if (total_time > total_temp_time) {
      total_time = total_temp_time;
      current_cpu = i;
    }
  }
  return current_cpu;
}

cpuTask *createCpuTask(int pid, int arrived_time, int run_time,
                       int remaining_time, int cpu_id, int sub_process) {
  cpuTask *new_task = (cpuTask *)malloc(sizeof(cpuTask));
  assert(new_task);
  new_task->pid = pid;
  new_task->cpu_id = cpu_id;
  new_task->sub_process = sub_process;
  new_task->running = false;
  new_task->arrived_time = arrived_time;
  new_task->remaining_time = remaining_time;
  new_task->run_time = run_time;
  new_task->next = NULL;
  return new_task;
}

void removeCpuTask(cpuTask *cpu, int pid) {
  cpuTask *temp;
  if (cpu != NULL) {
    while (cpu->next->pid != pid) {
      cpu = cpu->next;
    }
  }
  temp = cpu->next;
  cpu->next = temp->next;
  free(temp);
}

int checkCpuIdle(cpuTask *cpu[], int processors) {
  for (int i = 0; i < processors; i++) {
    cpuTask *temp = cpu[i];
    // Check whether the cpu is idle or not
    if (temp->next == NULL)
      return i;
  }
  // None of the cpu is idle
  return NO_IDLE_CPU;
}

cpuTask *insertNewTask(cpuTask *cpu_task, cpuTask *new_task) {
  cpuTask *temp_task = cpu_task;
  if (new_task->remaining_time < temp_task->remaining_time) {
    new_task->next = temp_task;
    return new_task;
  }
  while (temp_task->next &&
         (temp_task->next->remaining_time < new_task->remaining_time ||
          (temp_task->next->remaining_time == new_task->remaining_time &&
           temp_task->next->pid < new_task->pid))) {
    temp_task = temp_task->next;
  }
  new_task->next = temp_task->next;
  temp_task->next = new_task;
  return cpu_task;
}

bool checkSubTaskComplete(cpuTask *cpu[], int processors, int pid) {
  int num_sub = 0;
  for (int i = 0; i < processors; i++) {
    cpuTask *temp = cpu[i];
    while (temp->next != NULL) {
      if (temp->next->pid == pid)
        num_sub++;
      temp = temp->next;
    }
  }
  if (num_sub > 1)
    return false;
  return true;
}

int getNumCpuIsIdle(cpuTask *cpu[], int processors) {
  int num_cpu_idle = 0;
  for (int i = 0; i < processors; i++) {
    cpuTask *temp = cpu[i];
    // Check whether the cpu is idle or not
    if (temp->next == NULL)
      num_cpu_idle++;
  }
  // None of the cpu is idle
  return num_cpu_idle;
}