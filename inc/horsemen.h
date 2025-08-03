#ifndef HORSEMEN_H
#define HORSEMEN_H

#include <elf.h>
#include <sys/types.h>

typedef struct {
    void *mem;
    size_t size;
} file;

int file_mmap(const char *file_name, file *file);
int file_munmap(const file file);
int file_write(file file, const char *path);

const Elf64_Shdr *section_header_entry_get(file file, const Elf64_Ehdr header);
Elf64_Phdr *program_header_by_section_header_get(file file, const Elf64_Ehdr header, const Elf64_Shdr section_header);
#endif
