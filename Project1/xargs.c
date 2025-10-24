#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }

    char buf[512];
    char *args[MAXARG];
    int n = 0;

    // Copy command + its fixed arguments (e.g. "echo bye")
    for (int i = 1; i < argc; i++) {
        args[n++] = argv[i];
    }

    int m = 0;
    while (read(0, &buf[m], 1) == 1) {
        if (buf[m] == '\n') {
            buf[m] = 0; // Replace newline with null terminator

            // Append this line as one argument
            args[n] = buf;
            args[n + 1] = 0;

            if (fork() == 0) {
                exec(args[0], args);
                fprintf(2, "xargs: exec failed\n");
                exit(1);
            }
            wait(0);

            m = 0; // reset for next line
        } else {
            m++;
        }
    }

    exit(0);
}