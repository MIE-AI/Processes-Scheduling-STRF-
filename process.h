

// Linklist data structure to store process information
typedef struct PROCESSINFO {
  int pid;
  int cpu_id;
  int arrived_time;
  int run_time;
  int remaining_time;
  char parallelisable;
  struct PROCESSINFO *next;
} process;

// Create new process by giving information
process *createProcess(int arrived_time, int remaining_time, int pid,
                       char parallelisable);
// Remove process from linklist
void removeProcess(process *tasks, int pid);
// Check how many processors can be used to split by given task
int checkNumOfProcessors(process *task, int processors);