
#include <inc/determinism.h>
#include <inc/syscall.h>

int iprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    return 0;
}

static char BUF[1024];
static const char *FLAGS[33] = {"START", "REGS", "COPY", "ZERO",
    "SNAP|MERGE", "BECOME", "COPY|CLEAR", "STATUS"};
static char *flags2str(int flags)
{
    BUF[0] = 0;
    int i = 1, j = 0, k = 0;
    while (j<33)
    {
        if (flags & i)
        {
            if (0 != k)
            {
                BUF[k++] = ' ';
                BUF[k++] = '|';
                BUF[k++] = ' ';
            }
            memmove(BUF+k, FLAGS[j], strlen(FLAGS[j])+1);
            k += strlen(FLAGS[j]);
        }
        i *= 2;
        ++j;
    }
    return BUF;
}

// System call declarations.
long dput(pid_t child, int flags, void *start, size_t size, void *dst)
{
    long ret = syscall(SYS_dput,
            child,
            flags,
            (unsigned long)start, 
            size,
            (unsigned long)dst);
    if (ret < 0)
        fprintf(stderr, "dput error: %d (%s, %08lx, %08lx, %08lx)\n",
                (int)ret, flags2str(flags), (unsigned long)start,
                (unsigned long)size, (unsigned long)dst);
    return ret;
}

long dget(pid_t child, int flags, void *start, size_t size, void *dst)
{
    long ret = syscall(SYS_dget, child, flags, (unsigned long)start, size,
            (unsigned long)dst);
    if (ret < 0)
        fprintf(stderr, "dput error: %d (%s, %08lx, %08lx, %08lx)\n",
                (int)ret, flags2str(flags), (unsigned long)start,
                (unsigned long)size, (unsigned long)dst);
    return ret;
}

long dret(void)
{
    return syscall(SYS_dret);
}

int become_deterministic(void)
{
    return dput(0, DETERMINE_BECOME_MASTER, NULL, 0, NULL);
}

