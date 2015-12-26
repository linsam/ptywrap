/* Test program for ptywrap.
 *
 * Usage:
 *
 *   ./test | cat
 *   ./ptywrap ./test | cat
 *
 * Observe that without ptywrap, all of the output comes through at the end
 * of a 10 second delay, but with ptywrap, the output comes one line once
 * per second.
 *
 * This is because, in the first case, test's stdout is not a terminal, but
 * is attached to a pipe. Libc thus follows its default behavior of opening
 * stdout in fully buffered mode instead of line buffered.
 *
 * In the second case, ptywrap connects test's output to a pty and
 * redirects the pty output to the pipe. Because test's stdout IS a
 * terminal this time, libc sets stdout to line buffered mode.
 *
 * Note that in both cases, test is part of the same process group (bash
 * job) as ptywrap and cat, so things like ctrl+z, ctrl+c, etc. work as
 * expected. As such, they all are part of the session on the originating
 * tty, and no process is controlled by the pty made by ptywrap (nothing
 * sets it as controlling terminal, and there is no session leader there).
 */


#include <stdio.h>
#include <unistd.h>

int main()
{
    int i;
    for (i = 0; i < 10; i++) {
        printf("Out %i\n", i);
        sleep(1);
    }
    return 0;
}
