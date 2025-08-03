#include "horsemen.h"
#include <assert.h>
#include <dirent.h>
#include <elf.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int ac, char **av) {
    if (ac != 2) exit(42);

    char path_exe[15]; // "/proc/self/exe" + '\0'

    path_exe[0] = '/';
    path_exe[1] = 'p';
    path_exe[2] = 'r';
    path_exe[3] = 'o';
    path_exe[4] = 'c';
    path_exe[5] = '/';
    path_exe[6] = 's';
    path_exe[7] = 'e';
    path_exe[8] = 'l';
    path_exe[9] = 'f';
    path_exe[10] = '/';
    path_exe[11] = 'e';
    path_exe[12] = 'x';
    path_exe[13] = 'e';
    path_exe[14] = '\0';

    file file_self;
    if (file_mmap(path_exe, &file_self)) return 0;

    Elf64_Ehdr *header = file_self.mem;

    const Elf64_Shdr *section_self_text = section_header_entry_get(file_self, *header);
    if (section_self_text == NULL) return 0;

    const Elf64_Phdr *segment_self_text = program_header_by_section_header_get(file_self, *header, *section_self_text);
    if (segment_self_text == NULL) return 0;

    printf("%lx\n", section_self_text->sh_offset);

    // file file_target;
    // if (file_mmap(av[1], &file_target)) return 0;

    int fd_target = open(av[1], O_CREAT | O_RDWR, 0755);
    int offset_inject = lseek(fd_target, 0, SEEK_END);
    if (offset_inject == 0) return 1;
    (void)write(fd_target, file_self.mem + segment_self_text->p_offset, segment_self_text->p_filesz);

    close(fd_target);

    file file_target;
    file_mmap(av[1], &file_target);

    // append to the end of the target the code
    // mmap the whole file
    // check which are free segment to use for letting the elf loader know that we want to load something int
    //   seg

    file_munmap(file_self);
    // file_munmap(file_target);
}
