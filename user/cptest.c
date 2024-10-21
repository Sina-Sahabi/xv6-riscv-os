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

void clear_screen(int n) {
  for (int i = 0; i < n + 4; i++) {
    printf("\033[A");
    printf("\33[2K\r");
  }
}

void make_children() {
  fork();
  sleep(10);
  fork();
  sleep(1000);
  fork();
  sleep(600);
  exit(0);
}

int main(int argc, char *argv[])
{
  if (argc > 1) {
    fprintf(2, "top: argument error\n");
    exit(1);
  }

  uint64 ti = uptime();
  int pid = fork();

  if (!pid) {
    struct child_processes cp;

    if (!fork())
      make_children();

    for (;;) {
      if (chp(&cp))
        exit(-1);

      int x;
      printf("uptime: %d second(s)\n", (uptime() - ti) / 100);
      printf("total children processes: %d\n", x = cp.count);
      printf("processes data:\nname\tPID\tPPID\tstate\n");
      for (int i = 0; i < x; i++) {
        printf("%s\t%d\t%d\t%s\n",
        cp.processes[i].name, cp.processes[i].pid, cp.processes[i].ppid, state_name[cp.processes[i].state]);
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
