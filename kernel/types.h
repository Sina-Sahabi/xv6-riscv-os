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

#endif  //!__TYPES__H__FSJLDLSNS_BDJJD_BSSSNSP
