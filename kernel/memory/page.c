#include "page.h"

#include "lib/math.h"
#include "lib/type.h"

#include "mem.h"
#include "mmu.h"

int8_t PAGE_POOL[TOTAL_PAGE_FRAME] = { 0 };  // global array to record the usage of the page frame
page_t PGD_POOL[MAX_PGD_NUMS]      = { 0 };  // global array to record the pgd record, not the physical table
uint64_t PGD_POOL_USAGE            = 0;      // usage of pgd pool

// only be used in this file
int user_page_occupied ( );
int user_phy_addr_to_pfn ( intptr_t phy_addr );
intptr_t user_pfn_to_phy_addr ( int pfn );

page_t * user_pgd_alloc ( )
{
    int pgd_rec_idx = find_first_0_in_bit ( PGD_POOL_USAGE );
    PGD_POOL_USAGE |= ( 0b1 << pgd_rec_idx );

    page_t * pgd_rec = &PGD_POOL[pgd_rec_idx];

    pgd_rec->index    = pgd_rec_idx;
    pgd_rec->pgd_used = pgd_rec->pud_used = pgd_rec->pmd_used = 1;
    pgd_rec->pte_used                                         = 0;

    pgd_rec->pgd = (intptr_t *) user_pfn_to_phy_addr ( user_page_occupied ( ) );
    pgd_rec->pud = (intptr_t *) user_pfn_to_phy_addr ( user_page_occupied ( ) );
    pgd_rec->pmd = (intptr_t *) user_pfn_to_phy_addr ( user_page_occupied ( ) );
    pgd_rec->pte = (intptr_t *) user_pfn_to_phy_addr ( user_page_occupied ( ) );

    ( pgd_rec->pgd )[0] = ( (intptr_t) pgd_rec->pud ) | USER_PGD_ATTR;
    ( pgd_rec->pud )[0] = ( (intptr_t) pgd_rec->pmd ) | USER_PUD_ATTR;
    ( pgd_rec->pmd )[0] = ( (intptr_t) pgd_rec->pte ) | USER_PMD_ATTR;

    return pgd_rec;
}

void user_pgd_free ( page_t * pgd_rec )
{
    int i, j, k, l;
    // traversal pgd table
    for ( i = 0; i < pgd_rec->pgd_used; i++ )
    {
        // traversal pud table
        for ( j = 0; j < pgd_rec->pud_used; j++ )
        {
            // traversal pmd table
            for ( k = 0; k < pgd_rec->pmd_used; k++ )
            {
                // traversal pte table
                for ( l = 0; l < pgd_rec->pte_used; l++ )
                {
                    // release page pointed by entrey of pte
                    user_page_free ( pgd_rec, pgd_rec->pte[l] );
                }

                // release pte
                PAGE_POOL[user_phy_addr_to_pfn ( ( pgd_rec->pmd )[k] )] = 0;
            }

            // release pud
            PAGE_POOL[user_phy_addr_to_pfn ( ( pgd_rec->pud )[j] )] = 0;
        }

        // release pmd
        PAGE_POOL[user_phy_addr_to_pfn ( ( pgd_rec->pgd )[j] )] = 0;
    }

    // release pgd
    PAGE_POOL[user_phy_addr_to_pfn ( ( intptr_t ) ( pgd_rec->pgd ) )] = 0;

    // release pgd table foe next thread to used
    PGD_POOL_USAGE &= ~( 1UL << ( pgd_rec->index ) );
}

void * user_page_alloc ( page_t * pgd_rec )
{
    int pfn;
    int phy_page_base;
    int phy_page_offset;
    intptr_t va = 0;
    intptr_t phy_addr;

    pfn = user_page_occupied ( );

    if ( pfn < 0 )
        return NULL;

    phy_addr = user_pfn_to_phy_addr ( pfn );

    phy_page_base   = ( ( phy_addr >> PAGE_OFFSET ) << PAGE_OFFSET );
    phy_page_offset = phy_addr - phy_page_base;

    // updat the used of pte
    ( pgd_rec->pte_used )++;

    // count the va
    va |= ( ( ( uint64_t ) ( pgd_rec->pgd_used - 1 ) ) << ( PAGE_OFFSET + TABLE_BITS * 3 ) );
    va |= ( ( ( uint64_t ) ( pgd_rec->pud_used - 1 ) ) << ( PAGE_OFFSET + TABLE_BITS * 2 ) );
    va |= ( ( ( uint64_t ) ( pgd_rec->pmd_used - 1 ) ) << ( PAGE_OFFSET + TABLE_BITS * 1 ) );
    va |= ( ( ( uint64_t ) ( pgd_rec->pte_used - 1 ) ) << ( PAGE_OFFSET + TABLE_BITS * 0 ) );
    va <<= phy_page_offset;
    va |= USER_VA_START;

    // update pte table
    ( pgd_rec->pte )[pgd_rec->pte_used - 1] = ( phy_page_base | USER_PTE_ATTR );

    memzero ( (uint64_t *) ( phy_page_base | SYS_VA_START ), PAGE_SIZE );

    return (void *) va;
}

void user_page_free ( page_t * pgd_rec, intptr_t va )
{
    intptr_t phy_addr = user_va_to_phy_addr ( pgd_rec, va );
    int pfn           = user_phy_addr_to_pfn ( phy_addr );

    int pgd_idx = ( va >> ( PAGE_OFFSET + TABLE_BITS * 3 ) ) & ( TABLE_SIZE - 1 );
    int pud_idx = ( va >> ( PAGE_OFFSET + TABLE_BITS * 2 ) ) & ( TABLE_SIZE - 1 );
    int pmd_idx = ( va >> ( PAGE_OFFSET + TABLE_BITS * 1 ) ) & ( TABLE_SIZE - 1 );
    int pte_idx = ( va >> ( PAGE_OFFSET + TABLE_BITS * 0 ) ) & ( TABLE_SIZE - 1 );

    // page pool release
    PAGE_POOL[pfn] = 0;

    // disable in pte table, reset the last bit
    ( (intptr_t *) ( ( (intptr_t *) ( ( (intptr_t *) ( pgd_rec->pgd[pgd_idx] ) )[pud_idx] ) )[pmd_idx] ) )[pte_idx] &= ( ~( 1UL ) );
}

intptr_t user_va_to_phy_addr ( page_t * pgd_rec, intptr_t va )
{
    int pgd_idx     = ( va >> ( PAGE_OFFSET + TABLE_BITS * 3 ) ) & ( TABLE_SIZE - 1 );
    int pud_idx     = ( va >> ( PAGE_OFFSET + TABLE_BITS * 2 ) ) & ( TABLE_SIZE - 1 );
    int pmd_idx     = ( va >> ( PAGE_OFFSET + TABLE_BITS * 1 ) ) & ( TABLE_SIZE - 1 );
    int pte_idx     = ( va >> ( PAGE_OFFSET + TABLE_BITS * 0 ) ) & ( TABLE_SIZE - 1 );
    int page_offset = va & ( PAGE_SIZE - 1 );

    return ( (intptr_t *) ( ( (intptr_t *) ( ( (intptr_t *) ( pgd_rec->pgd[pgd_idx] ) )[pud_idx] ) )[pmd_idx] ) )[pte_idx] + page_offset;
}

int user_phy_addr_to_pfn ( intptr_t phy_addr )
{
    return ( phy_addr - LOW_MEMORY ) / PAGE_SIZE;
}

intptr_t user_pfn_to_phy_addr ( int pfn )
{
    return ( pfn * PAGE_SIZE + LOW_MEMORY );
}

int user_page_occupied ( )
{
    int pfn;

    for ( pfn = 0; pfn < TOTAL_PAGE_FRAME; pfn++ )
    {
        if ( !PAGE_POOL[pfn] )
        {
            PAGE_POOL[pfn] = 1;
            return pfn;
        }
    }

    return -1;
}