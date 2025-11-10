#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
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
    char *path_target = "./build/scaffolding";
    char *path_source = "./build/builder";

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

    Elf64_Ehdr *header = file_source.mem;
    Elf64_Shdr *section_header_text = NULL;
    Elf64_Shdr *sh_table = file_source.mem + header->e_shoff;
    for (int i = 0; i < header->e_shnum; i++) {
        Elf64_Shdr *sh = sh_table + i;
        if (sh->sh_flags == (SHF_ALLOC | SHF_EXECINSTR)) section_header_text = sh;
    }

    header = file_target.mem;
    Elf64_Phdr *ph_table = file_target.mem + header->e_phoff;
    for (int i = 0; i < header->e_phnum; i++) {
        Elf64_Phdr *ph = ph_table + i;
        if (ph->p_flags == (PF_R | PF_X)) {

            uint64_t builder_size = section_header_text->sh_size;
            uint64_t builder_start_offset_source = section_header_text->sh_offset;
            void *builder_start_target = file_target.mem + ph->p_offset + ph->p_filesz;
            void *builder_start_source = file_source.mem + builder_start_offset_source;
            memcpy(builder_start_target, builder_start_source, builder_size);
            ph->p_filesz += builder_size;
            ph->p_memsz += builder_size;

            uint64_t *old_e_entry = builder_start_target + builder_size - 24;
            *old_e_entry = header->e_entry;
            printf("old _entry %lx\n", *old_e_entry);
            uint64_t diff_addr_to_offset = 0x400000;
            header->e_entry = (uint64_t)builder_start_target - (uint64_t)file_target.mem + diff_addr_to_offset;

            uint64_t *table_entry_target_num = builder_start_target + builder_size - 8;
            uint64_t *table_entry_target_offset = builder_start_target + builder_size - 16;

            *table_entry_target_offset = ph->p_offset + ph->p_filesz;
            *table_entry_target_num = 0x1;

            uint64_t *table_start = file_target.mem + *table_entry_target_offset;

            table_start[0] = *old_e_entry - diff_addr_to_offset;
            table_start[1] = ph->p_offset + ph->p_filesz - builder_size - table_start[0];

            *old_e_entry = 0;
            printf("%lx\n", ph->p_offset);
            printf("%lx\n", table_start[0]);
            printf("%lx\n", table_start[1]);
            printf("%lx\n", *table_entry_target_offset);
            printf("%lx\n", *table_entry_target_num);
        }
    }

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
