
#include <inc/fs.h>
#include <inc/fs_ext.h>
#include <fcntl.h>

static uint8_t read_buffer[FS_BLOCK_SIZE];
int dfs_read_into_dfs(const char *actual, const char *dfs_file)
{
    int fd, dfs_fd;
    off_t pos;

    init_dfs();

    /* Read in the file sizeof(read_buffer) bytes at a time. */

    return 0;
}

