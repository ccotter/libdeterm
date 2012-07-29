

/* From mm/memory.c, we could probably move methods that rely on these into mm/memory.c TODO */
extern unsigned long zero_pfn __read_mostly;
#ifndef is_zero_pfn
static inline int is_zero_pfn(unsigned long pfn)
{
    return pfn == zero_pfn;
}
#endif

#ifndef my_zero_pfn
static inline unsigned long my_zero_pfn(unsigned long addr)
{
    return zero_pfn;
}
#endif

_STATIC_F_ int zero_pte_range(struct mm_struct *mm, struct vm_area_struct *vma, pmd_t *pmd, unsigned long addr, unsigned long end)
{
    pte_t *pte, *orig;
    spinlock_t *ptl;
    int progress = 0;
    pte_t entry;

again:

    orig = pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
    arch_enter_lazy_mmu_mode();

    do {
        if (progress >= 32)
        {
            progress = 0;
            if (need_resched() || spin_needbreak(ptl))
                break;
        }

        /* TODO supports only anonymous mappings afaik (what needs to be done to support file mappings?) */
        entry = pte_wrprotect(pte_mkspecial(pfn_pte(my_zero_pfn(addr), vma->vm_page_prot)));
        zap_pte(mm, vma, addr, pte);
        set_pte_at(mm, addr, pte, entry);
        update_mmu_cache(vma, addr, entry);
        
    } while (++pte, addr += PAGE_SIZE, addr != end);

    arch_leave_lazy_mmu_mode();
    pte_unmap_unlock(orig, ptl);
    cond_resched();
    if (addr != end)
        goto again;

    return 0;
}

_STATIC_F_ int zero_pmd_range(struct mm_struct *mm, struct vm_area_struct *vma, pud_t *pud, unsigned long addr, unsigned long end)
{
    unsigned long next;
    pmd_t *pmd;
    pmd = pmd_offset(pud, addr);

    do
    {
        next = pmd_addr_end(addr, end);
        if (pmd_none_or_clear_bad(pmd))
            continue;
        if (zero_pte_range(mm, vma, pmd, addr, next))
            return -ENOMEM;
    } while (++pmd, addr = next, addr != end);

    return 0;
}

_STATIC_F_ int zero_pud_range(struct mm_struct *mm, struct vm_area_struct *vma, pgd_t *pgd, unsigned long addr, unsigned long end)
{
    unsigned long next;
    pud_t *pud;
    pud = pud_offset(pgd, addr);

    do
    {
        if (pud_none_or_clear_bad(pud))
            continue;
        next = pud_addr_end(addr, end);
        if (zero_pmd_range(mm, vma, pud, addr, next))
            return -ENOMEM;
    } while (++pud, addr = next, addr != end);

    return 0;
}

    /* [start, end) contains one non whole page. */
    if (end - addr < PAGE_SIZE)
    {
        ret = manuallyZero(tsk, mm, addr, end, prot_flags, 1);
        goto unlock;
    }

    start_page = PAGE_ALIGN(addr);
    last_page = LOWER_PAGE(end);

    /* [start, end) covers two non whole pages. */
    if (last_page == start_page)
    {
        if ((ret = manuallyZero(tsk, mm, addr, start_page, prot_flags, 1)))
            goto unlock;
        ret = manuallyZero(tsk, mm, last_page, end, prot_flags, 1);
        goto unlock;
    }

    /* [addr, end) contains at least one whole page. */

    prev = NULL;
    vma = find_vma(mm, addr);
    if (!vma || (vma->vm_end > end))
    {
        /* TODO the entire region [start, end) is not mapped! Create the entire mapping. */
        addr = LOWER_PAGE(addr);
        end = PAGE_ALIGN(end);
        //ret = simple_map(mm, addr, end - addr, prot_flags);
        ret = do_brk_gen(tsk, addr, end - addr);
        if (ret == addr)
            ret = 0; /* Success. */
        goto unlock;
    }
    /* At this point, vma points to the VMA containing a subset of [start, end)
       that we are to zero out. This loop with run at least once. */
    ret = -EINVAL;
    while (vma && (vma->vm_start < end))
    {
        pgd_t *pgd;
        unsigned long local_end;

        if (vma->vm_start > addr)
        {
            /* Map memory up to vma->vm_start. */
            unsigned long len, aligned;
            aligned = LOWER_PAGE(addr);
            if (vma->vm_start > end)
                len = end - aligned;
            else
                len = vma->vm_start - aligned;
            //ret = simple_map(mm, aligned, len, prot_flags);
            ret = do_brk_gen(tsk, aligned, len);
            if (ret != aligned)
                goto unlock;
        }

        if (vma->vm_file)
        {
            ret = -EINVAL;
            goto unlock;
        }

        /* Continue to zero out memory by altering page table entries. */
        local_end = end < vma->vm_end ? end : vma->vm_end;
        pgd = pgd_offset(mm, addr);
        do
        {
            next = pgd_addr_end(addr, local_end);
            if (pgd_none_or_clear_bad(pgd))
                continue;
            if (zero_pud_range(mm, vma, pgd, addr, next))
            {
                ret = -ENOMEM;
                goto unlock;
            }
        } while (++pgd, addr = next, addr != local_end);
        prev = vma;
        vma = vma->vm_next;
    }
    ret = 0;
    /* It could be the case that we finished this loop but did not zero the last part of the
       memory region because it wasn't even mapped. So we map it now. */
    if (!vma || (prev->vm_end < end)) /* prev will not be null because the while loop above runs at least once. */
    {
        //ret = simple_map(mm, prev->vm_end, PAGE_ALIGN(end), prot_flags);
        ret = do_brk_gen(tsk, prev->vm_end, PAGE_ALIGN(end));
        if (ret == prev->vm_end)
            ret = 0;
    }

unlock:
    up_write(&mm->mmap_sem);
    mmput(mm);

    return ret;
