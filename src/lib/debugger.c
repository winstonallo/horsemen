#include <unistd.h>

#define PTRACE_TRACEME 0

// Returns `1` if the program is currently running in a debugger. This is based
// on the fact that a process can only be traced by one other process at a time.
// If the process is already being traced, `ptrace` will fail, indicating that
// we are being debugged.
int
in_debugger() {
    return syscall(101, PTRACE_TRACEME, 0, 1, 0) == -1;
}
