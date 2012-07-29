
#include <inc/mman.h>
#include <inc/syscall.h>

#define UL(a) ((unsigned long)a)

void *mmap(void *addr, size_t size, int prot, int flags, int fd, off_t length)
{
    return (void*)syscall6(SYS_mmap,
            UL(addr),
            UL(size),
            UL(prot),
            UL(flags),
            UL(fd),
            UL(length));
}

int munmap(void *addr, size_t length)
{
    return (int)syscall2(SYS_munmap, UL(addr), UL(length));
}

/* This most closely resembles the actual brk syscall (the C spec's versions
   interface to the brk syscall differently). */
static void *mbrk(void *new_brk)
{
    return (void*)syscall1(SYS_brk, UL(new_brk));
}

/* Dynamically allocate memory.

   -- Allocation --
   Allocate a block that is actually 4 bytes larger than requested. Store the size
   of the allocated block in the first 4 bytes of the block; call this number N.
   The smallest granularity of allocation supported is 4 bytes, so the low two bits
   of the 4-byte size are use for other purposes. The lowest order bit is a reference
   bit. If it is set, it means the next N & ~0x3 bytes of memory afterwards are in use.
   If it is not set, then the next N & ~0x3 bytes of memory are free to be allocated
   by malloc.

 */

static void *orig_brk;
static void *curr_brk;
static void *mem_ptr;

/* Always expand the heap by at least 16 KB. */
#define BYTES_PER_LONG 4
#define INITIAL_SIZE (1024 * 1024)
#define BUFFER_SIZE (0x1000 * 4)
#define PTR(x) ((void*)(x) + BYTES_PER_LONG)
#define BLOCK_SIZE(x) (*(uint32_t*)(x))
#define SET_SIZE(x, sz) do { (*(uint32_t*)(x) = (sz)); } while (0)
#define IS_USED(x) (BLOCK_SIZE(x) & 0x1)
#define SET_USED(x) do { *(uint32_t*)(x) |= 0x1; } while (0)
#define SET_UNUSED(x) do { *(uint32_t*)(x) &= ~0x1; } while (0)

int init_malloc(void)
{
    curr_brk = mem_ptr = orig_brk = (void*)ROUNDUP(UL(mbrk(0)), BYTES_PER_LONG);
    if (orig_brk + INITIAL_SIZE != mbrk(orig_brk + INITIAL_SIZE))
        return -1;
    SET_SIZE(mem_ptr, INITIAL_SIZE);
    SET_UNUSED(mem_ptr);
    curr_brk = orig_brk + INITIAL_SIZE;
    return 0;
}

void *malloc(ssize_t size_)
{
    void *at = mem_ptr;
    uint32_t size = (uint32_t)ROUNDUP(size_, BYTES_PER_LONG) + BYTES_PER_LONG;
    int looped = 0;
    uint32_t expand_size;
    void *expanded;

    /* Scan the current heap for an opening. */
    while (!looped || at < mem_ptr)
    {
        uint32_t at_size = BLOCK_SIZE(at);
        if (!IS_USED(at) && at_size >= size && at_size - size != BYTES_PER_LONG)
        { /* We can use this block. */
            void *leftover = at + size;
            SET_SIZE(at, size);
            SET_USED(at);
            SET_SIZE(leftover, at_size - size);
            SET_UNUSED(leftover);
            mem_ptr = leftover;
            return PTR(at);
        }
        at += at_size;
        if (at >= curr_brk)
        {
            at = orig_brk;
            looped = 1;
        }
    }

    /* Expand the heap. */
    expand_size = size + BUFFER_SIZE;
    expanded = mbrk(curr_brk + expand_size);
    if (expanded != at + expand_size)
        return NULL;
    at = curr_brk;
    curr_brk = expanded;
    SET_SIZE(at, size);
    SET_USED(at);
    SET_SIZE(at + size, expand_size - size);
    SET_UNUSED(at + size);
    return PTR(at);
}

/* TODO combine contiguous regions of freed memory. */
void free(void *ptr)
{
    if (NULL == ptr)
        return;
    if (!IS_USED(ptr))
        return;
    SET_UNUSED(ptr);
}

/* This potentially could be optimized internally. However, the
   current implementation does the dumb thing (allocate more
   than needed to ensure alignment). We assume alignment is a power
   of two.
 
   This function is provided more out of convenience since dput/dget
   require memory to be aligned on a page boundary for copying. The
   caller must keep track of the actual pointer that this library
   knows about to free the memory.
 */
void *malloc_aligned(ssize_t size, uint32_t alignment, void **actual)
{
    /*ssize_t s2ize = size;
    size = ROUNDUP(size, BYTES_PER_LONG);
    ssize_t off = (alignment - size % alignment) % alignment;
    void *v = malloc(size + off);
    if (!v)
        return NULL;
    *actual = v;
    iprintf("But actually (%08lx,%d)\n", ROUNDDOWN(UL(v),alignment),s2ize);
    return (void*)(ROUNDDOWN(UL(v), alignment));*/
    void *v = malloc(size + alignment - 1);
    if (!v)
        return NULL;
    *actual = v;
    return (void*)ROUNDUP(UL(v), alignment);
}

