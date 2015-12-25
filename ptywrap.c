/*
 * Wrap execution of a program inside a pty, copying the pty line data to
 * stdout.
 *
 * ptyer is useful as a general utility, but doesn't help on its own when
 * running a daemon via an auto-logging supervisor. This is because the
 * service scanner creates a pipe and then two supervisors (one for the
 * service itself and another for the logger) which are connected via that
 * pipe. By the time ptyer would be run to create the pty, the pty's FD
 * cannot be used to pass to the logger. One would need to launch a data
 * copier like 'cat' or 'dd' to connect the pty to the pipe to the logger,
 * which would make ps listings confusing to some people.
 *
 * This simplifies the task, and results in a (hopefully) easily understood
 * name in ps listings (especially when viewed in a tree/forest view).
 *
 * This utility has the additional functionality of emulating a standard
 * 80x24 terminal size (TODO). This allows programs to query the size
 * either via standard TTY requests *or* by sending one of the ANSI
 * sequences. While probably rarely useful for daemons, it would be useful
 * for running pagers or editors (like busybox vi) from a shared serial
 * port (where having multiple clients respond to the size request has
 * horrifying results).
 *
 * Caveats: The program input will still be from the original stream. Some
 * programs, like stty, use the inpput stream instead of the output stream
 * for the terminal configuration.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

static int
child(const char *name, int argc, char **argv);


int main(int argc, char **argv)
{
    /* TODO: Check for errors */
    char name[50] = "";
    int mfd; /* master terminal fd */
    pid_t client;
    mfd = posix_openpt(O_NOCTTY | O_RDWR);
    ptsname_r(mfd, name, sizeof(name));
    printf("Got fd %i, referencing '%s'\n", mfd, name);
    grantpt(mfd);
    unlockpt(mfd);
    client = fork();
    if (client == -1) {
        fprintf(stderr, "Failed to fork\n");
        return 1;
    }
    if (client == 0) {
        // child
        return child(name, argc, argv);
    }
    /* TODO: copy data from pty to stdout in some sort of optimally
     * buffered format.
     * TODO: monitor child for exit/signals
     * TODO: intercept some ANSI control sequences
     */
    char buf[50];
    int count;
    while (1) {
        count = read(mfd, buf, 50);
        if (count > 0) {
            write(1, buf, count);
        } else if (count == 0) {
            printf("done\n");
            break;
        } else {
            perror("whoops");
            break;
        }
    }
    waitpid(client, NULL, 0);
    return 0;
}

int child(const char *name, int argc, char **argv)
{
    int sfd; /* slave terminal fd */
    int olderr = dup2(2, 254);
    if (olderr != 254) {
        fprintf(stderr, "failed to dup\n");
        return 1;
    }
    FILE *myerror = fdopen(254, "w");
    if (!myerror) {
        fprintf(stderr, "failed to open myerror\n");
        return 1;
    }
    fclose(stdout);
    fclose(stderr);
    close(1);
    close(2);
    sfd = open(name, O_RDWR | O_NOCTTY);
    printf("SFD: %i\n", sfd);
    if (sfd != 1) {
        fprintf(myerror, "wrong fd for 1\n");
        return 1;
    }
    int e = dup2(1,2);
    if (e != 2) {
        fprintf(myerror, "wrong fd for 2\n");
        return 1;
    }
    execvp(argv[1], &argv[1]);
    fprintf(myerror, "exec failed\n");
    /* If the exec failed, return an error */
    /* TODO: execline scemantics for return codes */
    return 1;
}
