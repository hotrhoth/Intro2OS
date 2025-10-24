#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int left_fd) __attribute__((noreturn));

void sieve(int left_fd) {
  int prime;
  int n;
  int p[2]; 

  if (read(left_fd, &prime, sizeof(prime)) == 0) {
    close(left_fd);
    exit(0);
  }

  printf("prime %d\n", prime);

  pipe(p);

  if (fork() == 0) {
    // --- Child Process ---
    
    close(p[1]);
    close(left_fd);
    sieve(p[0]);

  } 
  else {
    // --- Parent Process ---

    close(p[0]);

    while (read(left_fd, &n, sizeof(n)) > 0) {
      if (n % prime != 0) {
        write(p[1], &n, sizeof(n));
      }
    }

    close(left_fd);
    close(p[1]); 
    wait(0);
  }

  exit(0);
}

int main(int argc, char *argv[]) {
  int p[2];

  pipe(p);

  if (fork() == 0) {
    close(p[1]);
    sieve(p[0]);
  } 
  else {
    close(p[0]);
    for (int i = 2; i <= 280; i++) {
      if (write(p[1], &i, sizeof(i)) != sizeof(i)) {
        fprintf(2, "write error\n");
        exit(1);
      }
    }
    close(p[1]);
    wait(0);
  }

  exit(0);
}


