#ifndef __SYS_PAGE_H
#define __SYS_PAGE_H

#include "lib/type.h"

#include "mmu.h"

#define TOTAL_PAGE_FRAME ( ( HIGH_MEMORY - LOW_MEMORY ) / ( 0b1 << PAGE_OFFSET ) )
#define MAX_PGD_NUMS     64

typedef struct
{
    int index;
    // how many entries are used in these tables
    int pgd_used;
    int pud_used;
    int pmd_used;
    int pte_used;
    intptr_t * pgd;
    intptr_t * pud;
    intptr_t * pmd;
    intptr_t * pte;

} page_t;

page_t * user_pgd_alloc ( );
void user_pgd_free ( page_t * pgd_rec );
void * user_page_alloc ( page_t * pgd_rec );
void user_page_free ( page_t * pgd_rec, intptr_t va );
intptr_t user_va_to_phy_addr ( page_t * pgd_rec, intptr_t va );

#endif