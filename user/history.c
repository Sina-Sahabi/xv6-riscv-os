#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  if (argc <= 2) {
    stringData str [20];
    int ind, len, n;
    
    for (ind = 0; (len = histry(&str[ind])) >= 0; ind++)
      str[ind].str[len] = 0;
    
    if (!ind) {
      printf("history not found\n");
      exit(0);
    }

    len = ind;
    while (ind--)
      printf("%d.\t%s\n", ind, str[ind].str);
    
    if (argc == 2) {
      n = atoi(argv[1]);
      if (n < 0 || n >= len) {
        fprintf(2, "history: argument error\n");
        exit(1);
      }
      printf("requested command: %s\n", str[n].str);
    }
  } else {
    fprintf(2, "history: argument error\n");
    exit(1);
  }
  exit(0);
}
