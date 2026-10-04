#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

int main(int argc, char *argv[]) {
    int use_lstat = 0;
    int first_file = 1;

    if (argc > 1 && strcmp(argv[1], "-l") == 0) {
        use_lstat = 1;
        first_file = 2;
    }
    if (first_file >= argc) {
        fprintf(stderr, "Usage: %s [-l] file...\n", argv[0]);
        return 1;
    }

    int had_error = 0;

    for (int i = first_file; i < argc; i++) {
        const char *path = argv[i];
        struct stat info;
        int result;

        if (use_lstat) {
            result = lstat(path, &info);
        } else {
            result = stat(path, &info);
        }

        if (result == -1) {
            perror(path);
            had_error = 1;
            continue;
        }

        const char *type;
        if (S_ISREG(info.st_mode)) {
            type = "regular file";
        }
        else if (S_ISDIR(info.st_mode)) {
            type = "directory";
        }
        else if (S_ISLNK(info.st_mode)) {
            type = "symbolic link";
        }
        else {
            type = "other";
        }

        char permissions[10] = "---------";
        permissions[0] = (info.st_mode & S_IRUSR) ? 'r' : '-';
        permissions[1] = (info.st_mode & S_IWUSR) ? 'w' : '-';
        permissions[2] = (info.st_mode & S_IXUSR) ? 'x' : '-';
        permissions[3] = (info.st_mode & S_IRGRP) ? 'r' : '-';
        permissions[4] = (info.st_mode & S_IWGRP) ? 'w' : '-';
        permissions[5] = (info.st_mode & S_IXGRP) ? 'x' : '-';
        permissions[6] = (info.st_mode & S_IROTH) ? 'r' : '-';
        permissions[7] = (info.st_mode & S_IWOTH) ? 'w' : '-';
        permissions[8] = (info.st_mode & S_IXOTH) ? 'x' : '-';

        printf("Name: %s\n", path);
        printf("Type: %s\n", type);
        printf("Size: %" PRIdMAX "\n", (intmax_t)info.st_size);
        printf("Permissions: %s\n", permissions);
        printf("Inode: %" PRIuMAX "\n", (uintmax_t)info.st_ino);

        printf("\n");
    }

    return had_error;
}

// gcc -Wall -Wextra -g fileinfo.c -o fileinfo