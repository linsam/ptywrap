#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

/*
 * Takes a block and executes it "in the background", and execs into the
 * rest of the line.
 *
 * The standard in is connected to the stdin of the block. The terminal
 * output is connected to the input of the rest of the line.
 *
 *
 * On second thought; that's too complex; execline already offers tools
 * that do all of that except the actual allocation of the PTYs.
 *
 * What we really want is a tool that creates the PTYs on a couple new FDs
 * (probably given on the command line; we'll dup2 them or something), then
 * exec. The fdmove and background commands can be used to put things where
 * they are wanted. This results in simpler code (for me) and a more
 * generally useful tool.
 */

int main(int argc, char **argv)
{
    /* TODO: Check for errors */
    /* TODO: Read argv for the FD numbers to use */
    char name[50] = "";
    int mfd; /* master terminal fd */
    int sfd; /* slave terminal fd */
    mfd = posix_openpt(O_NOCTTY | O_RDWR);
    ptsname_r(mfd, name, sizeof(name));
    printf("Got fd %i, referencing '%s'\n", mfd, name);
    grantpt(mfd);
    unlockpt(mfd);
    sfd = open(name, O_RDWR | O_NOCTTY);
    printf("SFD: %i\n", sfd);
    execvp(argv[1], &argv[1]);
    /* If the exec failed, return an error */
    /* TODO: execline scemantics for return codes */
    return 1;
}
