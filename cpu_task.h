

// Linklist data structure to store CPU task information
typedef struct CPUTASK {
  int pid;
  int cpu_id;
  int sub_process;
  int arrived_time;
  bool running;
  int run_time;
  int remaining_time;
  struct CPUTASK *next;
} cpuTask;

// Create new cpu task
cpuTask *createCpuTask(int pid, int arrived_time, int run_time,
                       int remaining_time, int cpu_id, int sub_process);
// Check whether or not is there any cpu idle in processors
int checkCpuIdle(cpuTask *cpu[], int processors);
// Get cpu with shortest remaining time in processors
int getCpuWithSRT(cpuTask *cpu[], int processors);
// Remove task from given cpu
void removeCpuTask(cpuTask *cpu, int pid);
// Insert a new task in given cpu
cpuTask *insertNewTask(cpuTask *temp_task, cpuTask *new_task);
// Check whether or not the parallelisable task is completed.
bool checkSubTaskComplete(cpuTask *cpu[], int processors, int pid);
// Get number of cpu which is idle
int getNumCpuIsIdle(cpuTask *cpu[], int processors);