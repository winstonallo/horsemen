#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

typedef unsigned long long u64;

int
scall(int sysno, u64 _rdi, u64 _rsi, u64 _rdx, u64 _r10, u64 _r8, u64 _r9) {
    int _rax;
    register u64 rax asm("rax") = sysno;
    register u64 rdi asm("rdi") = _rdi;
    register u64 rsi asm("rsi") = _rsi;
    register u64 rdx asm("rdx") = _rdx;
    register u64 r10 asm("r10") = _r10;
    register u64 r8 asm("r8") = _r8;
    register u64 r9 asm("r9") = _r9;

    asm volatile("syscall" : "=a"(_rax) : "r"(rax), "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");

    return _rax;
}

const char *
sstr(const char *needle, const char *haystack) {
    size_t i = 0, j = 0;

    while (haystack[i]) {
        int k = i;
        while (haystack[k] && needle[j] && haystack[k] == needle[j]) {
            j++;
            k++;
        }
        if (!needle[j]) {
            return &haystack[i];
        }
        j = 0;
        i++;
    }
    return 0;
}

size_t
slen(const char *s) {
    size_t len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}

int
sdigit(const char *s) {
    for (int i = 0; s[i]; ++i) {
        if (s[i] < '0' || s[i] > '9') {
            return 0;
        }
    }
    return 1;
}

int
is_process_running(const char *process_name) {
    struct dirent *entry;
    char path[256];
    path[0] = '/';
    path[1] = 'p';
    path[2] = 'r';
    path[3] = 'o';
    path[4] = 'c';
    path[5] = '/';
    const char cmdline_const[] = "/cmdline";
    char cmdline[1024];

    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        return 0;
    }

    while ((entry = readdir(proc_dir))) {
        if (!sdigit(entry->d_name)) {
            continue;
        }

        const size_t len = slen(entry->d_name);
        if (len + 14 > sizeof(path)) {
            continue;
        }
        size_t i = 0;
        for (; i < len; ++i) {
            path[i + 6] = entry->d_name[i];
        }
        for (; i < sizeof(cmdline_const) + len + 6; ++i) {
            path[i + 6] = cmdline_const[i - len];
        }
        path[i] = 0;

        int fd = scall(2, (long int)path, O_RDONLY, 0, 0, 0, 0);
        if (fd == -1) {
            return 0;
        }

        long int read_bytes = scall(0, fd, (long int)cmdline, sizeof(cmdline), 0, 0, 0);
        if (read_bytes == -1) {
            return 0;
        }
        for (int i = 0; i < read_bytes; ++i) {
            if (cmdline[i] == ' ') {
                cmdline[i] = 0;
                break;
            }
        }

        if (sstr(process_name, cmdline)) {
            return 1;
        }
    }
    return 0;
}

int
main(int ac, char **av) {
    if (ac == 2) {
        return is_process_running(av[1]) != 1;
    }
}
