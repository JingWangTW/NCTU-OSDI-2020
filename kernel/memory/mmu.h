#ifndef __SYS_MMU_H
#define __SYS_MMU_H

// to set up tcr_el1 - Translation Control Register
// https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/tcr_el1
// bits[31:30] - Granule size for the TTBR1_EL1. 0b10, 4KB
// bits[21:16] - T1SZ - The size offset of the memory region addressed by TTBR1_EL1.
// bits[5:0] - T0SZ - The size offset of the memory region addressed by TTBR0_EL1.
#define TCR_T0SZ           ( ( 64 - 48 ) << 0 )   // bis[5:0]
#define TCR_T1SZ           ( ( 64 - 48 ) << 16 )  // bits[16:21]
#define TCR_TG1_4KB        ( 0b10 << 30 )         // bits[30:31]
#define TCR_TG0_4KB        ( 0b00 << 14 )         // bits[14:15]
#define TCR_CONFIG_DEFAULT ( TCR_T0SZ | TCR_T1SZ | TCR_TG1_4KB | TCR_TG0_4KB )

// to set up mair_el1
// set up mair_el1 - Memory attribute indirection register
// mair_el1: consists of 8 sections, each negin 8bits long.
// Each section configures a common set of attrs.
// Descriptor then specifies an index of mair_el1 section.
// https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/mair_el1
// https://static.docs.arm.com/ddi0487/fb/DDI0487F_b_armv8_arm.pdf?_ga=2.180814439.1366931515.1589369038-999209164.1583919977 p.3335
#define MAIR_DEVICE_nGnRnE        0b00000000  // 0b0000dd00
#define MAIR_NORMAL_NOCACHE       0b01000100  // 0booooiiii
#define MAIR_DEVICE_nGnRnE_INDEX  0
#define MAIR_NORMAL_NOCACHE_IDNEX 1
#define MAIR_CONFIG_DEFAULT       ( ( MAIR_DEVICE_nGnRnE << ( MAIR_DEVICE_nGnRnE_INDEX * 8 ) ) | ( MAIR_NORMAL_NOCACHE << ( MAIR_NORMAL_NOCACHE_IDNEX * 8 ) ) )

// to set up PGD PUD attribute
#define PAGE_DESCRIPTOR_INVALID                ( 0b0 << 0 )
#define PAGE_DESCRIPTOR_VALID                  ( 0b1 << 0 )
#define PAGE_DESCRIPTOR_BLOCK                  ( 0b0 << 1 )
#define PAGE_DESCRIPTOR_TABLE                  ( 0b1 << 1 )
#define PAGE_DESCROPTOR_USER_ACCESS_PERMISSION ( 0b1 << 6 )
#define PAGE_DESCRIPTOR_ACCESS                 ( 0b1 << 10 )

#define SYS_PGD_ATTR        ( PAGE_DESCRIPTOR_TABLE | PAGE_DESCRIPTOR_VALID )
#define SYS_PUD_ATTR        ( PAGE_DESCRIPTOR_TABLE | PAGE_DESCRIPTOR_VALID )
#define SYS_PMD_ATTR_NORMAL ( PAGE_DESCRIPTOR_BLOCK | PAGE_DESCRIPTOR_VALID | PAGE_DESCRIPTOR_ACCESS | ( MAIR_NORMAL_NOCACHE_IDNEX << 2 ) )
#define SYS_PMD_ATTR_DEVICE ( PAGE_DESCRIPTOR_BLOCK | PAGE_DESCRIPTOR_VALID | PAGE_DESCRIPTOR_ACCESS | ( MAIR_DEVICE_nGnRnE_INDEX << 2 ) )

#define USER_PGD_ATTR ( PAGE_DESCRIPTOR_TABLE | PAGE_DESCRIPTOR_VALID | PAGE_DESCRIPTOR_ACCESS )
#define USER_PUD_ATTR ( PAGE_DESCRIPTOR_TABLE | PAGE_DESCRIPTOR_VALID | PAGE_DESCRIPTOR_ACCESS )
#define USER_PMD_ATTR ( PAGE_DESCRIPTOR_TABLE | PAGE_DESCRIPTOR_VALID | PAGE_DESCRIPTOR_ACCESS )
#define USER_PTE_ATTR ( PAGE_DESCRIPTOR_TABLE | PAGE_DESCRIPTOR_VALID | PAGE_DESCRIPTOR_ACCESS | ( MAIR_NORMAL_NOCACHE_IDNEX << 2 ) | PAGE_DESCROPTOR_USER_ACCESS_PERMISSION )

// define sizes
#define SYS_VA_START     0xffff000000000000
#define USER_VA_START    0x0000000000000000
#define PHYS_MEMORY_SIZE 0x40000000
#define DEVICE_BASE      0x3F000000

#define TABLE_BITS     9
#define PAGE_OFFSET    12
#define SECTION_OFFSET ( PAGE_OFFSET + TABLE_BITS )

#define TABLE_SIZE    ( 0b1 << TABLE_BITS )
#define PAGE_SIZE     ( 0b1 << PAGE_OFFSET )
#define SECTION_SIZE  ( 0b1 << SECTION_OFFSET )
#define PAGE_DIR_SIZE ( 3 * PAGE_SIZE )

#define HIGH_MEMORY ( DEVICE_BASE - SECTION_SIZE ) /* a section to place pcb */
#define LOW_MEMORY  ( 2 * SECTION_SIZE )

#define HEIGH_MEMORY_PAGE_NUMS ( ( PHYS_MEMORY_SIZE - HIGH_MEMORY ) / ( 0b1 << PAGE_OFFSET ) )
#define LOW_MEMORY_PAGE_NUMS   ( LOW_MEMORY / ( 0b1 << PAGE_OFFSET ) )

#endif