
#include <unistd.h>
#include <inc/fork_nondet.h>
#include <inc/fs_nondet.h>

pid_t dfork(pid_t id, int flags)
{
    return dput(id, DETERMINE_REGS | flags, NULL, 0, NULL);
}

/* Wait for a child process to finish (mark itself as done executing).
   Also, kill the child before we return (otherwise the child's resources
   will not be freed back to the OS). */
int dwaitpid(pid_t childid)
{
    int out_fd, nread;
    int child_state;
    char buf[512];
    off_t out_at = 0;
    while (1)
    {
        child_state = dfs_childstate(childid);
        if (child_state < 0)
            return -1;
        if (0 != dfs_get(childid))
            return -1;
        /* Forward __stdout to the console. */
        out_fd = dfs_open("/__stdout", DFS_O_RDWR);
        if (out_fd < 0)
            return -1;
        if (out_at != dfs_seek(out_fd, out_at, DSEEK_SET))
            return -1;
        while (0 < (nread = dfs_read(out_fd, buf, sizeof(buf))))
        {
            write(1, buf, nread);
        }
        out_at = dfs_tell(out_fd);
        dfs_close(1);
        if (1 != dfs_open("/__stdout", DFS_O_APPEND))
            return -1;
        if (0 != dfs_close(out_fd))
            return -1;
        if ((DPROC_EXITED | DPROC_FAULTED) & child_state)
            break;
        if (DPROC_INPUT & child_state)
        {
            /* Pass input to the child's stdin. */
            int n = read(0, buf, sizeof(buf));
            int fd_stdin = dfs_open("/__stdin", DFS_O_APPEND);
            dfs_write(fd_stdin, buf, n);
            dfs_close(fd_stdin);
        }
        dfs_put(childid);
        dput(childid, DETERMINE_START, NULL, 0, NULL);
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

