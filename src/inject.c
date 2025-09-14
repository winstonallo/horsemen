__attribute__((always_inline)) static inline long
sys(long n, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    register long r9 __asm__("r9") = arg6;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

#include "syscall.h"

struct mydata {
    int x;
    char y;
    char msg[16];
};

void
_start(void) {
    struct mydata d = {.x = 42, .y = 'a', .msg = "struct demo\n"};

    sys(1, 1, (long)&(d.y), 1, 0, 0, 0);

    sys(60, 2, 0, 0, 0, 0, 0);
}
