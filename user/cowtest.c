#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{

  printf("initiating test...");
  if (!fork()) {
    sbrk((PHYSTOP - KERNBASE) * 2 / 3);
    fork();
    sleep(800);
  } else {
    printf("\ntest is running\n");
  }

  exit(0);
}
