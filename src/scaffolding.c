#include <stdint.h>
__attribute__((always_inline)) inline long
sys(long n, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
  long ret;
  register long r10 __asm__("r10") = arg4;
  register long r8 __asm__("r8") = arg5;
  register long r9 __asm__("r9") = arg6;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"(n), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8),
                     "r"(r9)
                   : "rcx", "r11", "memory");
  return ret;
}

#include "syscall.h"
#include <fcntl.h>
#include <stddef.h>
#include <sys/syscall.h>

inline void ft_exit(int exit_code) { sys(SYS_exit, exit_code, 0, 0, 0, 0, 0); }

inline int ft_write(int fd, void *ptr, size_t size) {
  return sys(SYS_write, fd, (long)ptr, size, 0, 0, 0);
}

inline int ft_open(char *path, int flags, mode_t mode) {
  return sys(SYS_open, (long)path, flags, mode, 0, 0, 0);
}

inline int ft_getdents64(int fd, char dirp[], size_t count) {
  return sys(SYS_getdents64, fd, (long)dirp, count, 0, 0, 0);
}

inline int ft_strlen(char *str) {
  int i = 0;
  while (str[i] != '\0')
    i++;
  return (i);
}

typedef struct linux_dirent64 {
  uint64_t d_ino;          /* 64-bit inode number */
  uint64_t d_off;          /* Not an offset; see getdents() */
  unsigned short d_reclen; /* Size of this dirent */
  unsigned char d_type;    /* File type */
  char d_name[];           /* Filename (null-terminated) */
} dirent64;

int main(void) {
  char path[2];
  path[0] = '.';
  path[1] = '\0';

  int fd = ft_open(path, O_RDONLY, O_DIRECTORY);

  const uint64_t DIR_P_SIZE = 10000;
  char dirp[DIR_P_SIZE];
  int bytes_read = ft_getdents64(fd, dirp, DIR_P_SIZE);
  if (bytes_read < 0)
    ft_exit(1);

  dirent64 *dirent_cur = (dirent64 *)dirp;
  int i = 0;
  while ((((long)dirent_cur) - (long)dirp) < bytes_read) {
    ft_write(1, dirent_cur->d_name, ft_strlen(dirent_cur->d_name));
    ft_write(1, "\n", 1);
    dirent_cur = (dirent64 *)((void *)dirent_cur + dirent_cur->d_reclen);
    i++;
  }
  ft_exit(i);
}
