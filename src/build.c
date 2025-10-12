#include "sys/types.h"
#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
    void *mem;
    size_t size;
} file;

int file_write(file file, char *file_path);
const Elf64_Shdr *section_header_entry_get(file file, const Elf64_Ehdr header);
int file_mmap(const char *file_name, file *file);

int
main() {
    char *path_target = "./Famine";
    char *path_source = "./scaffolding";

    file file_source;
    if (file_mmap(path_source, &file_source)) {
        printf("error allocating %s\n", path_source);
        return (1);
    }

    file file_target;
    if (file_mmap(path_target, &file_target)) {
        printf("error allocating %s\n", path_target);
        return (1);
    }

    Elf64_Ehdr *header_source;
    header_source = file_source.mem;

    const Elf64_Shdr *section_header_entry_source = section_header_entry_get(file_source, *header_source);
    if (section_header_entry_source == NULL) {
        printf("could not find .text section in %s\n", path_source);
        return (1);
    }

    u_int64_t value = 1;
    memcpy(file_target.mem + 0x1000, &value, 8);

    value = 0x18;
    memcpy(file_target.mem + 0x1008, &value, 8);

    value = 0x18 + section_header_entry_source->sh_size;
    memcpy(file_target.mem + 0x1010, &value, 8);

    printf("Section header size: %lx\n", section_header_entry_source->sh_size);
    printf("Section header offset: %lx\n", section_header_entry_source->sh_offset);
    memcpy(file_target.mem + 0x1018, file_source.mem + section_header_entry_source->sh_offset, section_header_entry_source->sh_size);

    if (file_write(file_target, path_target)) {
        printf("error while writing back to %s\n", path_target);
        return (1);
    }

    return (0);
}

const Elf64_Shdr *
section_header_entry_get(file file, const Elf64_Ehdr header) {
    assert(file.mem != NULL);
    assert(header.e_shoff + header.e_shnum * header.e_shentsize <= file.size);

    const Elf64_Shdr *section_header_table = file.mem + header.e_shoff;

    for (int i = 0; i < header.e_shnum; i++) {
        const Elf64_Shdr *sh = section_header_table + i;
        if (sh->sh_addr <= header.e_entry && (sh->sh_addr + sh->sh_size) > header.e_entry) {
            return sh;
        }
    }

    fprintf(stderr, "error: could not find section header that is pointed to by e_entry\n");
    return NULL;
}

int
file_munmap(const file file) {
    return munmap(file.mem, file.size);
}

int
file_mmap(const char *file_name, file *file) {
    assert(file_name != NULL);
    assert(file != NULL);

    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror(file_name);
        return 1;
    }

    int off = lseek(fd, 0, SEEK_END);
    if (off == -1) {
        if (errno != 0) {
            perror(file_name);
        } else {
            fprintf(stderr,
                    "Could not lseek on fd %d. You probably passed a directory as a "
                    "file, but we are not allowed to use fcntl so all we can do is "
                    "guess - bye\n",
                    fd);
        }
        close(fd);
        return 1;
    }
    file->size = off;

    off = lseek(fd, 0, SEEK_SET);
    if (off != 0) {
        close(fd);
        perror(file_name);
        return 1;
    }

    file->mem = mmap(NULL, file->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    if (file->mem == MAP_FAILED) {
        perror("mem");
        return 1;
    }

    return 0;
}

int
file_write(file file, char *file_path) {
    int fd = open(file_path, O_CREAT | O_RDWR, 0755);
    if (fd == -1) {
        perror("woody");
        return 1;
    }
    size_t bytes_written = write(fd, file.mem, file.size);
    if (bytes_written != file.size) {
        close(fd);
        perror("woody");
        return 1;
    }

    close(fd);
    return 0;
}
