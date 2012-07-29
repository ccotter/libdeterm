
#ifndef _INC_FS_EXT_H_
#define _INC_FS_EXT_H_

/* This function reads a file from an actual physical disk (more specifically,
   the actual computer's file system) into a file in DFS . */
int dfs_read_into_dfs(const char *actual, const char *dfs_file);

#endif

