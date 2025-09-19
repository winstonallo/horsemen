#define PTRACE_TRACEME 0

typedef unsigned long long u64;
typedef long long i64;
typedef int i32;
typedef char bool;

struct linux_dirent64 {
    u64 d_ino;
    long d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};

i64
scall(i32 sysno, u64 _rdi, u64 _rsi, u64 _rdx) {
    i64 _rax;
    register u64 rax asm("rax") = sysno;
    register u64 rdi asm("rdi") = _rdi;
    register u64 rsi asm("rsi") = _rsi;
    register u64 rdx asm("rdx") = _rdx;

    asm volatile("syscall" : "=a"(_rax) : "r"(rax), "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r11", "memory");

    return _rax;
}

const char *
sstr(const char *const needle, const char *const haystack) {
    u64 i = 0, j = 0;

    while (haystack[i]) {
        i32 k = i;
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

u64
slen(const char *const s) {
    u64 len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}

bool
sdigit(const char *const s) {
    for (u64 i = 0; s[i]; ++i) {
        if (s[i] < '0' || s[i] > '9') {
            return 0;
        }
    }
    return 1;
}

// This function was written with no regard for safety whatsoever.
void
scat(char *const to, const char *const from) {
    u64 i = 0, j = 0;
    for (; to[i]; ++i) {
        /* ... */
    }
    for (; from[j]; ++j, ++i) {
        to[i] = from[j];
    }
    to[i + j] = 0;
}

void
szero(char *const s, u64 bytes) {
    for (u64 i = 0; i < bytes; ++i) {
        s[i] = 0;
    }
}

char *
schr(char *const s, char c) {
    for (u64 i = 0; s[i]; ++i) {
        if (s[i] == c) {
            return &s[i];
        }
    }
    return 0;
}

// Returns `true` if any process whose `argv[0]` includes `process_name`
// is currently running on the machine.
bool
is_process_running(const char *process_name) {
    char path[256] = {0};
    char cmdline_buffer[1024];
    char getdents_buffer[4096];

    i64 proc_dir_fd = scall(2, (u64) "/proc", 0, 0);
    if (!proc_dir_fd) {
        return 0;
    }

    scat(path, "/proc/");
    while (1) {

        i64 bytes_read = scall(217, proc_dir_fd, (u64)getdents_buffer, sizeof(getdents_buffer));
        if (bytes_read <= 0) {
            break;
        }
        i64 offset = 0;
        while (offset < bytes_read) {
            struct linux_dirent64 *entry = (struct linux_dirent64 *)(getdents_buffer + offset);
            offset += entry->d_reclen;
            if (!sdigit(entry->d_name)) {
                continue;
            }
            const u64 len = slen(entry->d_name);
            if (len + 14 > sizeof(path)) {
                continue;
            }
            scat(path, entry->d_name);
            scat(path, "/cmdline");
            i32 fd = scall(2, (u64)path, 0, 0);
            if (fd == -1) {
                continue;
            }
            szero(&path[6], sizeof(path) - 6);
            i64 read_bytes = scall(0, fd, (u64)cmdline_buffer, sizeof(cmdline_buffer));
            if (read_bytes == -1) {
                continue;
            }
            scall(3, fd, 0, 0);
            char *space = schr(cmdline_buffer, ' ');
            if (space) {
                *space = 0;
            }
            if (sstr(process_name, cmdline_buffer)) {
                scall(3, proc_dir_fd, 0, 0);
                return 1;
            }
        }
    }
    scall(3, proc_dir_fd, 0, 0);
    return 0;
}

// Returns `true` if the program is currently running in a debugger. This is based
// on the fact that a process can only be traced by one other process at a time.
// If the process is already being traced, `ptrace` will fail, indicating that
// we are being debugged.
bool
in_debugger() {
    return scall(101, PTRACE_TRACEME, 0, 1, 0) == -1;
}
