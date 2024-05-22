//
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x

//
// send one character to the uart.
// called by printf(), and to echo input characters,
// but not from write().
//
void
consputc(int c)
{
  if(c == BACKSPACE){
    // if the user typed backspace, overwrite with a space.
    uartputc_sync('\b'); uartputc_sync(' '); uartputc_sync('\b');
  } else {
    uartputc_sync(c);
  }
}

struct {
  struct spinlock lock;
  
  // input
#define INPUT_BUF_SIZE 128
  char buf[INPUT_BUF_SIZE];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
  int escape;
} cons;

struct { // will be protected by cons.lock
#define MAX_HISTORY 17
  stringData cmd [MAX_HISTORY];
  uint numOfCmdsInMem;
  uint currentHistory;
} historyArr;

//
// user write()s to the console go here.
//
int
consolewrite(int user_src, uint64 src, int n)
{
  int i;

  for(i = 0; i < n; i++){
    char c;
    if(either_copyin(&c, user_src, src+i, 1) == -1)
      break;
    uartputc(c);
  }

  return i;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
int
consoleread(int user_dst, uint64 dst, int n)
{
  uint target;
  int c;
  char cbuf;

  target = n;
  acquire(&cons.lock);
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cons.buffer.
    while(cons.r == cons.w){
      if(killed(myproc())){
        release(&cons.lock);
        return -1;
      }
      sleep(&cons.r, &cons.lock);
    }

    c = cons.buf[cons.r++ % INPUT_BUF_SIZE];

    if(c == C('D')){  // end-of-file
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cons.r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.
    cbuf = c;
    if(either_copyout(user_dst, dst, &cbuf, 1) == -1)
      break;

    dst++;
    --n;

    if(c == '\n'){
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  release(&cons.lock);

  return target - n;
}

void
save_history(void) {
  uint ind = historyArr.numOfCmdsInMem % MAX_HISTORY;
  uint len = 0;
  for (int i = cons.r; i < cons.w - 1; i++)
    historyArr.cmd[ind].str[len++] = cons.buf[i % INPUT_BUF_SIZE];
  historyArr.cmd[ind].str[len] = '\0';
  historyArr.cmd[ind].length = len;
  historyArr.currentHistory = ++historyArr.numOfCmdsInMem;
}


//
// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int c)
{
  acquire(&cons.lock);
  if (c == '\033') { // esc
    cons.escape = 1;
    goto consrel;
  }
      
  if (cons.escape == 1) {
    if (c == '[') {
      cons.escape = 2;
      goto consrel;
    } else { //?
      consputc('\033');
      cons.escape = 0;
    }
  }

  if (cons.escape == 2) {
    cons.escape = 0;
    if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
      switch (c)
      {
      case 'A': // code for arrow up
        if (historyArr.currentHistory > 0 &&
        historyArr.numOfCmdsInMem - historyArr.currentHistory < MAX_HISTORY - 1) {
          historyArr.currentHistory--;
          for (; cons.e < cons.w; cons.e++) {
            consputc('\033'), consputc('['), consputc('C');
          }
          while(cons.e != cons.r &&
          cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
            cons.e--;
            cons.w--;
            consputc(BACKSPACE);
          }

          for (int i = 0; i < historyArr.cmd[historyArr.currentHistory % MAX_HISTORY].length; i++) {
            consputc(cons.buf[cons.e % INPUT_BUF_SIZE] = 
            historyArr.cmd[historyArr.currentHistory % MAX_HISTORY].str[i]);
            cons.e++;
            cons.w++;
          }
        }
        break;
      case 'B': // code for arrow down
        for (; cons.e < cons.w; cons.e++) {
          consputc('\033'), consputc('['), consputc('C');
        }
        while(cons.e != cons.r &&
        cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
          cons.e--;
          cons.w--;
          consputc(BACKSPACE);
        }
        if (historyArr.currentHistory < historyArr.numOfCmdsInMem - 1) {
          historyArr.currentHistory++;

          for (int i = 0; i < historyArr.cmd[historyArr.currentHistory % MAX_HISTORY].length; i++) {
            consputc(cons.buf[cons.e % INPUT_BUF_SIZE] = 
            historyArr.cmd[historyArr.currentHistory % MAX_HISTORY].str[i]);
            cons.e++;
            cons.w++;
          }
        }
        break;
      case 'C': // code for arrow right
        if (cons.e < cons.w) {
          consputc('\033');
          consputc('[');
          consputc('C');
          cons.e++;
        }
        break;
      case 'D': // code for arrow left
        if (cons.r < cons.e) {
          consputc('\033');
          consputc('[');
          consputc('D');
          cons.e--;
        }
        break;
      }
      goto consrel;
    } else {
      consputc('\033');
      consputc('[');
    }
  }

  cons.escape = 0;
  switch(c){
  case C('P'):  // Print process list.
    procdump();
    break;
  case C('U'):  // Kill line.
    for (; cons.e < cons.w; cons.e++) {
      consputc('\033'), consputc('['), consputc('C');
    }
    while(cons.e != cons.r &&
          cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
      cons.e--;
      cons.w--;
      consputc(BACKSPACE);
    }
    break;
  case C('H'): // Backspace
  case '\x7f': // Delete key
    if(cons.e != cons.r){
      if (cons.e == cons.w)
        cons.w--;
      cons.e--;
      consputc(BACKSPACE);
    }
    break;
  default:
    if(c != 0 && cons.e-cons.r < INPUT_BUF_SIZE){
      c = (c == '\r') ? '\n' : c;
      // echo back to the user.
      consputc(c);

      // store for consumption by consoleread().
      cons.buf[cons.e++ % INPUT_BUF_SIZE] = c;
      if (cons.e > cons.w)
        cons.w = cons.e;

      if(c == '\n' || c == C('D') || cons.e-cons.r == INPUT_BUF_SIZE || c == C('C')){
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        if (c != C('C'))
          save_history();
        wakeup(&cons.r);
      }
    }
    break;
  }
  
  consrel:
  release(&cons.lock);
}


/**
 * @brief
 * return next command in history
 * @returns
 * len if ok
 * -1 if end of history
 */
int
nextHistory(stringData* result)
{
  if (result == 0) {
    historyArr.currentHistory = historyArr.numOfCmdsInMem;
    return 0;
  }
  if (!historyArr.currentHistory || historyArr.numOfCmdsInMem - historyArr.currentHistory >= MAX_HISTORY)
    return -1;

  historyArr.currentHistory--;
  result->length = historyArr.cmd[historyArr.currentHistory % MAX_HISTORY].length;
  strncpy(result->str, historyArr.cmd[historyArr.currentHistory % MAX_HISTORY].str, result->length);
  return result->length;
}


void
consoleinit(void)
{
  initlock(&cons.lock, "cons");

  uartinit();

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
}
