
#include <inc/std.h>
#include <inc/syscall.h>
#include <inc/fs.h>
#include <inc/io.h>

/* Declare initialization functions we know about (they are internal
   and therefore not readily available in header files). */
int init_malloc(void);
int _init_dfs(void);
int _init_io(void);

/* Initialize various library components. This is called from _dstart
   before the actual main is called. */
void _main_setup(int argc, char **argv, char **env)
{
    init_malloc();
    //_init_dfs();
    //_init_io();
}

/* Call when main() returns or a program exits (_exit calls _exit_finalize). This
   functions does some house keeping before returning control to the parent. */
void _exit_finalize(void)
{
    flush_printf_buffer();
    dfs_close_all();
}

/* On no account should a process try to actually kill itself - instead we mark ourselves
   as exited in user memory. A process's parent will kill it when it no longer needs the child. */
void _exit(int status)
{
    _exit_finalize();
    dfs_setstate(DPROC_EXITED);
    dret();
}

