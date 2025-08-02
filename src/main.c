#include <dirent.h>
#include <stdio.h>

int
walk(const char *const base_path) {
    struct dirent *file;
    DIR *base = opendir(base_path);

    if (base == NULL) {
        return -1;
    }

    while ((file = readdir(base)) != NULL) {
        if (file->d_type == DT_DIR) {
            walk(file->d_name);
            printf("%s\n", file->d_name);
        }
    }

    return 0;
}

int
main() {
    walk(".");
}
