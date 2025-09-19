#define PTRACE_TRACEME 0

typedef int i32;
typedef long long int i64;
typedef unsigned long long int u64;

i64
scall(i32 sysno, u64 _rdi, u64 _rsi, u64 _rdx, u64 _r10) {
    i64 _rax;
    register u64 rax asm("rax") = sysno;
    register u64 rdi asm("rdi") = _rdi;
    register u64 rsi asm("rsi") = _rsi;
    register u64 rdx asm("rdx") = _rdx;
    register u64 r10 asm("r10") = _r10;

    asm volatile("syscall" : "=a"(_rax) : "r"(rax), "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10) : "rcx", "r11", "memory");

    return _rax;
}

// Returns `1` if the program is currently running in a debugger. This is based
// on the fact that a process can only be traced by one other process at a time.
// If the process is already being traced, `ptrace` will fail, indicating that
// we are being debugged.
int
in_debugger() {
    return scall(101, PTRACE_TRACEME, 0, 1, 0) == -1;
}
