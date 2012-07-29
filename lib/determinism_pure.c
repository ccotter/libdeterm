
#include <inc/determinism_pure.h>
#include <inc/syscall.h>

#define UL(x) ((unsigned long)x)
// System call declarations.
long dput(pid_t child, int flags, void *start, size_t size, void *dst)
{
    long ret = syscall5(SYS_dput,
            UL(child),
            UL(flags),
            UL(start), 
            UL(size),
            UL(dst));
    return ret;
}

long dget(pid_t child, int flags, void *start, size_t size, void *dst)
{
    long ret = syscall5(SYS_dget,
            UL(child),
            UL(flags),
            UL(start),
            UL(size),
            UL(dst));
    return ret;
}
#undef UL

long dret(void)
{
    return syscall0(SYS_dret);
}

int become_deterministic(void)
{
    return dput(0, DETERMINE_BECOME_MASTER, NULL, 0, NULL);
}

