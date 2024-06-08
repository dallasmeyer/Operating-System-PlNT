#include "filesys/cache.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

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
