#include "filesys/cache.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

void buffer_cache_init(void) {
    list_init(&cache_list);

    for (int i = 0; i < 64; i++) {
        struct buffer_block *entry = malloc(sizeof(struct buffer_block));
        if (entry == NULL) {
            PANIC("Failed to allocate memory for buffer cache entry");
        }

        entry->vaddr = malloc(512);  /* Allocate 512 bytes for the block data */
        if(entry->vaddr == NULL) {
            PANIC("Failed to allocate memory for buffer cache data");
        }

        entry->sector = -1;  /* Initialize sector to an invalid value */
        entry->dirty = 0;
        entry->used = 0;
        entry->accessed = 0;

        list_push_back(&cache_list, &entry->elem);
    }
}
