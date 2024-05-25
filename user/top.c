#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "user/user.h"

char *state_name[] = {
  [UNUSED]    "unused",
  [USED]      "used  ",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
};

inline void clear_screen(int n) {
  for (int i = 0; i < n + 6; i++) {
    printf("\033[A");
    printf("\33[2K\r");
  }
}

int main(int argc, char *argv[])
{
  if (argc > 1) {
    fprintf(2, "top: argument error\n");
    exit(1);
  }

  int pid = fork();

  if (!pid) {
    struct top t;

    for (;;) {
      if (ttop(&t))
        exit(-1);

      int x;
      printf("uptime: %d second(s)\n", t.uptime / 100);
      printf("total process:%d\n", x = t.total_process);
      printf("running process:%d\n", t.running_process);
      printf("sleeping process:%d\n", t.sleeping_process);
      printf("process data:\nname\tPID\tPPID\tstate\ttime\tCPU%%\n");
      for (int i = 0; i < x; i++) {
        printf("%s\t%d\t%d\t%s\t%d\t%d.%d\n",
        t.p_list[i].name, t.p_list[i].pid, t.p_list[i].ppid, state_name[t.p_list[i].state], t.p_list[i].ctime / 100,
        t.p_list[i].rtime * 100L / t.uptime, (t.p_list[i].rtime * 10000L / t.uptime) % 100);
      }

      sleep(200);
      clear_screen(x);
    }
  } else {
    for (;;) {
      char ch;
      read(0, &ch, 1);
      if (ch == ('C' - '@')) {
        kill(pid);
        break;
      }
    }
  }
  
  exit(0);
}
