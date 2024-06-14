#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "filesys/directory.h" // Just for NAME_MAX

/** Sectors of system file inodes. */
#define FREE_MAP_SECTOR 0       /**< Free map file inode sector. */
#define ROOT_DIR_SECTOR 1       /**< Root directory file inode sector. */

/** Block device that contains the file system. */
struct block *fs_device;


void filesys_init (bool format);
void filesys_done (void);
bool filesys_create (const char *name, off_t initial_size, int is_dir); 
struct file *filesys_open (const char *name);
bool filesys_remove (const char *name);

// directory helper functions
bool split_path (const char *path, char *dir, char *base);
struct dir *dir_open_path (const char *path);
static bool get_next_part (char part[NAME_MAX + 1], const char **srcp);
#endif /**< filesys/filesys.h */
