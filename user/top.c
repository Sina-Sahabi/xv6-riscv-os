#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char state_name[][10] = {
  "unused", "used", "sleeping", "runnable", "running", "zombie"
};

int main(int argc, char *argv[])
{
  if (argc > 1) {
    fprintf(2, "top: argument error\n");
    exit(1);
  }

  struct top t;
  if (ttop(&t))
    exit(-1);
  
  printf("uptime:%d seconds\n", t.uptime / 10);
  printf("total process:%d\n", t.total_process);
  printf("running process:%d\n", t.running_process);
  printf("sleeping process:%d\n", t.sleeping_process);
  printf("process data:\nname\tPID\tPPID\tstate\n");
  for (int i = 0; i < t.total_process; i++) {
    printf("%s\t%d\t%d\t%s\n", t.p_list[i].name, t.p_list[i].pid, t.p_list[i].ppid, state_name[t.p_list[i].state]);
  }
  
  exit(0);
}
