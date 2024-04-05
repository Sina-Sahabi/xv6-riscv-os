#ifndef __TYPES__H__FSJLDLSNS_BDJJD_BSSSNSP
#define __TYPES__H__FSJLDLSNS_BDJJD_BSSSNSP

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;

typedef uint64 pde_t;

typedef struct {
#define MAX_STR_DATA 128
  char str [MAX_STR_DATA];
  uint length;
} stringData;

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
struct proc_info {
  char name [16];
  int pid;
  int ppid;
  enum procstate state;
};

struct top {
  struct proc_info p_list[64];
  long uptime;
  int total_process;
  int running_process;
  int sleeping_process;
};

#endif  //!__TYPES__H__FSJLDLSNS_BDJJD_BSSSNSP
