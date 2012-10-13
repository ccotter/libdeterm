
#include <fs_internal.h>
#include <string.h>
#include <mman.h>
#include <debug.h>

#define FS_FIRST_DATA_BLOCK (2 + FS_BITMAP_BLOCKS + (FS_NINODES * FS_INODE_SIZE + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE)
#define FS_FIRST_INODE_BLOCK ((2 + FS_BITMAP_BLOCKS) * FS_BLOCK_SIZE + FS_VM_START)

/* Ensure the scratch areas are the correct size. */
#if (FS_INODE_SIZE * FS_NINODES) > FS_SCRATCH1_SIZE
	#error "FS scratch space 1 is not large enough to hold all inodes."
#endif
#if (FS_BITMAP_BLOCKS * FS_BLOCK_SIZE) > FS_SCRATCH2_SIZE
	#error "FS scratch space is 2 not large enough to hold child bitmap."
#endif
#if FS_SCRATCH3_SIZE < FS_INODE_SIZE
	#error "FS scratch space 3 is not large enough to hold file block."
#endif
#if FS_SCRATCH4_SIZE < FS_BLOCK_SIZE
	#error "FS scratch space 4 is not large enough to hold indirect block."
#endif
#if FS_SCRATCH5_SIZE < FS_BLOCK_SIZE
	#error "FS scratch space 5 is not large enough to hold double indirect block."
#endif
//static_assert(FS_STATE_SIZE >= sizeof(struct dfs_state_struct));

#define SCRATCH_BITMAP FS_SCRATCH2
#define SCRATCH_BLOCK FS_SCRATCH3
#define SCRATCH_INDIRECT FS_SCRATCH4
#define SCRATCH_DOUBLE_INDIRECT FS_SCRATCH5

#define super (fs_state->s_sblock)
#define bitmap (fs_state->s_bmap)
/* Internal state. */
#define fs_state ((struct dfs_state_struct*)FS_STATE)

/* 
   This file provides implementation of the deterministic user file system. 
   The code is split up into two logical sections. The first implements the
   actual functions that users may use (dfs_open, etc). The section section
   implements static (private) helper functions for use by the use defined
   functions. These private functions know about the internals of the file
   system (inode structure, virtual memory layout).
 */

#define PERM_READ               0x0001
#define PERM_WRITE              0x0002
#define PERM_APPEND             0x0004
#define PERM_DIR                0x0008

/* Easy bitmap manipulation. bitmap should be a uint8_t pointer. */
#define bitmap_set(bm, bit) do {(bm)[(bit) / 8] |= (1 << ((bit) % 8));} while (0)
#define bitmap_clear(bm, bit) do {(bm)[(bit) / 8] &= ~(1 << ((bit) % 8));} while (0)
#define bitmap_present(bm, bit) ((bm)[(bit) / 8] & (1 << ((bit) % 8)))

#define update_file_version(f) \
do { \
	++(f)->f_version; \
} while (0)

/* Private static helper functions. */
static int get_block(struct inode *inode, uint32_t blockno, uint8_t **addr);
static int dir_lookup(struct inode *dir, struct inode **f, const char *name, ssize_t n);
static int dir_add_file(struct inode *dir, struct inode *file, const char *name);
static int dir_unlink(struct inode *dir, const char *name, ssize_t n);
static int file_set_size(struct inode *f, off_t size);
static int walk_path(const char *path, struct inode **parent, struct inode **dir, int create, int mkdir);
static blockno_t alloc_block(uint8_t *bmap, int clear);
static void dealloc_block(uint8_t *bmap, blockno_t blk);
static void putinode(ino_t ino, int been_used);
static ino_t alloc_inode(void);
static void init_inode(struct inode *inode);

static int alloc_fd(void);
static void putfd(int fd);

static int get_open_mode(int flags);

static int fetch_child_block(pid_t childid, struct inode *inode,
		blockno_t blkno, blockno_t *slot, void *dst, int flush, int clear);
static int get_child_block(pid_t childid, struct inode *inode, uint32_t blockno, void *dst);

#define reset_child_cache() \
do { \
	fs_state->s_block_slot = fs_state->s_indirect_slot = fs_state->s_double_indirect_slot = 0; \
} while (0)
/* Given a block number for the child FS, copy the block into local memory (internal cache).
   Keep track of the block number internally. To only flush, set blkno to 0. */
static int fetch_child_block(pid_t childid, struct inode *inode,
		blockno_t blkno, blockno_t *slot, void *dst, int flush, int clear)
{
	if (blkno == *slot)
		return 0;
	if (0 != *slot && flush) {
		if (0 > dput(childid, DET_VM_COPY, (unsigned long)dst, FS_BLOCK_SIZE,
					(unsigned long)blockaddr(*slot)))
			return -E_COPY_ERROR;
	}
	if (0 == blkno)
		return 0;
	if (0 > dget(childid, DET_VM_COPY, (unsigned long)dst,
				FS_BLOCK_SIZE, (unsigned long)blockaddr(blkno)))
		return -E_COPY_ERROR;
	if (clear)
		memset(dst, 0, FS_BLOCK_SIZE);
	*slot = blkno;
	return 0;
}

/* Simple routines to allocate the requested resources either all at
   once or not at all. */
static int alloc_block2(uint8_t* bmap, blockno_t *blk1, blockno_t *blk2, int clear)
{
	blockno_t b1, b2;
	if (0 == (b1 = alloc_block(bmap, clear)))
		return -ENOSPC;
	if (0 == (b2 = alloc_block(bmap, clear))) {
		dealloc_block(bmap, b1);
		return -ENOSPC;
	}
	*blk1 = b1;
	*blk2 = b2;
	return 0;
}
static int alloc_block3(uint8_t *bmap, blockno_t *blk1, blockno_t *blk2, blockno_t *blk3, int clear)
{
	blockno_t b1, b2, b3;
	if (0 == (b1 = alloc_block(bmap, clear)))
		return -ENOSPC;
	if (0 == (b2 = alloc_block(bmap, clear))) {
		dealloc_block(bmap, b1);
		return -ENOSPC;
	}
	if (0 == (b3 = alloc_block(bmap, clear))) {
		dealloc_block(bmap, b1);
		dealloc_block(bmap, b2);
		return -ENOSPC;
	}
	*blk1 = b1;
	*blk2 = b2;
	*blk3 = b3;
	return 0;
}

/* Reads block into memory. If the block does not exist in the file, it (and any necessary
   indirect blocks) are automatically allocated. */
static int get_child_block(pid_t childid, struct inode *inode, uint32_t blockno, void *dst)
{
	int rc, isnew = 0;
	blockno_t *diblk = (blockno_t*)SCRATCH_DOUBLE_INDIRECT;
	blockno_t *iblk = (blockno_t*)SCRATCH_INDIRECT;
	uint32_t outer_block;

	/* Direct blocks are easy. */
	if (blockno < FS_NDIRECT) {
		if (0 == inode->f_direct[blockno]) {
			inode->f_direct[blockno] = alloc_block(fs_state->s_child_bmap, 0);
			if (0 == inode->f_direct[blockno])
				return -ENOSPC;
			isnew = 1;
		}
		return fetch_child_block(childid, inode, inode->f_direct[blockno], &fs_state->s_block_slot, dst, 0, isnew);
	}

	/* For indirect block numbers, cache the indirect block itself to local scratch space. */
	blockno -= FS_NDIRECT;
	if (blockno < FS_NSINGLE_INDIRECT) {
		if (0 == inode->f_indirect) {
			blockno_t aiblk, ablk;
			if ((rc = alloc_block2(fs_state->s_child_bmap, &aiblk, &ablk, 0)))
				return rc;
			inode->f_indirect = aiblk;
			if ((rc = fetch_child_block(childid, inode, inode->f_indirect, &fs_state->s_indirect_slot, iblk, 1, 1)))
				return rc;
			iblk[blockno] = ablk;
		}
		if ((rc = fetch_child_block(childid, inode, inode->f_indirect, &fs_state->s_indirect_slot, iblk, 1, 0)))
			return rc;
		if (0 == iblk[blockno]) {
			iblk[blockno] = alloc_block(fs_state->s_child_bmap, 0);
			if (0 == iblk[blockno])
				return -ENOSPC;
			isnew = 1;
		}
		return fetch_child_block(childid, inode, iblk[blockno], &fs_state->s_block_slot, dst, 0, isnew);
	}

	/* For doubly indirect block numbers, cache the double indirect block. Then proceed as
	   previously for single indirect blocks. */
	blockno -= FS_NSINGLE_INDIRECT;
	outer_block = blockno / FS_NSINGLE_INDIRECT;
	blockno %= FS_NSINGLE_INDIRECT;
	if (0 == inode->f_double_indirect) {
		blockno_t adblk, aiblk, ablk;
		if ((rc = alloc_block3(fs_state->s_child_bmap, &adblk, &aiblk, &ablk, 0)))
			return rc;
		inode->f_double_indirect = adblk;
		if ((rc = fetch_child_block(childid, inode, adblk, &fs_state->s_double_indirect_slot, diblk, 1, 1)))
			return rc;
		diblk[outer_block] = aiblk;
		if ((rc = fetch_child_block(childid, inode, aiblk, &fs_state->s_indirect_slot, iblk, 1, 1)))
			return rc;
		iblk[blockno] = ablk;
		return fetch_child_block(childid, inode, ablk, &fs_state->s_block_slot, dst, 0, 1);
	}

	if ((rc = fetch_child_block(childid, inode, inode->f_double_indirect,
					&fs_state->s_double_indirect_slot, diblk, 1, 0)))
		return rc;

	if (0 == diblk[outer_block]) {
		blockno_t aiblk, ablk;
		if ((rc = alloc_block2(fs_state->s_child_bmap, &aiblk, &ablk, 0)))
			return rc;
		diblk[outer_block] = aiblk;
		if ((rc = fetch_child_block(childid, inode, aiblk, &fs_state->s_indirect_slot, iblk, 1, 1)))
			return rc;
		iblk[blockno] = ablk;
		return fetch_child_block(childid, inode, ablk, &fs_state->s_block_slot, dst, 0, 1);
	}
	if ((rc = fetch_child_block(childid, inode, diblk[outer_block], &fs_state->s_indirect_slot, iblk, 1, 0)))
		return rc;

	if (0 == iblk[blockno]) {
		iblk[blockno] = alloc_block(fs_state->s_child_bmap, 0);
		if (0 == iblk[blockno])
			return -ENOSPC;
		isnew = 1;
	}
	return fetch_child_block(childid, inode, iblk[blockno], &fs_state->s_block_slot, dst, 0, isnew);

}

/* Returns a pointer to the N-th block data. Exactly FS_BLOCK_SIZE bytes are
   guaranteed to be valid. If needed, intermediate (indirect) blocks are automatically
   allocated.

   This function's code is a little complex; we must be careful though to make sure we don't
   leak resources (blocks). */
static int get_block(struct inode *inode, uint32_t blockno, uint8_t **addr)
{
	blockno_t *iblock, *diblock;
	uint32_t outer_block;
	int rc;

	/* Direct blocks. */
	if (blockno < FS_NDIRECT) {
		if (0 == inode->f_direct[blockno])
			if (0 == (inode->f_direct[blockno] = alloc_block(fs_state->s_bmap, 1)))
				return -ENOSPC;
		*addr = blockaddr(inode->f_direct[blockno]);
		return 0;
	}

	/* Singly indirect block. */
	blockno -= FS_NDIRECT;
	if (blockno < FS_NSINGLE_INDIRECT) {
		if (0 == inode->f_indirect) {
			/* Must allocate indirect block. */
			blockno_t aiblk, ablk;
			if ((rc = alloc_block2(fs_state->s_bmap, &aiblk, &ablk, 1)))
				return rc;
			inode->f_indirect = aiblk;
			iblock = (blockno_t*)blockaddr(aiblk);
			iblock[blockno] = ablk;
			*addr = blockaddr(ablk);
			return 0;
		}
		iblock = (blockno_t*)blockaddr(inode->f_indirect);
		if (0 == iblock[blockno]) {
			iblock[blockno] = alloc_block(fs_state->s_bmap, 1);
			if (0 == iblock[blockno])
				return -ENOSPC;
		}
		*addr = blockaddr(iblock[blockno]);
		return 0;
	}

	/* Doubly indirect block. */
	blockno -= FS_NSINGLE_INDIRECT;
	outer_block = blockno / FS_NSINGLE_INDIRECT;
	blockno = blockno % FS_NSINGLE_INDIRECT;
	if (0 == inode->f_double_indirect) {
		/* Allocated doubly indirect block and single indirect block at the same time. */
		blockno_t adblk, aiblk, ablk;
		if ((rc = alloc_block3(fs_state->s_bmap, &adblk, &aiblk, &ablk, 1)))
			return rc;
		inode->f_double_indirect = adblk;
		diblock = (blockno_t*)blockaddr(adblk);
		diblock[outer_block] = aiblk;
		iblock = (blockno_t*)blockaddr(aiblk);
		iblock[blockno] = ablk;
		*addr = blockaddr(ablk);
		return 0;
	}
	diblock = (blockno_t*)blockaddr(inode->f_double_indirect);
	if (0 == diblock[outer_block]) {
		/* Must allocate singly indirect block. */
		blockno_t aiblk, ablk;
		if ((rc = alloc_block2(fs_state->s_bmap, &aiblk, &ablk, 1)))
			return rc;
		diblock[outer_block] = aiblk;
		iblock = (blockno_t*)blockaddr(aiblk);
		iblock[blockno] = ablk;
		*addr = blockaddr(ablk);
		return 0;
	}
	iblock = (blockno_t*)blockaddr(diblock[outer_block]);
	if (0 == iblock[blockno]) {
		iblock[blockno] = alloc_block(fs_state->s_bmap, 1);
		if (0 == iblock[blockno])
			return -ENOSPC;
	}
	*addr = blockaddr(iblock[blockno]);
	return 0;
}

/* Scans a directory entry looking for a file. If it is found, a pointer to
   the inode is returned. Otherwise, NULL is returned. */
static int dir_lookup(struct inode *dir, struct inode **f, const char *name, ssize_t n)
{
	struct direntry *dentry;
	uint32_t block, nblocks;
	int rc;

	/* Loop over blocks and inside each block, look at each direntry. 
	   We are done once we find the first direntry with d_ino=0. */
	nblocks = dir->f_size / FS_BLOCK_SIZE;
	block = 0;
	if ((rc = get_block(dir, block, (void*)&dentry)))
		return rc;

	if ((n < 0 && 0 == strcmp(name, ".")) || (n >= 0 && 0 == strncmp(name, ".", n))) {
		*f = dir;
		return 0;
	}

	while (dentry && block < nblocks) {
		uint32_t j;
		for (j = 0; j < NDIRENTRIES_PER_BLOCK; ++j) {
			if (0 == dentry[j].d_ino)
				return -E_NOEXIST;
			if (n < 0) {
				if (0 == strcmp(name, dentry[j].d_name)) {
					*f = inodeaddr(dentry[j].d_ino);
					return 0;
				}
			} else {
				if (0 == strncmp(name, dentry[j].d_name, n)) {
					*f = inodeaddr(dentry[j].d_ino);
					return 0;
				}
			}
		}
		++block;
		if ((rc = get_block(dir, block, (void*)&dentry)))
			return rc;
	}
	return -E_NOEXIST;
}

static int dir_add_file(struct inode *dir, struct inode *file, const char *name)
{
	struct direntry *dentry;
	uint32_t block, nblocks;
	int rc;

	block = 0;
	nblocks = dir->f_size / FS_BLOCK_SIZE;
	if ((rc = get_block(dir, block, (void*)&dentry)))
		return rc;
	while (dentry && block < nblocks) {
		uint32_t j;
		for (j = 0; j < NDIRENTRIES_PER_BLOCK; ++j) {
			if (0 == dentry[j].d_ino) {
				dentry[j].d_ino = inode2ino(file);
				strcpy(dentry[j].d_name, name);
				dentry[j].d_type = file->f_type;
				update_file_version(dir);
				return 0;
			}
		}
		++block;
		if ((rc = get_block(dir, block, (void*)&dentry)))
			return rc;
	}

	/* Add another block if we can. */
	if ((rc = get_block(dir, block, (void*)&dentry)))
		return rc;

	dir->f_size += FS_BLOCK_SIZE;
	dentry[0].d_ino = inode2ino(file);
	strcpy(dentry[0].d_name, name);
	dentry[0].d_type = file->f_type;
	update_file_version(dir);

	return 0;
}

static int dir_unlink(struct inode *dir, const char *name, ssize_t n)
{
	struct direntry *dentry, *to_remove, *last;
	uint32_t block, nblocks;
	int rc;

	/* Find the direntry to remove, then find the last direntry. Replace the
	   direntry to remove with the last direntry. */
	block = 0;
	to_remove = NULL;
	nblocks = dir->f_size / FS_BLOCK_SIZE;
	if ((rc = get_block(dir, block, (void*)&dentry)))
		return rc;
	while (dentry && block < nblocks) {
		uint32_t j;
		for (j = 0; j < NDIRENTRIES_PER_BLOCK; ++j) {
			if (0 == dentry[j].d_ino)
				break; /* This will also break the while loop since get_block will return NULL. */

			if ((n < 0 && 0 == strcmp(name, dentry[j].d_name)) ||
					(n >= 0 && 0 == strncmp(name, dentry[j].d_name, n))) {
				to_remove = &dentry[j];
			}
			last = &dentry[j];
		}
		++block;
		if ((rc = get_block(dir, block, (void*)&dentry)))
			return rc;
	}

	if (to_remove == last) {
		/* Since the last entry is the one to remove, we simply delete and return. */
		last->d_ino = 0;
		update_file_version(dir);
		return 0;
	}

	to_remove->d_ino = last->d_ino;
	strcpy(to_remove->d_name, last->d_name);
	to_remove->d_type = last->d_type;
	last->d_ino = 0;
	update_file_version(dir);
	return 0;
}

static int file_set_size(struct inode *f, off_t size)
{
	if (f->f_size > size) {
		/* TODO Truncate. */
	}
	f->f_size = size;
	return 0;
}

static const char *skip_slashes(const char *p)
{
	while ('/' == *p)
		++p;
	return p;
}

/* path must be a absolute path (i.e. starts with a forward slash from the root directory). 
   We assume that the last character is not a slash with the sole exception that path is "/"
   (we do not handle multiple slashes, we assume the root directory will always be referred to
   with one slash). */
static int walk_path(const char *path, struct inode **file, struct inode **parent, int create, int mkdir)
{
	struct inode *at, *f;
	const char *slash, *next;
	int rc;

	/* Handle exception. */
	if (0 == strcmp(path, "/")) {
		*file = *parent = inodeaddr(super->s_root);
		return 0;
	}

	at = inodeaddr(super->s_root);
	slash = skip_slashes(strchr(path, '/'));
	next = strchr(slash, '/');
	while (NULL != next) {
		/* Iterate over directory entires to find the file. */
		rc = dir_lookup(at, &f, slash, next - slash);
		if (rc)
			return -E_NOEXIST;
		if (FS_DIR_TYPE != f->f_type)
			return -E_NOEXIST;

		slash = skip_slashes(next + 1);
		next = strchr(slash, '/');
		at = f;
	}

	/* at points to the parent directory of the file. */
	/* If the last character was a slash, then we expect that 'at' is a directory
	   and we should return that at the file. */
	rc = dir_lookup(at, &f, slash, -1);
	if (rc) {
		if (create) {
			ino_t newf = alloc_inode();
			f = inodeaddr(newf);
			init_inode(f);
			if (mkdir) {
				/* Set ".." entry to parent. */
				struct direntry *dentry;
				f->f_type = FS_DIR_TYPE;
				f->f_size = FS_BLOCK_SIZE;
				if ((rc = get_block(f, 0, (void*)&dentry))) {
					putinode(newf, 0);
					return rc;
				}
				dentry[0].d_ino = inode2ino(at);
				strcpy(dentry[0].d_name, "..");
				dentry[0].d_type = FS_DIR_TYPE;
			} else {
				f->f_type = FS_FILE_TYPE;
			}

			f->f_version = 1;
			if ((rc = dir_add_file(at, f, slash))) {
				putinode(newf, 0);
				return rc;
			}
		} else {
			return rc;
		}
	}
	if (parent)
		*parent = at;
	*file = f;

	return 0;
}

/* Finds a free block and returns the block number. Returns 0 if none
   are available since blockno 0 is reserved. The block is zeroed out. */
static blockno_t alloc_block(uint8_t *bmap, int clear)
{
	blockno_t b;
	for (b = FS_FIRST_DATA_BLOCK; b < FS_NBLOCKS; ++b) {
		if (bitmap_present(bmap, b)) {
			/* Clear the block. */
			bitmap_clear(bmap, b);
			if (clear)
				memset(blockaddr(b), 0, FS_BLOCK_SIZE);
			return b;
		}
	}
	return 0;
}

static void dealloc_block(uint8_t *bmap, blockno_t blk)
{
	bitmap_set(bmap, blk);
}

/* Find a free inode and return its inumber. Set the refcount to 1. */
static ino_t alloc_inode(void)
{
	ino_t i;
	for (i = 1; i < FS_NINODES; ++i) {
		struct inode *inode = inodeaddr(i);
		if (0 == inode->f_refcount && !(INODE_DELETED & inode->f_flags)) {
			++inode->f_refcount;
			return i;
		}
	}
	return 0; /* inumber 0 is never used. */
}

/* been_used is set to 0 if the inode was allocated, but immediately
   deallocated because some resource was not able to be allocated. If
   been_used is not 0, then we will mark the inode as having been
   actually "deleted". */
static void putinode(ino_t i, int been_used)
{
	struct inode *inode = inodeaddr(i);
	--inode->f_refcount;
	if (0 == inode->f_refcount && been_used) {
		update_file_version(inode);
		inode->f_flags |= INODE_DELETED;
	}
}

static void init_inode(struct inode *inode)
{
	uint32_t ref = inode->f_refcount;
	memset(inode, 0, sizeof(struct inode));
	inode->f_refcount = ref;
}

/* Ensures that the deterministic file system internals are initiailized.
   If there are any inconsistencies, this returns a useful error code
   (negative integer).*/
#define init_dfs()                      \
do {                                    \
	int __rc;                           \
	if (!fs_state->s_is_initialized &&     \
			(__rc = _init_dfs()) < 0)   \
		return __rc;                    \
} while(0)
int _init_dfs(void)
{
	if (fs_state->s_is_initialized)
		return 0;

	memset(fs_state, 0, sizeof(struct dfs_state_struct));
	super = (struct super_block*)blockaddr(1);
	fs_state->s_bmap = blockaddr(2);
	fs_state->s_is_initialized = 1;
	return 0;
}

/* Management of file descriptors. */
/* Returns >= 0 on success, < 0 if none are available. */
static int alloc_fd(void)
{
	int at;
	for (at = 0; at < NMAX_OPEN_FILES; ++at) {
		if (0 == fs_state->s_openfiles[at].f_inode) {
			return at;
		}
	}
	return -E_MAXFILES;
}
/* Called when a process closes a file descriptor. */
static void putfd(int fd)
{
	/* memset(&fs_state->s_openfiles[fd], 0, sizeof(struct file_state_struct)); */
	fs_state->s_openfiles[fd].f_inode = 0;
}

/* Converts dfs_open() flags into a more manageable representation.
   Also checks for conflicting mode flags. */
static int get_open_mode(int flags)
{
	int ret;
	if ((DFS_O_APPEND & flags) &&
			(DFS_O_RDONLY | DFS_O_RDWR | DFS_O_WRONLY) & flags)
		return -1;

	if (DFS_O_RDONLY & flags)
		ret = PERM_READ;
	else if (DFS_O_RDWR & flags)
		ret = PERM_READ | PERM_WRITE;
	else if (DFS_O_WRONLY & flags)
		ret = PERM_WRITE;
	else if (DFS_O_APPEND & flags)
		ret = PERM_APPEND;
	else
		ret = 0;

	return ret;
}

/* Implementation of user visible functions. */

/* Initializes the file system to a clean state. Since the file system
 * exists entirely in memory, initializing the file system to a clean
 * slate allows the process to then populate the file system. The file
 * system has only one file after initialization, the root directory "/".
 *
 * Should only be used internally by the library upon startup.
 */
int __dfs_init_clean(void)
{
	blockno_t b;
	struct inode *root;
	struct direntry *dentry;
	int rc, i;
	struct inode *inodes;

	/* Map the file system memeory region. */
	void *addr = mmap((void*)FS_VM_START, FS_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_VM_START != addr)
		return -ENOMEM;
	/* Also map the scratch space. */
	addr = mmap((void*)FS_SCRATCH1, FS_SCRATCH1_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_SCRATCH1 != addr)
		return -ENOMEM;
	addr = mmap((void*)FS_SCRATCH2, FS_SCRATCH2_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_SCRATCH2 != addr)
		return -ENOMEM;
	addr = mmap((void*)FS_SCRATCH3, FS_SCRATCH3_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_SCRATCH3 != addr)
		return -ENOMEM;
	addr = mmap((void*)FS_SCRATCH4, FS_SCRATCH4_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_SCRATCH4 != addr)
		return -ENOMEM;
	addr = mmap((void*)FS_SCRATCH5, FS_SCRATCH5_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_SCRATCH5 != addr)
		return -ENOMEM;
	addr = mmap((void*)FS_STATE, FS_STATE_SIZE, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((void*)FS_STATE != addr)
		return -ENOMEM;

	memset(fs_state, 0, sizeof(struct dfs_state_struct));
	super = (struct super_block*)blockaddr(1);
	super->s_magic = FS_MAGIC;
	super->s_nblocks = FS_NBLOCKS;

	/* Initialize the bitmap. */
	fs_state->s_bmap = blockaddr(2);
	memset(fs_state->s_bmap, 0xFF, FS_BITMAP_BLOCKS * FS_BLOCK_SIZE);

	/* Mark the first two blocks, the bitmap and the inode
	   blocks as taken. */
	for (b = 0; b < FS_FIRST_DATA_BLOCK; ++b) {
		bitmap_clear(fs_state->s_bmap, b);
	}

	/* Calculate where the first inode is. This is after the bitmap.*/
	super->s_inode = (struct inode*)blockaddr(FS_BITMAP_BLOCKS + 2);
	/* Mark all inodes as not in use. */
	inodes = (struct inode*)FS_FIRST_INODE_BLOCK;
	for (i = 0; i < FS_NINODES; ++i)
		inodes[i].f_version = inodes[i].f_refcount = inodes[i].f_flags = 0;

	/* Setup the root directory "/" */
	super->s_root = alloc_inode();
	if (0 == super->s_root)
		return -ENOSPC;
	root = inodeaddr(super->s_root);
	init_inode(root);
	root->f_version = 1;
	root->f_size = FS_BLOCK_SIZE;
	root->f_type = FS_DIR_TYPE;
	if ((rc = get_block(root, 0, (void*)&dentry)))
		return rc;
	dentry[0].d_ino = super->s_root;
	dentry[0].d_type = FS_DIR_TYPE;
	strcpy(dentry[0].d_name, "..");

	return 0;
}

/* Adds initial slash if not already present. */
static char path_buffer[MAX_PATH_LENGTH + 1];
static int normalize_path(const char *path)
{
	char *pb = path_buffer;
	int len = 0;
	if ('/' != *path) {
		*pb = '/';
		++pb; ++len;
	}
	while (*path && len < MAX_PATH_LENGTH) {
		*pb = *path;
		++pb; ++path; ++len;
	}
	if (*path) {
		return -1; /* Not enough space in path_buffer. Use malloc? */
	}
	*pb = 0;
	--pb;
	while ('/' == *pb && pb >= path_buffer) {
		*pb = 0;
		--pb;
	}
	if (0 == path_buffer[0]) {
		path_buffer[0] = '/';
		path_buffer[1] = 0;
	}
	return 0;
}

int dfs_open(const char *path, int flags)
{
	struct inode *file, *dir;
	int rc, fd, mode;

	mode = get_open_mode(flags);
	if (-1 == mode)
		return -EINVAL;

	/* See if the file exists. */
	fd = alloc_fd();
	if (normalize_path(path))
		return -EINVAL;
	rc = walk_path(path_buffer, &file, &dir, DFS_O_CREAT & flags, 0);
	if (rc) {
		putfd(fd);
		return rc;
	}

	if (FS_DIR_TYPE == file->f_type) {
		putfd(fd);
		return -EINVAL;
	}

	if (DFS_O_APPEND & flags) {
		fs_state->s_openfiles[fd].f_offset = file->f_size;
		file->f_append_off = file->f_size;
		file->f_flags |= INODE_APPENDED;
	}
	else
		fs_state->s_openfiles[fd].f_offset = 0;
	fs_state->s_openfiles[fd].f_inode = file;
	fs_state->s_openfiles[fd].f_mode = mode;

	return fd;
}

int dfs_mkdir(const char *path)
{
	struct inode *dir, *file;

	if (normalize_path(path))
		return -EINVAL;
	return walk_path(path_buffer, &file, &dir, 1, 1);
}

static int _dfs_read(struct file_state_struct *state, void *buf, size_t len)
{
	struct inode *f;
	off_t pos, offset, end;

	f = state->f_inode;
	offset = state->f_offset;

	len = MIN(len, (size_t)(f->f_size - offset));
	/* Read bytes FS_BLOCK_SIZE at a time. */
	end = offset + len;
	for (pos = state->f_offset; pos < end; ) {
		uint8_t *blk;
		get_block(f, pos / FS_BLOCK_SIZE, &blk);
		off_t rd = MIN(FS_BLOCK_SIZE - pos % FS_BLOCK_SIZE,
				(off_t)(offset + len - pos));
		memmove(buf, blk + pos % FS_BLOCK_SIZE, rd);
		pos += rd;
		buf += rd;
	}
	state->f_offset += len;

	return len;
}

/* Assumes that buf is writable and valid. */
int dfs_read(int fd, void *buf, size_t len)
{
	struct file_state_struct *state = &fs_state->s_openfiles[fd];

	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == state->f_inode)
		return -EINVAL;
	if (!(PERM_READ & state->f_mode))
		return -EPERM;
	if (PERM_DIR & state->f_mode)
		return -EINVAL;

	return _dfs_read(state, buf, len);
}

/* Doesn't check for validity of arguments (dfs_write does that). */
static size_t _dfs_write(struct file_state_struct *state, const void *buf, size_t len)
{
	struct inode *f;
	off_t pos, offset, end, written;
	size_t maxlen;

	f = state->f_inode;
	offset = state->f_offset;
	maxlen = offset + len;

	/* Write bytes FS_BLOCK_SIZE at a time. */
	end = offset + len;
	written = 0;
	for (pos = state->f_offset; pos < end; ) {
		uint8_t *blk;
		off_t rd = MIN(FS_BLOCK_SIZE - pos % FS_BLOCK_SIZE, (off_t)(offset + len - pos));
		if (pos + rd > f->f_size) {
			size_t newsize = MIN((size_t)ROUNDUP(f->f_size + 1, FS_BLOCK_SIZE), maxlen);
			if (file_set_size(f, newsize))
				break;
		}
		if (get_block(f, pos / FS_BLOCK_SIZE, &blk))
			break;
		memmove(blk + pos % FS_BLOCK_SIZE, buf, rd);
		pos += rd;
		buf += rd;
		written += rd;
	}
	state->f_offset += written;
	if (written >= 0)
		update_file_version(f);

	return written;
}

int dfs_write(int fd, const void *buf, size_t len)
{
	struct file_state_struct *state = &fs_state->s_openfiles[fd];

	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;
	if (!(PERM_WRITE & state->f_mode) && !(PERM_APPEND & state->f_mode))
		return -EPERM;
	if (PERM_DIR & state->f_mode)
		return -EINVAL;

	return _dfs_write(state, buf, len);
}

int dfs_close(int fd)
{
	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;
	if (PERM_DIR & fs_state->s_openfiles[fd].f_mode)
		return -EINVAL;
	putfd(fd);
	return 0;
}

int dfs_unlink(const char *path)
{
	int rc, i;
	struct inode *dir, *file;
	const char *name;

	if (normalize_path(path))
		return -EINVAL;
	if ((rc = walk_path(path_buffer, &file, &dir, 0, 0)))
		return rc;

	/* See if the inode is opened. */
	for (i = 0; i < NMAX_OPEN_FILES; ++i) {
		if (file == fs_state->s_openfiles[i].f_inode)
			return -E_ALREADY_OPENED;
	}

	/* If the file is a directory, make sure it is empty. */
	if (FS_DIR_TYPE == file->f_type) {
		struct direntry *dentry;
		get_block(file, 0, (void*)&dentry);
		++dentry; /* First should be the ".." entry. */
		if (dentry->d_ino)
			return -ENOTEMPTY;
	}

	name = strrchr(path_buffer, '/') + 1;
	if ((rc = dir_unlink(dir, name, -1)))
		return rc;
	putinode(inode2ino(file), 1);

	return 0;
}

int dfs_opendir(const char *path)
{
	struct inode *file, *parent;
	int rc, fd;

	/* See if the file exists.*/
	fd = alloc_fd();
	if (normalize_path(path))
		return -EINVAL;
	if ((rc = walk_path(path_buffer, &file, &parent, 0, 0))) {
		putfd(fd);
		return rc;
	}

	if (FS_DIR_TYPE != file->f_type) {
		putfd(fd);
		return -ENOTDIR;
	}

	fs_state->s_openfiles[fd].f_offset = 0;
	fs_state->s_openfiles[fd].f_inode = file;
	fs_state->s_openfiles[fd].f_mode = PERM_DIR | PERM_READ;

	return fd;
}

int dfs_readdir(int fd, struct direntry *dentry)
{
	struct file_state_struct *state;
	struct inode *f;
	struct direntry *ent;

	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;

	state = &fs_state->s_openfiles[fd];
	f = state->f_inode;

	if (!(PERM_READ & state->f_mode))
		return -EPERM;
	if (!(PERM_DIR & state->f_mode))
		return -EINVAL;

	if (state->f_offset > f->f_size)
		//return NULL; TODO
		return -1;
	get_block(f, state->f_offset / FS_BLOCK_SIZE, (void*)&ent);
	ent += (state->f_offset % FS_BLOCK_SIZE) / sizeof(struct direntry);
	if (0 == ent->d_ino)
		//return NULL; TODO
		return -1;
	dentry->d_ino = ent->d_ino;
	strcpy(dentry->d_name, ent->d_name);
	dentry->d_type = ent->d_type;
	state->f_offset += sizeof(struct direntry);

	return 0;
}

int dfs_rewinddir(int fd)
{
	struct file_state_struct *state;

	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;

	state = &fs_state->s_openfiles[fd];

	if (!(PERM_READ & state->f_mode))
		return -EPERM;
	if (!(PERM_DIR & state->f_mode))
		return -EINVAL;

	state->f_offset = 0;

	return 0;
}

int dfs_closedir(int fd)
{
	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;
	if (!(PERM_DIR & fs_state->s_openfiles[fd].f_mode))
		return -EINVAL;
	putfd(fd);
	return 0;
}

/* Merge parent/child file systems. */

int dfs_put(pid_t childid)
{
	uint32_t i;
	int rc;
	void *copy_start;
	size_t copy_size;
	struct inode *cinodes = (struct inode*)FS_SCRATCH1;

	/* Copy everything up to the inode area, then copy the entire data block region. */
	copy_start = (void*)FS_VM_START;
	copy_size = (2 + FS_BITMAP_BLOCKS) * FS_BLOCK_SIZE;
	if (0 > (rc = dput(childid, DET_VM_COPY, (unsigned long)copy_start, copy_size,
					(unsigned long)copy_start)))
		return -E_COPY_ERROR;

	copy_start += copy_size;
	copy_start += FS_NINODES * FS_INODE_SIZE;
	copy_size = (FS_SIZE - ((unsigned long)copy_start - FS_VM_START));

	if (0 > (rc = dput(childid, DET_VM_COPY, (unsigned long)copy_start, copy_size,
					(unsigned long)copy_start)))
		return -E_COPY_ERROR;
	/* Copy the file state. */
	if (0 > (rc = dput(childid, DET_VM_COPY, (unsigned long)fs_state,
					sizeof(struct dfs_state_struct), (unsigned long)fs_state)))
		return -E_COPY_ERROR;

	/* In the scratch space, update the reference version. Then copy the inodes
	   into the child. */
	memmove(cinodes, (void*)FS_FIRST_INODE_BLOCK, FS_NINODES * FS_INODE_SIZE);
	for (i = 0; i < FS_NINODES; ++i) {
		cinodes[i].f_rversion = cinodes[i].f_version;
	}

	copy_start = (void*)FS_FIRST_INODE_BLOCK;
	if (0 > (rc = dput(childid, DET_VM_COPY, (unsigned long)cinodes,
					FS_NINODES * FS_INODE_SIZE, (unsigned long)copy_start)))
		return -E_COPY_ERROR;

	/* Map the scratch spaces so that the virtual memory space is valid in the child. */
	if (0 > (rc = dput(childid, DET_VM_COPY, FS_SCRATCH1, FS_STATE - FS_SCRATCH1, FS_SCRATCH1)))
		return -E_COPY_ERROR;

	return 0;
}

int dfs_prepare_for_put(void)
{
	return 0;
}

static int reconcile_inode(struct inode *pinode, struct inode *cinode, pid_t childid)
{
	int fd = FS_INTERNAL_FD, rc;
	uint32_t blockno, nblocks;
	struct file_state_struct *state = &fs_state->s_openfiles[fd];
	reset_child_cache();

	/* If the file is marked as bad or deleted, then apply this to the parent inode
	   and return. Otherwise, copy the entire file contents into the parent. */
	if (INODE_DELETED & cinode->f_flags) {
		/* Mark parent deleted and actually delete the child inode. */
		file_set_size(pinode, 0);
		pinode->f_flags |= INODE_DELETED;
		cinode->f_refcount = 0;
		init_inode(cinode);
		return 0;
	}

	/* "Open" file in parent for writing. */
	fs_state->s_openfiles[fd].f_offset = 0;
	fs_state->s_openfiles[fd].f_inode = pinode;
	file_set_size(pinode, 0); /* Delete all contents. */

	/* Iterate over child blocks and write them to file fd. */
	nblocks = (cinode->f_size  + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
	rc = 0;
	for (blockno = 0; blockno < nblocks; ++blockno) {
		/* Even though dfs_write's contract doesn't guarantee to write all the
		   bytes requested,  we (the implementers) know it actually will unless
		   the FS runs out of space. */
		if ((rc = get_child_block(childid, cinode, blockno, (void*)SCRATCH_BLOCK)))
			break;

		if (FS_BLOCK_SIZE != (rc = _dfs_write(state, (void*)SCRATCH_BLOCK, FS_BLOCK_SIZE))) {
			rc = ENOSPC;
			break;
		}
		rc = 0;
	}
	file_set_size(pinode, cinode->f_size);

	fs_state->s_openfiles[fd].f_inode = 0;
	return rc;
}

static uint8_t append_buffer[FS_BLOCK_SIZE];
static int reconcile_append(struct inode *pinode, struct inode *cinode, pid_t childid)
{
	int fd = FS_INTERNAL_FD, rc;
	struct file_state_struct *state = &fs_state->s_openfiles[fd];
	off_t pos, offset, end, ppos, written;
	size_t len;
	uint8_t *blk = (void*)SCRATCH_BLOCK;
	reset_child_cache();

	/* First, copy child writes into parent. Then copy parent writes into child. */
	fs_state->s_openfiles[fd].f_offset = pinode->f_size;
	fs_state->s_openfiles[fd].f_inode = pinode;

	ppos = pinode->f_size;
	pos = cinode->f_append_off;
	end = cinode->f_size;
	len = end - pos;
	while (pos < end) {
		size_t rd = MIN(FS_BLOCK_SIZE - pos % FS_BLOCK_SIZE, (off_t)(end - pos));
		if ((rc = get_child_block(childid, cinode, pos / FS_BLOCK_SIZE, blk)))
			goto end;

		if (rd != _dfs_write(state, blk + pos % FS_BLOCK_SIZE, rd)) {
			rc = -ENOSPC;
			goto end;
		}
		pos += rd;
	}

	/* Now append to child file. */
	fs_state->s_openfiles[fd].f_offset = pinode->f_append_off;
	pos = cinode->f_size;
	end = pos + (ppos - pinode->f_append_off);
	written = 0;
	while (pos < end) {
		size_t rd = MIN(FS_BLOCK_SIZE - pos % FS_BLOCK_SIZE, (off_t)(end - pos));
		_dfs_read(state, append_buffer, rd);
		if ((rc = get_child_block(childid, cinode, pos / FS_BLOCK_SIZE, blk)))
			goto end;
		memmove(blk + pos % FS_BLOCK_SIZE, append_buffer, rd);
		if ((rc = fetch_child_block(childid, cinode, 0, &fs_state->s_block_slot, blk, 1, 0)))
			goto end;
		pos += rd;
		written += rd;
	}

	/* Flush back blocks to child. */
	if ((rc = fetch_child_block(childid, cinode, 0, &fs_state->s_block_slot, blk, 1, 0)))
		goto end;
	if ((rc = fetch_child_block(childid, cinode, 0, &fs_state->s_indirect_slot, (void*)SCRATCH_INDIRECT, 1, 0)))
		goto end;
	if ((rc = fetch_child_block(childid, cinode, 0, &fs_state->s_double_indirect_slot,
					(void*)SCRATCH_DOUBLE_INDIRECT, 1, 0)))
		goto end;
	cinode->f_size += written;

end:
	fs_state->s_openfiles[fd].f_inode = 0;
	//cinode->f_flags &= ~INODE_APPENDED;
	//pinode->f_flags &= ~INODE_APPENDED;
	cinode->f_append_off = cinode->f_size;
	//pinode->f_append_off = pinode->f_size;
	update_file_version(cinode);
	update_file_version(pinode);

	return rc;
}

int dfs_get(pid_t childid)
{
	int rc;
	uint32_t ino, i;
	struct file_state_struct *at;
	struct inode *cinodes = (struct inode*)FS_SCRATCH1;
	struct inode *pinodes = (struct inode*)FS_FIRST_INODE_BLOCK;
	uint8_t *cbitmap = (uint8_t*)SCRATCH_BITMAP;
	fs_state->s_child_bmap = cbitmap;

	/* First, make sure all file descriptors are free (the process can not keep files
	   open during a file system merge. */
	/*for (i = 0; i < NMAX_OPEN_FILES; ++i) {
		if (fs_state->s_openfiles[i].f_inode)
			return -E_FILES_OPEN;
	}*/

	/* Copy the inodes from the child process into scratch space. */
	if (0 > dget(childid, DET_VM_COPY, (unsigned long)cinodes,
				FS_NINODES * FS_INODE_SIZE, (unsigned long)pinodes))
		return -E_COPY_ERROR;
	/* Copy bitmap from child process. */
	if (0 > dget(childid, DET_VM_COPY, (unsigned long)cbitmap,
				FS_BITMAP_BLOCKS * FS_BLOCK_SIZE, (unsigned long)blockaddr(2)))
		return -E_COPY_ERROR;

	/* Reconciliation is a two pass process. The first pass check for any merge conflicts
	   (concurrent writes to the same file). If no conflicts are found, the file system is
	   merged into the parent on a second pass. If a conflict is found then we return indicating
	   failure. */
	for (ino = 0; ino < FS_NINODES; ++ino) {
		if (0 == cinodes[ino].f_version)
			continue; /* Not in use by the child. */
		if (cinodes[ino].f_version == cinodes[ino].f_rversion)
			continue; /* Not changed in child. */
		if ((INODE_APPENDED & pinodes[ino].f_flags) && (INODE_APPENDED & cinodes[ino].f_flags))
			continue;
		if (pinodes[ino].f_version == cinodes[ino].f_rversion)
			continue;
		/* Parent and child changed the inode. Conflict error! */
		return -E_CONFLICT;
	}

	for (ino = 0; ino < FS_NINODES; ++ino) {
		if (0 == cinodes[ino].f_version)
			continue; /* Not in use by the child. */
		if (cinodes[ino].f_version == cinodes[ino].f_rversion)
			continue; /* Not changed in child. */

		if ((INODE_APPENDED & pinodes[ino].f_flags) && (INODE_APPENDED & cinodes[ino].f_flags)) {
			/* Append only changes made. */
			if ((rc = reconcile_append(&pinodes[ino], &cinodes[ino], childid))) {
				return rc;
			}
		}

		if (pinodes[ino].f_version == cinodes[ino].f_rversion) {
			/* Not changed in parent. Copy child inode changes to parent. */
			if ((rc = reconcile_inode(&pinodes[ino], &cinodes[ino], childid))) {
				return rc;
			}
		}
	}

	/* Update local file descriptor offsets for updated files. */
	/* TODO also need to update offsets that were only changed by the child, etc */
	for (i = 0, at = &fs_state->s_openfiles[0]; i < NMAX_OPEN_FILES; ++i, ++at) {
		struct inode *f = at->f_inode;
		if (f && (INODE_APPENDED & f->f_flags))
			at->f_offset = f->f_size;
	}

	/* Once we finish, copy the child inodes we have been working on back into the
	   child since we possibly made changes to the child's inodes. Also copy back the*/
	if (0 > dput(childid, DET_VM_COPY, (unsigned long)cinodes,
				FS_NINODES * FS_INODE_SIZE, (unsigned long)pinodes))
		return -E_COPY_ERROR;

	return 0;
}

static int is_io_init;
int _init_io(void)
{
	int stdout_, stdin_;

	if (is_io_init)
		return -E_ALREADY_OPENED;
	
	stdin_ = dfs_open("/__stdin", DFS_O_CREAT | DFS_O_RDONLY);
	stdout_ = dfs_open("/__stdout", DFS_O_CREAT | DFS_O_APPEND);
	assert(0 == stdin_);
	assert(1 == stdout_);
	is_io_init = 1;

	return 0;
}

int dfs_seek(int fd, long offset, int whence)
{
	struct file_state_struct *state = &fs_state->s_openfiles[fd];
	struct inode *f;
	off_t sz, noff;

	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;

	f = state->f_inode;
	sz = f->f_size;
	
	switch(whence) {
		case DSEEK_SET:
			noff = offset;
			break;
		case DSEEK_CUR:
			noff = state->f_offset + offset;
			break;
		case DSEEK_END:
			noff = sz + offset;
			break;
		default:
			return -EINVAL;
	}
	if (noff < 0)
		state->f_offset = 0;
	else if (noff > sz)
		state->f_offset = sz;
	else
		state->f_offset = noff;
	return state->f_offset;
}

void dfs_setstate(int state)
{
	fs_state->proc_state = state;
}

int dfs_childstate(pid_t childid)
{
	struct dfs_state_struct *cstate = (struct dfs_state_struct*)FS_SCRATCH1;
	int rc = dget(childid, DET_VM_COPY, (unsigned long)cstate, sizeof(struct dfs_state_struct),
			FS_STATE);
	if (DET_S_READY != rc)
		return -E_COPY_ERROR;
	return cstate->proc_state;
}

void dfs_close_all(void)
{
	int fd;
	for (fd = 0; fd < NMAX_OPEN_FILES; ++fd) {
		dfs_close(fd);
	}
}

int dfs_truncate(int fd, off_t len)
{
	struct file_state_struct *state = &fs_state->s_openfiles[fd];

	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;
	if (len < 0)
		return -EINVAL;

	file_set_size(state->f_inode, len);
	return 0;
}

long dfs_tell(int fd)
{
	struct file_state_struct *state = &fs_state->s_openfiles[fd];
	if (fd < 0 || fd >= NMAX_OPEN_FILES || 0 == fs_state->s_openfiles[fd].f_inode)
		return -EINVAL;
	return state->f_offset;
}

