#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include <stdint.h>
#include "devices/block.h" /* Include this header for block_sector_t */


struct buffer_block {
    int dirty;          /* flag for knowing if the block has been changed */
    int used;           /* flag for knowing if the block has been used yet */
    int accessed;       /* flag for knowing if the block has been accessed recently */
    block_sector_t sector;  /* on-disk location (sector number) of the block */
    void *vaddr;  /* virtual address of the associated buffer cache entry */
};

#endif /**< filesys/cache.h */