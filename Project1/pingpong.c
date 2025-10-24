#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // We need two pipes:
  // 1. parent -> child
  // 2. child -> parent
  int p2c[2]; // Pipe for parent to child
  int c2p[2]; // Pipe for child to parent

  // Create both pipes. pipe() returns -1 on error.
  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pingpong: pipe creation failed\n");
    exit(1);
  }

  // A buffer to hold the single byte for ping-pong
  char buf[1];

  // Fork to create the child process
  int pid = fork();

  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // --- CHILD PROCESS ---

    // The child doesn't need to write to the p2c pipe or read from the c2p pipe.
    // It's good practice to close unused file descriptors.
    close(p2c[1]); // Close write end of parent->child pipe
    close(c2p[0]); // Close read end of child->parent pipe

    // Read the "ping" byte from the parent.
    // read() returns the number of bytes read. We expect 1.
    if (read(p2c[0], buf, 1) != 1) {
      fprintf(2, "pingpong: child failed to read from parent\n");
      exit(1);
    }
    
    // Print the required message.
    printf("%d: received ping\n", getpid());

    // Write the byte back to the parent as a "pong".
    if (write(c2p[1], buf, 1) != 1) {
      fprintf(2, "pingpong: child failed to write to parent\n");
      exit(1);
    }

    // Close remaining pipe ends before exiting.
    close(p2c[0]);
    close(c2p[1]);
    
    exit(0);

  } else {
    // --- PARENT PROCESS ---

    // The parent doesn't need to read from the p2c pipe or write to the c2p pipe.
    close(p2c[0]); // Close read end of parent->child pipe
    close(c2p[1]); // Close write end of child->parent pipe

    // Write the "ping" byte to the child.
    buf[0] = 'B'; // Any byte will do.
    if (write(p2c[1], buf, 1) != 1) {
      fprintf(2, "pingpong: parent failed to write to child\n");
      exit(1);
    }

    // Wait and read the "pong" byte from the child.
    if (read(c2p[0], buf, 1) != 1) {
      fprintf(2, "pingpong: parent failed to read from child\n");
      exit(1);
    }

    // Print the required message.
    printf("%d: received pong\n", getpid());

    // Close remaining pipe ends before exiting.
    close(p2c[1]);
    close(c2p[0]);

    // Optional: wait for child to fully exit to prevent a zombie process,
    // though in this simple case it's not strictly necessary as parent exits immediately.
    wait(0);
    
    exit(0);
  }
}