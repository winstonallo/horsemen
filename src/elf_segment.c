#include "horsemen.h"
#include <assert.h>
#include <elf.h>
#include <stdio.h>
#include <unistd.h>

Elf64_Phdr *
program_header_by_section_header_get(file file, const Elf64_Ehdr header, const Elf64_Shdr section_header) {
    assert(file.mem != NULL);
    assert(header.e_phoff + header.e_phnum * header.e_phentsize <= file.size);

    const uint64_t sh_start = section_header.sh_offset;
    const uint64_t sh_end = section_header.sh_offset + section_header.sh_size - 1;

    Elf64_Phdr *program_header_table = file.mem + header.e_phoff;

    for (int i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr *ph = program_header_table + i;

        const uint64_t ph_start = ph->p_offset;
        const uint64_t ph_end = ph->p_offset + ph->p_filesz;

        const uint16_t section_is_inside_program_header = sh_start >= ph_start && sh_end <= ph_end;
        if (section_is_inside_program_header) {
            return ph;
        }
    }

    fprintf(stderr, "could not find program header with section header inside\n");
    return NULL;
}
