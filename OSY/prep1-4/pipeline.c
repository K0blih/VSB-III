#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* Roura nema radky: cteme znaky az do '\n'.
 * Vraci 1 = radek, 0 = konec vstupu, -1 = chyba. */
int read_line(int fd, char *line, int capacity)
{
    int length = 0;
    char c;
    ssize_t n;
    while ((n = read(fd, &c, 1)) > 0) {
        if (c == '\n') {
            break;
        }
        if (length == capacity - 1) {
            fprintf(stderr, "Prilis dlouhy radek\n");
            return -1;
        }
        line[length++] = c;
    }
    if (n < 0) {
        perror("read");
        return -1;
    }
    if (length > 0 && line[length - 1] == '\r') {
        length--;
    }
    line[length] = '\0';
    return n > 0 || length > 0;
}

/* write() muze zapsat jen cast textu, proto pokracujeme se zbytkem. */
int write_text(int fd, const char *text)
{
    size_t remaining = strlen(text);
    while (remaining > 0) {
        ssize_t n = write(fd, text, remaining);
        if (n <= 0) {
            return -1;
        }
        text += n;
        remaining -= (size_t)n;
    }
    return 0;
}

int main(int argc, char **argv)
{
    /* Hlavicky vypisujeme ihned, bez bufferu kopirovaneho pri fork(). */
    setbuf(stdout, NULL);
    signal(SIGPIPE, SIG_IGN); /* Chybu zapisu osetrime misto ukonceni signalem. */
    const char *filename = "jmena.txt";
    if (argc > 1) {
        filename = argv[1];
    }
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("open");
        return 1;
    }

    int pipes[3][2];
    for (int i = 0; i < 3; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            close(file);
            return 1;
        }
    }

    /* Ctvrta roura: potomek 3 potvrzuje rodici dokonceni jmena. */
    int ack[2];
    if (pipe(ack) == -1) {
        perror("pipe ack");
        for (int i = 0; i < 3; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        close(file);
        return 1;
    }

    int role = 0; /* 0 = rodic, 1 az 3 = potomci. */
    int children = 0;
    int error = 0;
    /* Potomka 1 vytvari pouze puvodni rodic. */
    if (role == 0 && !error) {
        printf("[rodič] Spouštím potomka 1\n");
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            error = 1;
        } else if (pid == 0) {
            role = 1;
        } else {
            children++;
        }
    }

    /* Potomka 2 vytvari pouze puvodni rodic. */
    if (role == 0 && !error) {
        printf("[rodič] Spouštím potomka 2\n");
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            error = 1;
        } else if (pid == 0) {
            role = 2;
        } else {
            children++;
        }
    }

    /* Potomka 3 vytvari pouze puvodni rodic. */
    if (role == 0 && !error) {
        printf("[rodič] Spouštím potomka 3\n");
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            error = 1;
        } else if (pid == 0) {
            role = 3;
        } else {
            children++;
        }
    }

    int input;
    if (role == 0) {
        input = file; /* Rodic cte ze souboru. */
    } else {
        int previous_pipe = role - 1;
        input = pipes[previous_pipe][0]; /* Potomek cte predchozi rouru. */
    }

    int output;
    if (role < 3) {
        output = pipes[role][1]; /* Rodic a potomci 1, 2 posilaji dal. */
    } else {
        output = -1; /* Potomek 3 uz nema vystupni rouru. */
    }
    for (int i = 0; i < 3; i++) {
        if (pipes[i][0] != input) {
            close(pipes[i][0]);
        }
        if (pipes[i][1] != output) {
            close(pipes[i][1]);
        }
    }
    if (role != 0) {
        close(file);
    }

    /* Jen rodic cte potvrzeni, jen potomek 3 je posila. */
    if (role != 0) {
        close(ack[0]);
    }
    if (role != 3) {
        close(ack[1]);
    }

    char line[512];
    char result[1024]; /* Rezerva pro cislo a delku jmena. */
    size_t number = 0;
    int received = 0;
    /* Jmeno ma nejvyse 255 bajtu; potomci maji prostor i pro pridany text. */
    int capacity = 512;
    if (role == 0) {
        capacity = 256;
    }
    while (!error) {
        received = read_line(input, line, capacity);
        if (received != 1) {
            break;
        }
        if (role == 0) {
            printf("[rodič] Čtu ze souboru: \"%s\"\n", line);
        } else {
            printf("[potomek%d] Přijal: \"%s\"\n", role, line);
        }

        if (role == 1) {
            for (int i = 0; line[i]; i++) {
                line[i] = (char)tolower((unsigned char)line[i]);
            }
            snprintf(result, sizeof(result), "%zu. %s", ++number, line);
        } else if (role == 2) {
            char *name = strstr(line, ". ") + 2; /* Preskocime cislo a '. '. */
            snprintf(result, sizeof(result), "%s (%zu)", line, strlen(name));
        } else if (role == 3) {
            for (int i = 0; line[i]; i++) {
                if (i == 0 || isspace((unsigned char)line[i - 1])) {
                    line[i] = (char)toupper((unsigned char)line[i]);
                }
            }
            printf("[potomek3] Výstup: \"%s\"\n", line);
            /* Vystup uz je vypsany: rodic muze precist dalsi jmeno. */
            if (write(ack[1], "x", 1) != 1) {
                perror("write ack");
                error = 1;
            }
            continue; /* Potvrzeni jde zpet, jmeno uz neposilame dal. */
        } else {
            strcpy(result, line); /* Rodic posila puvodni jmeno. */
        }

        if (role != 0) {
            printf("[potomek%d] Posílám: \"%s\"\n", role, result);
        }
        if (write_text(output, result) == -1 || write_text(output, "\n") == -1) {
            perror("write");
            error = 1;
        }
        if (role == 0 && !error) {
            char done;
            /* read() ceka, dokud potomek 3 neposle jeden bajt. */
            ssize_t count = read(ack[0], &done, 1);
            if (count == -1) {
                perror("read ack");
                error = 1;
            } else if (count == 0) {
                fprintf(stderr, "Potomek 3 skoncil bez potvrzeni\n");
                error = 1;
            }
        }
    }
    if (received == -1) {
        error = 1;
    }
    close(input);
    if (output != -1) {
        close(output); /* Dalsi proces po docteni dat dostane EOF. */
    }

    if (role == 0) {
        close(ack[0]);
    }
    if (role == 3) {
        close(ack[1]);
    }

    if (role == 0) {
        for (int i = 0; i < children; i++) {
            int status;
            if (wait(&status) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                error = 1;
            }
        }
    }
    return error;
}
