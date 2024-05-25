#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  printf("My first xv6 program\n");
  int pid;
  if (!(pid = fork())) {
    for(;;);
  }
  if (!fork()) {
    sleep(800);
    kill(pid);
  }

  exit(0);
}
