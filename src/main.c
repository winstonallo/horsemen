#include "mem.h"
#include <assert.h>
#include <dirent.h>
#include <elf.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
int
walk(const char *const base_path, void (*f)(const char *)) {
    char full_path[1024];
    struct dirent *file;
    DIR *base = opendir(base_path);

    if (base == NULL) {
        return -1;
    }
    mmap(MAP_PRIVATE);
    struct stat sb;
    lseek(1, SEEK_END, 1);
    while ((file = readdir(base)) != NULL) {
        if (ft_memcmp(file->d_name, ".", 2) == 0 || ft_memcmp(file->d_name, "..", 3) == 0) {
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, file->d_name);
        if (file->d_type == DT_DIR) {
            walk(full_path, f);
        }
        f(full_path);
    }

    return 0;
}

void
print_path(const char *const path) {
    printf("%s\n", path);
}

int
main() {
    walk(".", print_path);
}
