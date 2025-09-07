#include <unistd.h>
void
main() {
    syscall(60, 42);
}
