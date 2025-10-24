#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Forward declare the function with the noreturn attribute.
void sieve(int left_fd) __attribute__((noreturn));

// The recursive function that represents one stage of the prime sieve pipeline.
// It reads numbers from its left neighbor (via the 'left_fd' pipe) and
// writes filtered numbers to its right neighbor.
void sieve(int left_fd) {
  int prime;
  int n;
  int p[2]; // Pipe for the right neighbor

  // Read the first number from the left pipe. This is our prime.
  // If read returns 0, the pipe was closed and there are no more numbers.
  // This is the base case for the recursion.
  if (read(left_fd, &prime, sizeof(prime)) == 0) {
    close(left_fd);
    exit(0);
  }

  // We found a prime, print it.
  printf("prime %d\n", prime);

  // Create a new pipe for the next stage of the sieve.
  pipe(p);

  if (fork() == 0) {
    // --- Child Process ---
    // This child will become the next filter in the pipeline.

    // It doesn't need the write-end of the new pipe.
    close(p[1]);
    // It also doesn't need the pipe from its grandparent to its parent.
    close(left_fd);

    // Recursively call sieve, with the read-end of the new pipe as its left input.
    sieve(p[0]);

  } else {
    // --- Parent Process ---
    // This parent is the filter for the 'prime' it found.

    // It doesn't need the read-end of the new pipe.
    close(p[0]);

    // Read subsequent numbers from its left neighbor.
    while (read(left_fd, &n, sizeof(n)) > 0) {
      // If the number is not divisible by our prime, pass it to the right.
      if (n % prime != 0) {
        write(p[1], &n, sizeof(n));
      }
    }

    // All numbers from the left are processed. Close our pipes.
    close(left_fd);
    close(p[1]); // This signals EOF to our child.

    // Wait for the child process (and all its descendants) to finish.
    wait(0);
  }

  exit(0);
}

int main(int argc, char *argv[]) {
  int p[2];

  // Create the first pipe.
  pipe(p);

  if (fork() == 0) {
    // --- Child Process (The start of the sieve pipeline) ---

    // It only needs to read from the pipe, so close the write-end.
    close(p[1]);
    
    // Start the sieve process.
    sieve(p[0]);

  } else {
    // --- Parent Process (The number generator) ---

    // It only needs to write to the pipe, so close the read-end.
    close(p[0]);

    // Feed the numbers 2 through 280 into the pipeline.
    for (int i = 2; i <= 280; i++) {
      // We write 4 bytes (the size of an int).
      if (write(p[1], &i, sizeof(i)) != sizeof(i)) {
        fprintf(2, "write error\n");
        exit(1);
      }
    }

    // Done writing. Close the write-end of the pipe.
    // This will cause the first child's `read` to eventually return 0 (EOF).
    close(p[1]);

    // Wait for the entire pipeline to terminate.
    wait(0);
  }

  exit(0);
}


