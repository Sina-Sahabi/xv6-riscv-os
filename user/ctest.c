#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

const int N = 1e9;

inline void
count(int id)
{
  for (int i = 0; i < N; i++);
  printf("%d is done!\n", id);
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(2, "test: argument error\n");
    exit(1);
  }

  int x = atoi(argv[1]);

  for (int i = 0; i < x; i++) {
    if (!fork()) {
      count(i + 1);
      exit(0);
    }
  }

  exit(0);
}
