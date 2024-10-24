#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// return histroy of commands
uint64
sys_histry(void)
{
  stringData *str;
  stringData kstr;

  argaddr(0, (uint64 *)&str);

  if (str == 0) {
    return nextHistory(0);
  }

  acquire(&tickslock);
  int res = nextHistory(&kstr);
  release(&tickslock);
  
  struct proc *p = myproc();
  if(res != -1 && copyout(p->pagetable, (uint64)str, (char*)&kstr, sizeof(kstr)) < 0)
    return -1;
  
  return res;
}

// return current process info
uint64
sys_ttop(void)
{
  struct top *t;
  struct top kt;

  argaddr(0, (uint64 *)&t);

  kt.uptime = sys_uptime();
  acquire(&tickslock);
  fill_top(&kt);
  release(&tickslock);

  struct proc *p = myproc();
  if(copyout(p->pagetable, (uint64)t, (char*)&kt, sizeof(kt)) < 0)
    return -1;
  
  return 0;
}

// return childern processes
uint64
sys_chp(void)
{
  struct child_processes *cp;
  struct child_processes kcp;

  argaddr(0, (uint64 *)&cp);

  int e = fill_chp(&kcp);

  struct proc *p = myproc();
  if(copyout(p->pagetable, (uint64)cp, (char*)&kcp, sizeof(kcp)) < 0)
    return -1;
  
  return e;
}

// return childern processes' traps
uint64
sys_rptrap(void)
{
  struct report_traps *rt;
  struct report_traps krt;

  argaddr(0, (uint64 *)&rt);

  int e = reportraps(&krt);

  struct proc *p = myproc();
  if(copyout(p->pagetable, (uint64)rt, (char*)&krt, sizeof(krt)) < 0)
    return -1;
  
  return e;
}
