#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        perror(path);
        return 1;
    }

    off_t position = lseek(fd, 0, SEEK_END);
    if (position == (off_t)-1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    while (1) {
        struct stat info;

        if (fstat(fd, &info) == -1) {
            perror("fstat");
            break;
        }

        if (info.st_size < position) {
            if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
                perror("lseek");
                break;
            }

            position = 0;
        }

        char buffer[4096];

        while (position < info.st_size) {
            ssize_t count = read(fd, buffer, sizeof(buffer));

            if (count == -1) {
                if (errno == EINTR) {
                    continue;
                }

                perror("read");
                close(fd);
                return 1;
            }

            if (count == 0) {
                break;
            }

            size_t total_written = 0;

            while (total_written < (size_t)count) {
                ssize_t written = write(STDOUT_FILENO, buffer + total_written, (size_t)count - total_written);

                if (written == -1) {
                    if (errno == EINTR) {
                        continue;
                    }

                    perror("write");
                    close(fd);
                    return 1;
                }

                if (written == 0) {
                    fprintf(stderr, "Output made no progress\n");
                    close(fd);
                    return 1;
                }

                total_written += (size_t)written;
            }

            position += count;
        }

        sleep(1);
    }

    close(fd);
    return 1;
}
