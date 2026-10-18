#define _POSIX_C_SOURCE 200809L
#include "common.h"
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int verbose = argc > 1 && !strcmp(argv[1], "-v");
    int first = 1;
    if (verbose) {
        first = 2;
    }
    if (argc != first + 3) {
        fprintf(stderr, "Pouziti: %s [-v] N even|odd square|sqrt|neg\n", argv[0]);
        return 1;
    }

    /* Hodnoty argumentu kontroluji samotne pomocne programy. */
    char *gen_args[] = {"./gen", argv[first], NULL};
    char *filter_args[] = {"./filter", argv[first + 1], NULL};
    char *transform_args[] = {"./transform", argv[first + 2], NULL};
    char *sum_args[] = {"./sum", NULL};

    /* [0] = cteni, [1] = zapis. Vsechny roury vytvorime pred fork(). */
    int gen_to_filter[2];
    int filter_to_transform[2];
    int transform_to_sum[2];
    pipe(gen_to_filter);
    pipe(filter_to_transform);
    pipe(transform_to_sum);

    /* gen: stdout -> prvni roura. */
    if (verbose) {
        fprintf(stderr, "[calcpipe] Spoustim ./gen %s\n", gen_args[1]);
    }
    pid_t gen_pid = fork();
    if (gen_pid == 0) {
        dup2(gen_to_filter[1], STDOUT_FILENO);

        close(gen_to_filter[0]);
        close(gen_to_filter[1]);
        close(filter_to_transform[0]);
        close(filter_to_transform[1]);
        close(transform_to_sum[0]);
        close(transform_to_sum[1]);
        execvp(gen_args[0], gen_args);
        perror("./gen");
        _exit(127);
    }
    close(gen_to_filter[1]); // Dalsi potomci tento konec uz nededi.

    /* filter: prvni roura -> stdin, stdout -> druha roura. */
    if (verbose) {
        fprintf(stderr, "[calcpipe] Spoustim ./filter %s\n", filter_args[1]);
    }
    pid_t filter_pid = fork();
    if (filter_pid == 0) {
        dup2(gen_to_filter[0], STDIN_FILENO);
        dup2(filter_to_transform[1], STDOUT_FILENO);
        close(gen_to_filter[0]);
        close(filter_to_transform[0]);
        close(filter_to_transform[1]);
        close(transform_to_sum[0]);
        close(transform_to_sum[1]);
        execvp(filter_args[0], filter_args);
        perror("./filter");
        _exit(127);
    }
    close(gen_to_filter[0]);
    close(filter_to_transform[1]);

    /* transform: druha roura -> stdin, stdout -> treti roura. */
    if (verbose) {
        fprintf(stderr, "[calcpipe] Spoustim ./transform %s\n", transform_args[1]);
    }
    pid_t transform_pid = fork();
    if (transform_pid == 0) {
        dup2(filter_to_transform[0], STDIN_FILENO);
        dup2(transform_to_sum[1], STDOUT_FILENO);
        close(filter_to_transform[0]);
        close(transform_to_sum[0]);
        close(transform_to_sum[1]);
        execvp(transform_args[0], transform_args);
        perror("./transform");
        _exit(127);
    }
    close(filter_to_transform[0]);
    close(transform_to_sum[1]);

    /* sum: treti roura -> stdin, stdout zustava v terminalu. */
    if (verbose) {
        fprintf(stderr, "[calcpipe] Spoustim ./sum\n");
    }
    pid_t sum_pid = fork();
    if (sum_pid == 0) {
        dup2(transform_to_sum[0], STDIN_FILENO);
        close(transform_to_sum[0]);
        execvp(sum_args[0], sum_args);
        perror("./sum");
        _exit(127);
    }
    close(transform_to_sum[0]);

    /* Vsechny procesy uz bezi; teprve ted na ne cekame. */
    waitpid(gen_pid, NULL, 0);
    waitpid(filter_pid, NULL, 0);
    waitpid(transform_pid, NULL, 0);
    waitpid(sum_pid, NULL, 0);
    return 0;
}
