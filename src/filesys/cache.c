#include "filesys/cache.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"
#include "devices/block.h"
#include <string.h>
#include <stdio.h>

#define NUM_SECTORS 128 /* Number of sectors in the buffer cache */

/* Initialize cache_list and allocate memory for buffer cache entries */
void buffer_cache_init(void) {
    list_init(&cache_list);

    // Create the buffer cache 
    for (int i = 0; i < NUM_SECTORS; i++) {
        // Create a buffer block entry
        struct buffer_block *entry = malloc(sizeof(struct buffer_block));
        if (entry == NULL) {
            PANIC("Failed to allocate memory for buffer cache entry");
        }
        // Allocate BLOCK_SECTOR_SIZE bytes for the block data 
        entry->vaddr = malloc(BLOCK_SECTOR_SIZE);  
        if(entry->vaddr == NULL) {
            PANIC("Failed to allocate memory for buffer cache data");
        }
        // Fill the initial values
        entry->sector = -1;  /* Initialize sector to an invalid value */
        entry->dirty = 0;
        entry->used = 0;
        entry->accessed = 0;
        // Add to the list
        list_push_back(&cache_list, &entry->elem);
    }
}
//-------------------------------------------------//
/* buffer list operation functions                 */
//-------------------------------------------------//
/* Helper function to find a buffer block in the cache */
struct buffer_block *buffer_cache_find(block_sector_t sector) {
    struct list_elem *e;
    for (e = list_begin(&cache_list); e != list_end(&cache_list); e = list_next(e)) {
        struct buffer_block *entry = list_entry(e, struct buffer_block, elem);
        if (entry->sector == sector) {
            entry->accessed = 1;
            return entry;
        }
    }
    return NULL;
}

/* Helper function to evict the least recently used block from the cache */
struct buffer_block *buffer_cache_evict(void) {
    struct list_elem *e = list_begin(&cache_list);
    struct buffer_block *evict_entry = list_entry(e, struct buffer_block, elem);

    for (e = list_next(e); e != list_end(&cache_list); e = list_next(e)) {
        struct buffer_block *entry = list_entry(e, struct buffer_block, elem);
        if (!entry->accessed) {
            evict_entry = entry;
            break;
        }
        entry->accessed = 0;
    }

    if (evict_entry->dirty) {
        block_write(fs_device, evict_entry->sector, evict_entry->vaddr);
        evict_entry->dirty = 0;
    }

    return evict_entry;
}

//-------------------------------------------------//
/* buffer cache block operation functiosn          */
//-------------------------------------------------//

/* Read a block from the buffer cache or disk */
void buffer_cache_read(block_sector_t sector, void *buffer) {
    ASSERT(fs_device != NULL); 
    struct buffer_block *entry = buffer_cache_find(sector);
    if (entry == NULL) {
        entry = buffer_cache_evict();
        block_read(fs_device, sector, entry->vaddr);
        entry->sector = sector;
        entry->used = 1;
        entry->accessed = 1;
    }
    memcpy(buffer, entry->vaddr, BLOCK_SECTOR_SIZE);
}
/* Write a block to the buffer cache */
void buffer_cache_write(block_sector_t sector, const void *buffer) {
    ASSERT(fs_device != NULL);  
    struct buffer_block *entry = buffer_cache_find(sector);
    if (entry == NULL) {
        entry = buffer_cache_evict();
        block_read(fs_device, sector, entry->vaddr);
        entry->sector = sector;
        entry->used = 1;
        entry->accessed = 1;
    }
    memcpy(entry->vaddr, buffer, BLOCK_SECTOR_SIZE);
    entry->dirty = 1;
}

/* Flush all dirty blocks to disk */
void buffer_cache_flush(void) {
    struct list_elem *e;
    for (e = list_begin(&cache_list); e != list_end(&cache_list); e = list_next(e)) {
        struct buffer_block *entry = list_entry(e, struct buffer_block, elem);
        if (entry->dirty) {
            block_write(fs_device, entry->sector, entry->vaddr);
            entry->dirty = 0;
        }
    }
}
