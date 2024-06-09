#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include <stdint.h>
#include "devices/block.h" /* Include this header for block_sector_t */
#include "lib/kernel/list.h" /* Include Pintos list header */
#include "threads/synch.h"

struct buffer_block {
    int dirty;          /* flag for knowing if the block has been changed */
    int used;           /* flag for knowing if the block has been used yet */
    int accessed;       /* flag for knowing if the block has been accessed recently */
    block_sector_t sector;  /* on-disk location (sector number) of the block */
    void *vaddr;  /* virtual address of the associated buffer cache entry */
    struct list_elem elem;  /* List element for inclusion in cache_list */
};

/* Declare the global cache list */
struct list cache_list;

/* Function to initialize the buffer cache */
void buffer_cache_init(void);

/* A lock for synchronizing buffer cache operations. */
static struct lock buffer_cache_lock;

/* Helper function to find a buffer block in the cache */
struct buffer_block *buffer_cache_find(block_sector_t sector);
/* Read a block from the buffer cache or disk */
void buffer_cache_read(block_sector_t sector, void *target, int sector_ofs, int chunk_size);
/* Write a block to the buffer cache */
void buffer_cache_write(block_sector_t sector, const void *source, int sector_ofs, int chunk_size);
/* Close the buffer cache, flushing all dirty entries to disk. this is a write-back method */
void buffer_cache_close(void);
#endif /* filesys/cache.h */
