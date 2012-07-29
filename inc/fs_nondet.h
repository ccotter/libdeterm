
#ifndef _INC_FS_H_
#define _INC_FS_H_

#include <stdint.h>

/*
   User include file that defines the various file system functions. The
   deterministic file system (occasionally referred to as DFS).
 */

#define E_INIT              1
#define E_NOINODES          2
#define E_NOSPACE           3
#define E_NOEXIST           4
#define E_MAXFILES          5
#define E_INVAL             6
#define E_PERM              7
#define E_ALREADY_OPENED    8
#define E_NOT_EMPTY         9
#define E_NOT_DIR           10
#define E_EOF               11
#define E_COPY_ERROR        12
#define E_OOM               13
#define E_FILES_OPEN        14
#define E_CONFLICT          15

#define DFS_O_RDONLY    0x0001
#define DFS_O_RDWR      0x0002
#define DFS_O_WRONLY    0x0004
#define DFS_O_APPEND    0x0008
#define DFS_O_CREAT     0x0010

typedef unsigned long blockno_t;
typedef unsigned long ino_t;

#define MAX_FNAME_LENGTH 120
#define MAX_PATH_LENGTH 4095

#define FS_DIR_TYPE 1
#define FS_FILE_TYPE 2

#define DPROC_READY         0x0001
#define DPROC_FAULTED       0x0002
#define DPROC_EXITED        0x0004
#define DPROC_INPUT         0x0008

#define DSEEK_SET 1
#define DSEEK_CUR 2
#define DSEEK_END 3

struct direntry
{
    ino_t d_ino;
    uint32_t d_type;
    char d_name[MAX_FNAME_LENGTH];
};

/* Initializes the file system to a clean state. Since the file system
   exists entirely in memory, initializing the file system to a clean
   slate allows the process to then populate the file system. The file
   system has only one file after initialization, the root directory "/".
 */
int dfs_init_clean(void);

/* Opens up a file. On success, open returns a nonnegative integer which acts
   like a Unix style file descriptor. On failure, returns a negative integer
   representing the possible errors. 
   This function is used only for opening regular files; use dfs_opendir
   for opening directories. */
int dfs_open(const char *path, int flags);

/* Writes at most nbytes to the file. Returns the number of bytes actually written
   or returns a negative integer indicating the error. */
int dfs_write(int fd, const void *buf, size_t nbytes);

/* Read at most nbytes from the file. Returns the number of bytes read or a negative
   integer indicating the error.*/
int dfs_read(int fd, void *buf, size_t nbytes);

/* Closes a file descriptor. */
int dfs_close(int fd);
void dfs_close_all(void);

/* Moves internal buffer pointer to desired position. If the position is out of
   bounds, the position is set to the nearest offset (beginning or end).
   Returns (on success) the new position in bytes, otherwise an error code. */
int dfs_seek(int fd, long offset, int whence);

/* Returns the current value of the file position offset for the given file descriptor. */
long dfs_tell(int fd);

/* Sets the file to the desired size. Of the original size is larger than the
   truncated size, those bytes are lost (deleted). If the original size is smaller
   than the truncated size, then the file is extended and filled with zero bytes. */
int dfs_truncate(int fd, off_t len);

/* Deletes a file link. The file itself might be refered to by different
   names, so the physical file will be deleted once all links to the file
   are removed. The physical file being deleted can not be simultaneously
   opened by the process. */
int dfs_unlink(const char *path);

/* All parent and grandparent (etc) directories must exist before creating
   the directory. Returns 0 on success, < 0 error code otherwise. */
int dfs_mkdir(const char *path);

/* Opens a directory for reading. Returns a special file descriptor that
   can only be used for directory operations. */
int dfs_opendir(const char *path);

/* Copies the next direntry into the pointer argument. */
int dfs_readdir(int fd, struct direntry *dentry);

/* Resets the directory stream. */
int dfs_rewinddir(int fd);

/* Closes a directory file descriptor. */
int dfs_closedir(int fd);

/* Copies this process's file system (open file descriptor state and file contents)
   into the specified deterministic child). */
int dfs_put(pid_t fd);

/* A child process calls this to prepare internal data structures for a merge
   of its file system changes into its parent process. */
int dfs_prepare_for_get(void);

/* A parent calls this function to merge a child's file system changes. Before
   this function is called, the child must have call dfs_prepare_for_get and
   not made any successive FS chanegs.
 
   All files in the parent must be closed prior to calling this function (dfs_get
   will not attempt to merge otherwise.) */
int dfs_get(pid_t childid);

int _init_io(void);

/* User level process state. This mechanism allows processes to communicate to parents the
   runnable state of the process. */
void dfs_setstate(int state);
int dfs_childstate(pid_t childid);

#endif

