#include <dirent.h>
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    off_t size;
    char path[1024];
} CodeCave;

int
compare_code_caves(const void *a, const void *b) {
    off_t size_a = ((CodeCave *)a)->size;
    off_t size_b = ((CodeCave *)b)->size;
    return (size_a > size_b) - (size_a < size_b);
}

void
print_code_caves(const char *filename, CodeCave *caves, int *cave_count) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Failed to get file stats");
        close(fd);
        return;
    }

    // Map the ELF file into memory
    void *file_map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_map == MAP_FAILED) {
        perror("Failed to map file into memory");
        close(fd);
        return;
    }

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_map;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "Not an ELF file\n");
        munmap(file_map, st.st_size);
        close(fd);
        return;
    }

    Elf64_Shdr *shdr = (Elf64_Shdr *)((char *)file_map + ehdr->e_shoff);
    Elf64_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];
    char *strtab = (char *)file_map + shstrtab->sh_offset;

    // Iterate through sections to find gaps
    for (int i = 1; i < ehdr->e_shnum; i++) {
        Elf64_Shdr *current = &shdr[i];
        if (current->sh_type == SHT_NULL || current->sh_size == 0 || current->sh_type == SHT_PROGBITS) continue;

        // Find the next section
        Elf64_Shdr *next = (i + 1 < ehdr->e_shnum) ? &shdr[i + 1] : NULL;

        // Calculate the gap between the end of the current section and the start of the next
        off_t current_end = current->sh_offset + current->sh_size;
        off_t next_start = (next) ? next->sh_offset : st.st_size;

        if (current_end < next_start) {
            off_t gap_size = next_start - current_end;
            if (gap_size > 0x10) {
                caves[*cave_count].size = gap_size;
                strncpy(caves[*cave_count].path, filename, sizeof(caves[*cave_count].path) - 1);
                (*cave_count)++;
            }
        }
    }

    munmap(file_map, st.st_size);
    close(fd);
}

void
scan_directory(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    char path[1024];
    CodeCave caves[1024];
    int cave_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);
        struct stat st;
        if (stat(path, &st) == -1) {
            perror("Failed to get file stats");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Recursively scan subdirectories
            scan_directory(path);
        } else if (S_ISREG(st.st_mode)) {
            // Check if the file is an ELF file
            int fd = open(path, O_RDONLY);
            if (fd == -1) {
                perror("Failed to open file");
                continue;
            }

            Elf64_Ehdr ehdr;
            if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
                perror("Failed to read ELF header");
                close(fd);
                continue;
            }

            close(fd);

            if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) {
                // It's an ELF file, process it
                print_code_caves(path, caves, &cave_count);
            }
        }
    }

    closedir(dir);

    // Sort the code caves by size
    qsort(caves, cave_count, sizeof(CodeCave), compare_code_caves);

    // Print table header
    printf("| Code Cave Size (Dec) | Code Cave Size (Hex) | File Path                                              |\n");
    printf("|----------------------|----------------------|--------------------------------------------------------|\n");

    // Print sorted code caves
    for (int i = 0; i < cave_count; i++) {
        printf("| %10ld | 0x%10lx | %-50s |\n", caves[i].size, caves[i].size, caves[i].path);
    }
}

int
main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    scan_directory(argv[1]);
    return EXIT_SUCCESS;
}
