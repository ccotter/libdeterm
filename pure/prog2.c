
#include <inc/mman.h>
#include <inc/determinism_pure.h>
#include <inc/fs.h>
#include <inc/assert.h>
#include <inc/io.h>
#include <inc/fork.h>
#include <inc/std.h>

/* Ensure buffer is NULL terminated. len is length of buffer array. */
static int readline(char *buffer, int len)
{
    int i = 0, ch;
    while (EOF != (ch = getchar()) && '\n' != (char)ch)
    {
        buffer[i] = (char)ch;
        ++i;
        if (i == len - 1)
            break;
    }
    buffer[i] = 0;
    return i;

}

const char *skipword(const char *itr) /* precondition: *itr != 0 */
{
    if (!(*itr))
        return NULL;
    while (*itr && (' ' != *itr))
        ++itr;
    return itr;
}
const char *skipspaces(const char *itr)
{
    while (*itr && (' ' == *itr))
        ++itr;
    if (!*itr)
        return NULL;
    return itr;
}

/* Can only launch a program - no args, redirection...nothing useful really. */
/* Support up to 10 args. */
char *ARGS[11]; /* NULL terminated array. */
static void runcmd(int childid, const char *cmd)
{
    const char *orig=cmd;
    int i = 0;
    const char *at = cmd = skipspaces(cmd);
    if (!cmd)
        _exit(1);
    while ((at = skipword(cmd)))
    {
        ARGS[i] = (char*)malloc(sizeof(char) * (at - cmd + 1));
        strncpy(ARGS[i], cmd, at - cmd);
        ++i;
        if (!(cmd = skipspaces(at)))
            break;
    }
    ARGS[i] = NULL;
    char *ENVS[] = {NULL};
    dexec(childid, ARGS[0], ARGS, ENVS);
    printf("Command (%s) failed to execute\n", orig);
    /* If we get here, then dexec failed (invalid command most likely). */
    _exit(1);
}

/* A most minimal shell (you'll wonder if it is possible to have *fewer* features). */
int main(void)
{
    char linebuf[512];
    dret();
    int i=0;
    while(1)
    {
        int id;
        int child = i++;
        printf("> ");
        id = readline(linebuf, sizeof(linebuf));
        id = dfork(child, DETERMINE_START);
        if (id < 0)
        {
            printf("dfork error: %d\n", id);
            return 1;
        }
        if (!id)
        {
            runcmd(child, linebuf);
        }
        else
        {
            dwaitpid(child);
        }
    }

    return 0;
}

