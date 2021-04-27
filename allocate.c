#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "allocate.h"
#include "utility.h"

int main(int argc, char *argv[]) {
  char opt;
  char filepath[100];
  int processors = 0;
  bool flag = false;
  while ((opt = getopt(argc, argv, "f:p:c")) != -1) {
    switch (opt) {
    case 'f':
      strcpy(filepath, optarg);
      break;
    case 'p':
      processors = atoi(optarg);
      break;
    case 'c':
      flag = true;
      break;
    default:
      printf("Option %c not defiend\n", opt);
      break;
    }
  }
  scheduler(processors, filepath, flag);
  return 0;
}

void scheduler(int processors, char filepath[], bool flag) {
  FILE *fp;
  if ((fp = fopen(filepath, "r")) == NULL) {
    perror("File does not exit");
    exit(-1);
  }

  int arrived_time = 0, run_time = 0, pid = 0;
  int num_task = 0;
  char parallelisable = ' ';

  process *processes_head = (process *)malloc(sizeof(process));
  assert(processes_head);
  process *processes_tail = processes_head;

  while (fscanf(fp, "%d %d %d %c", &arrived_time, &pid, &run_time,
                &parallelisable) != EOF) {
    processes_tail->next =
        createProcess(arrived_time, run_time, pid, parallelisable);
    processes_tail = processes_tail->next;
    num_task++;
  }

  scheduling(processes_head, num_task, processors, flag);
}

void scheduling(process *tasks, int num, int processors, bool flag) {
  cpuTask *cpu[processors];
  int time = 0;
  int num_tasks = num;
  int num_cpu_tasks = 0;
  float turnaround_time = 0, overhead_avg = 0, overhead_max = 0;

  for (int i = 0; i < processors; i++) {
    cpu[i] = createCpuTask(NOT_ASSIGNED, NOT_ASSIGNED, NOT_ASSIGNED,
                           NOT_ASSIGNED, NOT_ASSIGNED, NOT_ASSIGNED);
  }

  for (; num_cpu_tasks + num_tasks > 0; time++) {
    int num_new_tasks = 0;

    for (int i = 0; i < processors; i++) {
      cpuTask *temp_cpu = cpu[i]->next;
      if (temp_cpu == NULL)
        continue;
      else
        temp_cpu->remaining_time--;
      // Check how many cpu task is completed at this time.
      if (temp_cpu->remaining_time == 0 &&
          temp_cpu->sub_process == NOT_ASSIGNED)
        num_cpu_tasks--;
    }

    for (int i = 0; i < processors; i++) {
      cpuTask *temp_cpu = cpu[i]->next;
      if (temp_cpu == NULL)
        continue;
      if (temp_cpu->remaining_time == 0) {
        float temp_overhead = 0.0;
        if (temp_cpu->sub_process == NOT_ASSIGNED) {
          turnaround_time =
              turnaround_time + (1.0 * time - temp_cpu->arrived_time);
          overhead_avg =
              overhead_avg +
              round((1.0 * time - temp_cpu->arrived_time)) / temp_cpu->run_time;
          temp_overhead =
              round(1.0 * time - temp_cpu->arrived_time) / temp_cpu->run_time;
          printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", time, temp_cpu->pid,
                 num_cpu_tasks);
          removeCpuTask(cpu[i], temp_cpu->pid);
        } else {
          if (checkSubTaskComplete(cpu, processors, temp_cpu->pid)) {
            num_cpu_tasks--;
            turnaround_time =
                turnaround_time + (1.0 * time - temp_cpu->arrived_time);
            overhead_avg =
                overhead_avg + round((1.0 * time - temp_cpu->arrived_time)) /
                                   temp_cpu->run_time;
            temp_overhead =
                round(1.0 * time - temp_cpu->arrived_time) / temp_cpu->run_time;
            printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", time,
                   temp_cpu->pid, num_cpu_tasks);
          }
          removeCpuTask(cpu[i], temp_cpu->pid);
        }
        if (overhead_max == 0)
          overhead_max = temp_overhead;
        else if (temp_overhead > overhead_max)
          overhead_max = temp_overhead;
      }
    }
    if (!flag) {
      if ((num_new_tasks = seekNewTask(tasks, cpu, time, processors)) !=
          NOT_ASSIGNED) {
        // New tasks got assign into cpu
        num_cpu_tasks += num_new_tasks;
        // Remove from tasks list
        num_tasks -= num_new_tasks;
      }
    } else {
      if ((num_new_tasks = optimizeSeekNewTask(tasks, cpu, time, processors)) !=
          NOT_ASSIGNED) {
        // New tasks got assign into cpu
        num_cpu_tasks += num_new_tasks;
        // Remove from tasks list
        num_tasks -= num_new_tasks;
      }
    }

    for (int i = 0; i < processors; i++) {
      cpuTask *temp_cpu = cpu[i]->next;
      // This cpu is idle
      if (temp_cpu == NULL)
        continue;
      else if (!temp_cpu->running) {
        if (temp_cpu->sub_process == NOT_ASSIGNED)
          printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=%d\n", (int)time,
                 temp_cpu->pid, temp_cpu->remaining_time, temp_cpu->cpu_id);
        else
          printf("%d,RUNNING,pid=%d.%d,remaining_time=%d,cpu=%d\n", (int)time,
                 temp_cpu->pid, temp_cpu->sub_process, temp_cpu->remaining_time,
                 temp_cpu->cpu_id);
      }
      temp_cpu->running = true;
    }
  }
  time--;
  printf("Turnaround time %d\n", (int)ceil(turnaround_time / num));
  printf("Time overhead %g %g\n", roundFloat(overhead_max),
         roundFloat(overhead_avg / num));
  printf("Makespan %d\n", (int)time);
}

int seekNewTask(process *tasks, cpuTask *cpu[], int time, int processors) {
  int num_new_tasks = 0;
  process *first_arrived, *temp_task;
  while (tasks->next) {
    first_arrived = NULL;
    temp_task = tasks->next;
    while (temp_task != NULL) {
      if (temp_task->arrived_time <= time) {
        if (!first_arrived) {
          first_arrived = temp_task;
          continue;
        }
        if (temp_task->remaining_time < first_arrived->remaining_time) {
          first_arrived = temp_task;
        }
        if (temp_task->remaining_time == first_arrived->remaining_time &&
            temp_task->pid < first_arrived->pid) {
          first_arrived = temp_task;
        }
      }
      temp_task = temp_task->next;
    }

    if (first_arrived == NULL) {
      break;
    }

    int temp_cpu;
    if (first_arrived->parallelisable == 'p' && processors >= 2) {
      int num_processors = checkNumOfProcessors(first_arrived, processors);
      // create subprocess according to num_processors
      for (int i = 0; i < num_processors; i++) {
        if ((temp_cpu = checkCpuIdle(cpu, processors)) >= 0) {
          cpu[temp_cpu]->next = createCpuTask(
              first_arrived->pid, first_arrived->arrived_time,
              first_arrived->run_time,
              ceil((double)first_arrived->remaining_time / num_processors) + 1,
              temp_cpu, i);
        } else {
          temp_cpu = getCpuWithSRT(cpu, processors);
          cpuTask *temp_task = cpu[temp_cpu]->next;
          cpuTask *new_task = createCpuTask(
              first_arrived->pid, first_arrived->arrived_time,
              first_arrived->run_time,
              ceil((double)first_arrived->remaining_time / num_processors) + 1,
              temp_cpu, i);
          cpu[temp_cpu]->next = insertNewTask(temp_task, new_task);
          if (temp_task != cpu[temp_cpu]->next) {
            temp_task->running = false;
          }
        }
      }
    } else {
      if ((temp_cpu = checkCpuIdle(cpu, processors)) >= 0) {
        cpu[temp_cpu]->next = createCpuTask(
            first_arrived->pid, first_arrived->arrived_time,
            first_arrived->run_time, first_arrived->remaining_time, temp_cpu,
            NOT_ASSIGNED);
      } else {
        temp_cpu = getCpuWithSRT(cpu, processors);
        cpuTask *temp_task = cpu[temp_cpu]->next;
        cpuTask *new_task = createCpuTask(
            first_arrived->pid, first_arrived->arrived_time,
            first_arrived->run_time, first_arrived->remaining_time, temp_cpu,
            NOT_ASSIGNED);
        cpu[temp_cpu]->next = insertNewTask(temp_task, new_task);
        if (temp_task != cpu[temp_cpu]->next) {
          temp_task->running = false;
        }
      }
    }

    if (DEBUG)
      for (int i = 0; i < processors; i++) {
        printf("time(%d)=> Cpu: [%d]->", time, i);
        for (cpuTask *temp_task = cpu[i]->next; temp_task;
             temp_task = temp_task->next) {
          printf("pid: %d-> remaining_time: (%d)->", temp_task->pid,
                 temp_task->remaining_time);
        }
        printf("\n");
      }
    removeProcess(tasks, first_arrived->pid);
    num_new_tasks++;
  }
  // Got new tasks
  if (num_new_tasks != 0)
    return num_new_tasks;
  // No new task
  return NO_NEW_TASK;
}

int optimizeSeekNewTask(process *tasks, cpuTask *cpu[], int time, int processors) {
  int num_new_tasks = 0;
  process *first_arrived, *temp_task;
  // Process that arrvied in same time and has same run time.
  int num_tasks_SASR = 0;
  while (tasks->next) {
    first_arrived = NULL;
    temp_task = tasks->next;
    while (temp_task != NULL) {
      if (temp_task->arrived_time <= time) {
        if (!first_arrived) {
          first_arrived = temp_task;
          continue;
        }
        if (temp_task->remaining_time < first_arrived->remaining_time) {
          first_arrived = temp_task;
        }
        if (temp_task->remaining_time == first_arrived->remaining_time) {
          first_arrived = temp_task;
          num_tasks_SASR++;
        }
      }
      temp_task = temp_task->next;
    }

    if (first_arrived == NULL) {
      break;
    }

    int temp_cpu;
    if (first_arrived->parallelisable == 'p' && processors >= 2) {
      int num_processors = 0;
      if (num_tasks_SASR > 1 && processors >= 4) {
        if (processors % 2 == 0 && getNumCpuIsIdle(cpu, processors) >= 2) {
          num_processors = 2;
          num_tasks_SASR--;
        } else if (processors % 2 != 0 && getNumCpuIsIdle(cpu, processors) >= 3) {
          num_processors = 3;
          num_tasks_SASR--;
        } else {
          num_processors = processors;
          num_tasks_SASR--;
        }
      } else if (num_tasks_SASR > 1){
        num_processors = 2;
        num_tasks_SASR--;
      } else {
        num_processors = checkNumOfProcessors(first_arrived, processors);
      }
      
      // create subprocess according to num_processors
      for (int i = 0; i < num_processors; i++) {
        if ((temp_cpu = checkCpuIdle(cpu, processors)) >= 0) {
          cpu[temp_cpu]->next = createCpuTask(
              first_arrived->pid, first_arrived->arrived_time,
              first_arrived->run_time,
              ceil((double)first_arrived->remaining_time / num_processors) + 1,
              temp_cpu, i);
        } else {
          temp_cpu = getCpuWithSRT(cpu, processors);
          cpuTask *temp_task = cpu[temp_cpu]->next;
          cpuTask *new_task = createCpuTask(
              first_arrived->pid, first_arrived->arrived_time,
              first_arrived->run_time,
              ceil((double)first_arrived->remaining_time / num_processors) + 1,
              temp_cpu, i);
          cpu[temp_cpu]->next = insertNewTask(temp_task, new_task);
          if (temp_task != cpu[temp_cpu]->next) {
            temp_task->running = false;
          }
        }
      }
    } else {
      if ((temp_cpu = checkCpuIdle(cpu, processors)) >= 0) {
        cpu[temp_cpu]->next = createCpuTask(
            first_arrived->pid, first_arrived->arrived_time,
            first_arrived->run_time, first_arrived->remaining_time, temp_cpu,
            NOT_ASSIGNED);
      } else {
        temp_cpu = getCpuWithSRT(cpu, processors);
        cpuTask *temp_task = cpu[temp_cpu]->next;
        cpuTask *new_task = createCpuTask(
            first_arrived->pid, first_arrived->arrived_time,
            first_arrived->run_time, first_arrived->remaining_time, temp_cpu,
            NOT_ASSIGNED);
        cpu[temp_cpu]->next = insertNewTask(temp_task, new_task);
        if (temp_task != cpu[temp_cpu]->next) {
          temp_task->running = false;
        }
      }
    }

    if (DEBUG)
      for (int i = 0; i < processors; i++) {
        printf("time(%d)=> Cpu: [%d]->", time, i);
        for (cpuTask *temp_task = cpu[i]->next; temp_task;
             temp_task = temp_task->next) {
          printf("pid: %d-> remaining_time: (%d)->", temp_task->pid,
                 temp_task->remaining_time);
        }
        printf("\n");
      }
    removeProcess(tasks, first_arrived->pid);
    num_new_tasks++;
  }
  // Got new tasks
  if (num_new_tasks != 0)
    return num_new_tasks;
  // No new task
  return NO_NEW_TASK;
}