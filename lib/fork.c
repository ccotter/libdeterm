
#include <inc/determinism_pure.h>
#include <inc/fork.h>
#include <inc/fs.h>

pid_t dfork(pid_t id, int flags)
{
    return dput(id, DETERMINE_REGS | flags, NULL, 0, NULL);
}

/* Wait for a child process to finish (mark itself as done executing).
   Also, kill the child before we return (otherwise the child's resources
   will not be freed back to the OS). */
int dwaitpid(pid_t childid)
{
    int child_state;
    while (1)
    {
        int out_fd;
        child_state = dfs_childstate(childid);
        if (child_state < 0)
            return -1;
        if (0 != dfs_get(childid))
            return -1;
        io_sync(0);
        if ((DPROC_EXITED | DPROC_FAULTED) & child_state)
            break;
        if (DPROC_INPUT & child_state)
            io_sync(1);
        if (0 != dfs_put(childid))
            return -1;
        if (DETERMINE_S_RUNNING != dput(childid, DETERMINE_START, NULL, 0, NULL))
            return -1;
    }
    dput(childid, DETERMINE_CHILD_STATUS | DETERMINE_KILL, NULL, 0, NULL);
    return child_state;
}

/* Wait for a child to request a sync point - i.e. the child calls dret(). */
int dsyncchild(pid_t childid)
{
    return dget(childid, 0, NULL, 0, NULL);
}

int dkill(pid_t childid)
{
    int rc = dput(childid, DETERMINE_CHILD_STATUS | DETERMINE_KILL, NULL, 0, NULL);
    return 1 != rc; /* Return 0 if the child was killed successfully. */
}

