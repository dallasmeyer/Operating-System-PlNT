#include "filesys/cache.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"
#include "devices/block.h"
#include <string.h>
#include <stdio.h>

#define NUM_SECTORS 128 /* Number of sectors in the buffer cache */

/* Flush all dirty blocks to disk */
static void buffer_cache_flush(struct buffer_block *entry);

static struct buffer_block* buffer_cache_evict(void);



/* Initialize cache_list and allocate memory for buffer cache entries */
void buffer_cache_init(void) {
    // Initalize the lock and buffer cache list
    list_init(&cache_list);
    lock_init(&buffer_cache_lock);

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
/* buffer cache list operation functions           */
//-------------------------------------------------//
/* Helper function to find a buffer block in the cache */
struct buffer_block *buffer_cache_find(block_sector_t sector) {
    struct list_elem *e;
    // Use a for loop to iterate through the buffer cache list to try and find the block
    for (e = list_begin(&cache_list); e != list_end(&cache_list); e = list_next(e)) {
        struct buffer_block *entry = list_entry(e, struct buffer_block, elem);
        if (entry->sector == sector) {
            return entry;
        }
    }
    return NULL; // Cache miss
}

/* Helper function to evict the least recently used block from the cache */
static struct buffer_block* buffer_cache_evict(void) {
    ASSERT(lock_held_by_current_thread(&buffer_cache_lock));
    // Grab the first buffered block element in the list 
    struct list_elem *e = list_begin(&cache_list);
    struct buffer_block *evict_entry = list_entry(e, struct buffer_block, elem);
    // Try to find an used block to evict
    while (true) {
        // If the block is dirty, flush it to the disk
        if (!evict_entry->used) {
            if (evict_entry->dirty) {
                buffer_cache_flush(evict_entry);
            }
            // Change the sector number to -1 indicate the block is free
            evict_entry->sector = (block_sector_t)-1;
            return evict_entry;
        }
        // Mark the block as unused for eviction at a later date
        evict_entry->used = 0;
        // Move onto the next block in the list
        e = list_next(e);
        if (e == list_end(&cache_list)) {
            // If the end of the list is reached, wrap back aroudn
            e = list_begin(&cache_list);
        }
        evict_entry = list_entry(e, struct buffer_block, elem);
    }
}

/* Close the buffer cache, flushing all dirty entries to disk. this is a write-back method */
void buffer_cache_close(void) {
    lock_acquire(&buffer_cache_lock);
    // For loop writing all used sectors back to the disk
    printf("(buffer_cache_close) starting flusing to disk\n");
    struct list_elem *e;
    for (e = list_begin(&cache_list); e != list_end(&cache_list); e = list_next(e)) {
        struct buffer_block *entry = list_entry(e, struct buffer_block, elem);
        if (entry->sector != (block_sector_t)-1) {
            buffer_cache_flush(entry);
        }
    }
    printf("(buffer_cache_close) finished flusing to disk\n");
    lock_release(&buffer_cache_lock);
}
//-------------------------------------------------//
/* buffer cache: block operation functions          */
//-------------------------------------------------//

/* Read a block from the buffer cache or disk into a specified memory location. */
void buffer_cache_read(block_sector_t sector, void *target, int sector_ofs, int chunk_size) {
    lock_acquire(&buffer_cache_lock);
    printf("(buffer_cache_read) starting\n");
    // Check if the block we want is in the buffer cache
    struct buffer_block *entry = buffer_cache_find(sector);
    if (entry == NULL) {
        // Cache miss: the block was not found so we need to evict based on LRU
        printf("   (buffer_cache_read) cache miss\n");
        entry = buffer_cache_evict();
        ASSERT(entry != NULL);
        // Initialize the buffer cache block
        block_read(fs_device, sector, entry->buf);
        entry->sector = sector;
        entry->dirty = 0;
    } else {
        printf("  (buffer_cache_read) CACHE HIT\n");
    }
    // Change the access and used flags of the buffer cache block
    entry->used = 1;
    entry->accessed = 1;

    // Perform the read operation
    memcpy(target, entry->buf + sector_ofs, chunk_size);
    printf("(buffer_cache_read) finished\n");
    lock_release(&buffer_cache_lock);
}

/* Write a block to the buffer cache */
void buffer_cache_write(block_sector_t sector, const void *source, int sector_ofs, int chunk_size) {
    lock_acquire(&buffer_cache_lock);  // Ensures exclusive access to the buffer cache.
    printf("(buffer_cache_write) attempting to write to sector: %d, offset: %d, size: %d\n", sector, sector_ofs, chunk_size);
    struct buffer_block *entry = buffer_cache_find(sector);
    if (entry == NULL) {
        // Cache miss: need eviction
        printf("    (buffer_cache_write) cache miss\n");
        entry = buffer_cache_evict();  // Evict a cache block if necessary.
        ASSERT(entry != NULL);

        // Read the sector data into the cache block only if necessary.
        if (!entry->used || entry->sector != sector) {
            block_read(fs_device, sector, entry->vaddr);  // Load the sector data into the cache.
            entry->sector = sector;
            entry->dirty = 0;  // Initially not dirty because we just loaded it.
        }
    }
    else{ printf("  (buffer_cache_write) CACHE HIT\n");} // Comment this out later

    // Mark the entry as used, accessed, and dirty since it's being modified.
    entry->used = 1;
    entry->accessed = 1;
    entry->dirty = 1;

    // Perform the write operation to the buffer, not the disk.
    memcpy(entry->buf + sector_ofs, source, chunk_size);
    printf("(buffer_cache_write) finished\n");
    lock_release(&buffer_cache_lock);  // Release the lock after the operation.
}

/* Flush all dirty blocks to disk */
static void buffer_cache_flush(struct buffer_block *entry){
    ASSERT(lock_held_by_current_thread(&buffer_cache_lock));
    if (entry->dirty) {
        block_write(fs_device, entry->sector, entry->vaddr);
        entry->dirty = 0;
    }
}

