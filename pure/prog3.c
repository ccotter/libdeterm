
#include <inc/determinism_pure.h>
#include <inc/fs.h>
#include <inc/assert.h>
#include <inc/io.h>

/* Dumb program that stores tuples (name, occupation). */

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

const char *nextcomma(const char *itr)
{
    while (*itr && (',' != *itr))
        ++itr;
    if (!(*itr))
        return NULL;
    return itr;
}

char db_buffer[0x10000];
static void readdb(void)
{
    int fd, rd, counter = 0;
    const char *at, *next;
    fd = dfs_open("/database", DFS_O_RDONLY);
    if (fd < 0)
        return;
    if (0 >= (rd = dfs_read(fd, db_buffer, sizeof(db_buffer))))
    {
        dfs_close(fd);
        return;
    }
    db_buffer[rd] = 0;
    at = db_buffer;
    next = db_buffer - 1;
    while((next = nextcomma(next + 1)))
    {
        if (0 == counter % 2)
            printf("%.*s: ", next - at, at);
        else
            printf("%.*s\n", next - at, at);
        at = next + 1;
        ++counter;
    }
    dfs_close(fd);
}

static int add(const char *name, const char *occ)
{
    int fd, rc, sz;
    fd = dfs_open("/database", DFS_O_CREAT | DFS_O_APPEND);
    if (fd < 0)
        return fd;
    dfs_write(fd, name, strlen(name));
    dfs_write(fd, ",", 1);
    dfs_write(fd, occ, strlen(occ));
    dfs_write(fd, ",", 1);
    dfs_close(fd);
    return 0;
}

int main(int argc, char **argv)
{
    if (2 == argc && 0 == strcmp(argv[1], "list"))
    {
        printf("Here are the contents:\n");
        readdb();
    }
    else
    {
        char name[128], occ[128];
        int rc;
        printf("Enter your name: ");
        readline(name, sizeof(name));
        printf("Enter your occupation: ");
        readline(occ, sizeof(occ));
        if (add(name, occ))
            printf("Error saving to the database!\n");
        else
            printf("Your information (%s, %s) has been saved.\n", name, occ);
    }
    return 0;
}

