#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Textovy vstup: size musi byt alespon 2.
 * Vraci pocet prectenych bajtu, 0 pri EOF nebo -1 pri chybe.
 * read() nemusi naplnit cely buffer ani precist cely radek. */
ssize_t read_chars(int fd, char buffer[], size_t size)
{
    ssize_t count = read(fd, buffer, size - 1);
    if (count >= 0) {
        buffer[count] = '\0';
    }
    return count;
}

/* Cte do '\n', EOF nebo zaplneni bufferu. size musi byt alespon 2.
 * Novy radek ponecha v bufferu, aby prazdny radek mel delku 1.
 * Dlouhy radek vraci po castech; dalsi volani pokracuje ve cteni. */
ssize_t read_line(int fd, char buffer[], size_t size)
{
    size_t length = 0;
    while (length < size - 1) {
        char c;
        ssize_t count = read(fd, &c, 1);
        if (count == -1) {
            return -1;
        }
        if (count == 0) {
            break;
        }
        buffer[length] = c;
        length++;
        if (c == '\n') {
            break;
        }
    }
    buffer[length] = '\0';
    return (ssize_t)length;
}

/* write() muze zapsat jen cast textu. Opakujeme ho do zapsani vseho.
 * Vraci 0 pri uspechu nebo -1 pri chybe. Text nesmi obsahovat vnitrni '\0'. */
int write_text(int fd, const char text[])
{
    size_t length = strlen(text);
    size_t written = 0;
    while (written < length) {
        ssize_t count = write(fd, text + written, length - written);
        if (count <= 0) {
            return -1;
        }
        written += (size_t)count;
    }
    return 0;
}

/* Zkouska: printf 'hello\nworld\n' | ./io_practice line
 * Rezim chars pouzije read_chars(), rezim line pouzije read_line(). */
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Pouziti: %s chars|line\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "chars") != 0 && strcmp(argv[1], "line") != 0) {
        fprintf(stderr, "Pouziti: %s chars|line\n", argv[0]);
        return 1;
    }

    char buffer[128];
    while (1) {
        ssize_t count;
        if (strcmp(argv[1], "line") == 0) {
            count = read_line(STDIN_FILENO, buffer, sizeof(buffer));
        } else {
            count = read_chars(STDIN_FILENO, buffer, sizeof(buffer));
        }
        if (count == 0) {
            break;
        }
        if (count == -1) {
            perror("read");
            return 1;
        }
        if (write_text(STDOUT_FILENO, buffer) == -1) {
            perror("write");
            return 1;
        }
    }
    return 0;
}
