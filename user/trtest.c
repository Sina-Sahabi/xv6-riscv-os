#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "user/user.h"

int main()
{
  if (fork() == 0) {
    int *ptr = (int*)-1;
    fork();
    sleep(5);
    printf("%d\n", *ptr);
    exit(0);
  }

  if (fork() == 0) {
    int *ptr = (int*)-1;
    printf("%d\n", *ptr);
    exit(0);
  }

  if (fork() == 0) {
    int *ptr = (int*)-1;
    printf("%d\n", *ptr);
    exit(0);
  }

  sleep(20);

  struct report_traps rt;

  if (rptrap(&rt))
    exit(-1);

  printf("number of exceptions: %d\n", rt.count);
  printf("PID\tPNAME\tscause\t\t\tsepc\t\t\tstval\n");
  for (int i = 0; i < rt.count; i++) {
    printf("%d\t%s\t%p\t%p\t%p\n", rt.reports[i].pid, rt.reports[i].name,
    rt.reports[i].scause, rt.reports[i].sepc, rt.reports[i].stval);
  }

  exit(0);
}
