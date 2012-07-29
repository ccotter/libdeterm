
#include <inc/assert.h>
#include <inc/fs.h>
#include <inc/string.h>

uint8_t data[0x10000];
uint8_t buffer[0x10000];

char str1[] = "This is string 1.";
char str2[] = "This is string 2.";
char str3[] = "This is string 3.";

char tabs[] = "\t\t\t\t\t\t\t";

void printdir(const char *path, int depth)
{
    struct direntry ent;
    char dir_buffer[1024];
    int fd = dfs_opendir(path);
    assert(fd >= 0);
    while (!dfs_readdir(fd, &ent))
    {
        iprintf("%.*s%s\n", depth, tabs, ent.d_name);
        if (FS_DIR_TYPE == ent.d_type && 0 != strcmp(ent.d_name, ".."))
        {
            strcpy(dir_buffer, path);
            dir_buffer[strlen(path)] = '/';
            strcpy(dir_buffer + strlen(path) + 1, ent.d_name);
            dir_buffer[strlen(path) + strlen(ent.d_name) + 1] = 0;
            //snprintf(dir_buffer, sizeof(dir_buffer), "%s/%s", path, ent.d_name);
            printdir(dir_buffer, depth+1);
        }
    }
    dfs_closedir(fd);
}

int main(void)
{
    int i;
    assert(0 == dfs_init_clean());
    assert(0 == dfs_mkdir("/home"));
    assert(0 == dfs_mkdir("/home/chris"));
    assert(0 == dfs_mkdir("/home/guest"));
    assert(0 == dfs_mkdir("/usr"));
    assert(0 == dfs_mkdir("/usr/../../././././././../../usr/bin"));
    assert(0 == dfs_mkdir("/usr/include"));
    int fd;
    char buf[100];
    fd = dfs_open("/home/../../../../home/chris/./././f1", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    assert(strlen(str1) == dfs_write(fd, str1, strlen(str1)));
    dfs_close(fd);
    fd = dfs_open("/home/chris/f1", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    dfs_read(fd, buf, strlen(str1));
    assert(0 == strcmp(buf, str1));
    dfs_close(fd);

    fd = dfs_open("/home/chris/f2", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    assert(strlen(str2) == dfs_write(fd, str2, strlen(str2)));
    dfs_close(fd);
    fd = dfs_open("/home/chris/f2", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    dfs_read(fd, buf, strlen(str2));
    assert(0 == strcmp(buf, str2));
    dfs_close(fd);

    fd = dfs_open("/home/chris/f3", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    assert(strlen(str3) == dfs_write(fd, str3, strlen(str3)));
    dfs_close(fd);
    fd = dfs_open("/home/chris/f3", DFS_O_CREAT | DFS_O_RDWR);
    assert(fd >= 0);
    dfs_read(fd, buf, strlen(str3));
    assert(0 == strcmp(buf, str3));
    dfs_close(fd);

    // Print entire FS layout.
    printdir("/", 0);

    /* Delete some files. */
    assert(0 == dfs_unlink("/home/chris/f1"));
    assert(-E_NOEXIST == dfs_unlink("/home/chris/f1"));
    assert(-E_NOT_EMPTY == dfs_unlink("/home/chris//"));
    assert(0 == dfs_unlink("/home/chris/f2"));
    assert(-E_NOEXIST == dfs_unlink("/home/chris/f2"));
    assert(-E_NOT_EMPTY == dfs_unlink("/home/chris//////"));
    assert(0 == dfs_unlink("/home/chris/f3"));
    assert(-E_NOEXIST == dfs_unlink("/home/chris/f3"));
    assert(0 == dfs_unlink("/home/chris//////"));

    iprintf("Done\n");
    return 0;
}

