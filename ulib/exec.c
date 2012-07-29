
#include <inc/mman.h>
#include <inc/determinism_pure.h>
#include <inc/fs.h>
#include <inc/elf.h>
#include <inc/std.h>
#include <inc/fork.h>

/* Assumes align is a power of two. */
#define PAGE_SIZE 0x1000
static int map_segment(int childid, unsigned long va, size_t memsz,
        int fd, size_t filesz, off_t fileoff, int prot, uint32_t align)
{
    int i, rc;
    void *vaddr, *tofree;

    vaddr = malloc_aligned(PAGE_SIZE, PAGE_SIZE, &tofree);
    if ((i = ((align-1) & va)))
    {
        va -= i;
        memsz += i;
        filesz += i;
        fileoff -= i;
    }

    i = 0;
    while (i < memsz)
    {
        if (i >= filesz)
        { /* Zeroed area. */
            if (0 > dput(childid, DETERMINE_ZERO_FILL, NULL, (memsz - i), (void*)(va + i)))
            {
                free(tofree);
                return -1;
            }
            break;
        }
        else
        { /* From file. */
            int end = PAGE_SIZE + i;
            if (end >= filesz)
                end = filesz;
            if (0 > (rc = dfs_seek(fd, fileoff + i, DSEEK_SET)))
            {
                free(tofree);
                return -1;
            }
            if (end - i != (rc = dfs_read(fd, vaddr, end - i)))
            {
                free(tofree);
                return -1;
            }
            if (0 > dput(childid, DETERMINE_VM_COPY, vaddr, end - i, (void*)(va + i)))
            {
                free(tofree);
                return -1;
            }
            i += end - i;
        }
    }
    free(tofree);
    return 0;
}

#define STACK (0xC0000000)
/* Create a region in the child containing the initial stack arguments. 
   This code does a lot of pointer arithmetic and might have been easier
   (or just as difficult depending on your viewpoint) to code in assembly. */
static unsigned long create_stack(int childid, char **argv, char **env)
{
    int argc, envc, i, strspace;
    unsigned long esp, *stack, diff;
    ssize_t stacksize;
    void *tofree, *vaddr;
    char *sptr;

    /* First, calculate the size we will need for the stack. */
    strspace = argc = envc = 0;
    while (argv[argc])
    {
        strspace += strlen(argv[argc]) + 1;
        ++argc;
    }
    while (env[envc])
    {
        strspace += strlen(env[envc]) + 1;
        ++envc;
    }
    stacksize = sizeof(unsigned long) * (1 + argc + 1 + envc + 1) +
        sizeof(char) * strspace;
    stacksize = ROUNDUP(stacksize, 4);

    /* The start of linux stack is not well defined (well, the value 0xc0000000
       is specific to my kernel configuration). */
    esp = STACK;
    /* Make room for argc, argv, env. Linux syscall exec aranges the stack in the
       following manner (which is different than the way "main" expects the args):

argc        <--- esp
argv[0]
argv[1]
...
argv[argc-1]
argv[argc](=NULL)
env[0]
...
env[envc-1]
env[envc](=NULL)
Argv/env strings    We copy the argv, env strings in the stack space here.
                    The pointers (argv[0], etc) on the stack above will point
                    to locations here.
...
...
...
NULL (the last string's null termination character) <--- this will be our 0xC0000000

     */
    esp -= stacksize;
    /* Do some alignment to make sure we can use dput to copy memory. */
    vaddr = malloc_aligned(ROUNDUP(stacksize, PAGE_SIZE), PAGE_SIZE, &tofree);
    stack = vaddr + ROUNDUP(stacksize, PAGE_SIZE) - stacksize;
    /* Compute the difference between the local "stack" memory we are creating
       and where the new stack will actually be in memory. */
    diff = esp - (unsigned long)stack;
    /* Fill the stack doing two things at once:
       1) Place the argv[i]/env[i] pointers on the stack.
       2) Simultaneously copy the strings above the argv/env pointers. */
    stack[0] = argc;
    sptr = (char*)&stack[1 + argc + 1 + envc + 1];
    for (i = 0; i < argc; ++i)
    {
        stack[i + 1] = (unsigned long)sptr + diff;
        sptr = stpcpy(sptr, argv[i]);
    }
    for (i = 0; i < envc; ++i)
    {
        stack[i + 1 + argc + 1] = (unsigned long)sptr + diff;
        sptr = stpcpy(sptr, env[i]);
    }
    /* Argv/env are null terminated arrays. */
    stack[1 + argc] = (unsigned long)NULL;
    stack[1 + argc + 1 + envc] = (unsigned long)NULL;
    if (0 > dput(childid, DETERMINE_VM_COPY, stack, stacksize, (void*)STACK-stacksize))
    {
        free(tofree);
        return -1;
    }
    free(tofree);
    return esp;
}

extern void _start(void);

int dexec(int childid, const char *file, char **argv, char **env)
{
    int fd, rc, i;
    uint8_t buf[512];
    Elf32_Ehdr *elf;
    Elf32_Phdr *ph;
    unsigned long esp;

    elf = (Elf32_Ehdr*)buf;
    fd = dfs_open(file, DFS_O_RDONLY);
    if (fd < 0)
        return -1;
    rc = dfs_read(fd, buf, sizeof(buf));
    if (sizeof(buf) != rc || 0 != strncmp((char*)elf->e_ident, ELFMAG, SELFMAG))
        goto fd;
    if ((unsigned long)_start != elf->e_entry)
        goto fd; /* We can only exec processes who have the same startup code we do. */

    /* Create child. */
    rc = dput(childid, DETERMINE_REGS | DETERMINE_CLEAR_CHILD, NULL, 0, NULL);
    if (rc < 0)
        goto fd;
    ph = (Elf32_Phdr*)(buf + elf->e_phoff);
    for (i = 0; i < elf->e_phnum; ++i, ++ph)
    {
        int prot = PROT_READ | PROT_WRITE;
        if (PT_LOAD != ph->p_type)
            continue;
        if (PF_X & ph->p_flags)
            prot |= PROT_EXEC;
        if (0 > map_segment(childid, ph->p_vaddr, ph->p_memsz, fd, ph->p_filesz,
                ph->p_offset, prot, ph->p_align))
            goto child;
    }
    dfs_close(fd);
    dfs_put(childid);

    esp = create_stack(childid, argv, env);
    if ((unsigned long)-1 == esp)
        goto child;
    _start_exec(childid, esp); /* Execution will not return here. */
    return 0; /* This really won't happen. */
child:
    // Kill child. TODO
fd:
    dfs_close(fd);
    return -1;
}

