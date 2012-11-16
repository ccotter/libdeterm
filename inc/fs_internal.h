
#ifndef _INC_FS_INTERNAL_H_
#define _INC_FS_INTERNAL_H_

#include <sys/types.h>
#include <fs.h>
#include <assert.h>

typedef unsigned long blockno_t;

/* Deterministic file system (dfs for short) implemented in user space.
   This is essentially a replica of unix style inode file system. */

#define PAGE_SIZE 0x1000

/* Pick the size of a page since we can easily optomize using copy-on-write. */
#define FS_BLOCK_SIZE (PAGE_SIZE*1024)

/* This determines the size in bytes of our file system (FS_BLOCK_SIZE * FS_NBLOCKS). */
#define FS_NBLOCKS (0x10000/1024 - 6)

/* FS_BLOCK_SIZE PAGE_SIZE
   FS_NBLOCKS (0x10000 - 100)
   */

/* Total number of bytes our file system can store. */
#define FS_SIZE (FS_NBLOCKS * FS_BLOCK_SIZE)

/* A block can determine the in-use bit of FS_BLKBITSIZE blocks. */
#define FS_BLKBITSIZE (FS_BLOCK_SIZE * 8)

/* How many blocks do we need for the bitmap? */
#define FS_BITMAP_BLOCKS ((FS_NBLOCKS + FS_BLKBITSIZE - 1) / FS_BLKBITSIZE)

/* The inodes contain a fixed number of direct, indirect and doubly indirect blocks. */
#define FS_NDIRECT 10
/* Number of direct pointers in an indirect block. */
#define FS_NSINGLE_INDIRECT (FS_BLOCK_SIZE / sizeof(unsigned long))
/* Number of indirect pointers in a doubly indirect block. */
#define FS_NDOUBLE_INDIRECT (FS_BLOCK_SIZE / sizeof(unsigned long))

/* We will pad the inode by the required amount to enforce this. */
#define FS_INODE_SIZE 128

struct inode
{
    /* Size in bytes of this file. */
    off_t f_size;

    /* Number of times this inode is referenced by a directory file. */
    uint32_t f_refcount;

    /* Type of file (either directory or "normal" file). */
    uint32_t f_type;

    /* Direct blocks. A direct block is allocated if and only if the value is != 0. */
    blockno_t f_direct[FS_NDIRECT];

    /* Pointer to the indirect block. */
    blockno_t f_indirect;

    /* Pointer to the doubly indirect block. */
    blockno_t f_double_indirect;

    /* We use versioning to manage the distributed nature of the file system. 
       Version 0 indicates the file is not in use. Version numbers start at 1. */
    uint32_t f_version;
    uint32_t f_rversion;

    /* Keep track of how this inode was changed. */
    uint32_t f_flags;

    /* For files that have append only changes, keep track of the original file offset
       to know where to begin copying on a file system merge. */
    off_t f_append_off;

    /* Pad out to FS_INODE_SIZE. */
    uint8_t f_pad[FS_INODE_SIZE - sizeof(uint32_t) * 5 -
		sizeof(blockno_t) * (2 + FS_NDIRECT) - 2 * sizeof(off_t)];

} __attribute__((packed));

_Static_assert(sizeof(struct inode) == FS_INODE_SIZE,
		"struct inode size incorrect");

/* Flags that determine how an inode was changed. */
#define INODE_DELETED       0x0001
#define INODE_APPENDED      0x0002

#define FS_MAGIC 0xdeadbeef

struct super_block
{
    unsigned long s_magic; /* Helps ensure validity of our file system. */
    unsigned long s_nblocks; /* How large our file system is. */
    struct inode *s_inode; /* Points to first inode. */
    ino_t s_root; /* inode number of the root file inode (i.e. "/"). */
};

#define FS_VM_START 0x1000000000
#define FS_VM_END (FS_VM_START + FS_SIZE)
#define FS_SCRATCH1 FS_VM_END
#define FS_SCRATCH1_SIZE ((6 - FS_BITMAP_BLOCKS - 4) * FS_BLOCK_SIZE)
#define FS_SCRATCH2 (FS_SCRATCH1 + FS_SCRATCH1_SIZE)
#define FS_SCRATCH2_SIZE (FS_BLOCK_SIZE * FS_BITMAP_BLOCKS)
#define FS_SCRATCH3 (FS_SCRATCH2 + FS_SCRATCH2_SIZE)
#define FS_SCRATCH3_SIZE FS_BLOCK_SIZE
#define FS_SCRATCH4 (FS_SCRATCH3 + FS_SCRATCH3_SIZE)
#define FS_SCRATCH4_SIZE FS_BLOCK_SIZE
#define FS_SCRATCH5 (FS_SCRATCH4 + FS_SCRATCH4_SIZE)
#define FS_SCRATCH5_SIZE FS_BLOCK_SIZE
#define FS_STATE (FS_SCRATCH5 + FS_SCRATCH4_SIZE)
#define FS_STATE_SIZE FS_BLOCK_SIZE
/* Must be a multiple of FS_BLOCK_SIZE / FS_INODE_SIZE */
#define FS_NINODES (0x8000)

_Static_assert(0 == FS_NINODES % (FS_BLOCK_SIZE / FS_INODE_SIZE),
		"NINODES not multiple of BLOCK_SIZE/INODE_SIZE");
_Static_assert(0x1010000000 == FS_STATE + FS_STATE_SIZE,
		"FS memory location incorrect");

#define NMAX_OPEN_FILES 10
#define FS_INTERNAL_FD NMAX_OPEN_FILES

#define NDIRENTRIES_PER_BLOCK (FS_BLOCK_SIZE / sizeof(struct direntry))

/* These structures define how our file system keeps internal state for a process's open files.
   We also have to keep track of the files we modify. This makes merging changes with the parent
   easier to manage. Our implementation, however, limits us to keeping track of a fixed number
   of modified files. This implies that a process can only modify a pre-determined number of files
   before it must sync with its parent. */

/* This structure tracks specific files that a process opens. */
struct file_state_struct
{
    /* Uniquely determines the file we are working with. */
    struct inode *f_inode;

    /* Internal pointer to where we read from or write to. This can be set
       by a dfs_seek. */
    off_t f_offset;

    /* Read/Write/Append etc. */
    int f_mode;
};

/* Our "file descriptors" map directly into the s_openfiles array. */
struct dfs_state_struct
{
    /* Keeps track of currently opened files. Keep one extra spot open (+1)
       so that internal library code can still work with file descriptors. */
    struct file_state_struct s_openfiles[NMAX_OPEN_FILES + 1];

    /* This allows the parent to determine the runnable status of the child. */
    int proc_state;

    int s_is_initialized;
    struct super_block *s_sblock;
    uint8_t *s_bmap;
    uint8_t *s_child_bmap;

    /* When we merge child files into a parent, we sometimes need to copy the
       indirect and direct blocks from the child into parent scratch space. We
       cache the blocks in special locations. Here we track the block number for
       each slot. */
    blockno_t s_block_slot;
    blockno_t s_indirect_slot;
    blockno_t s_double_indirect_slot;

};

/* Define some simply static low level functions. */
#define blockaddr(blockno) ((uint8_t*)(FS_VM_START + (blockno) * FS_BLOCK_SIZE))
#define inodeaddr(ino) (super->s_inode + (ino))
#define inode2ino(i) (((unsigned long)i - (unsigned long)super->s_inode) / sizeof(struct inode))

#endif

