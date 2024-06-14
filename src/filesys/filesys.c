#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "filesys/cache.h"
#include "threads/thread.h"

/** Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/** Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();
  
  /* NEW: Initialize cache_list */
  buffer_cache_init();

  // Initalize the root directory
  if (format) 
    do_format ();


  free_map_open ();
}

/** Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  /* Flush all dirty blocks to disk */
  void buffer_cache_close();
  free_map_close ();
}
/** Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size, int is_dir) 
{
  block_sector_t inode_sector = 0;
  char *dir_name = malloc(strlen(name) + 1);
  char *base_name = malloc(strlen(name) + 1);

  // split the path into directory and base names
  if (!split_path(name, dir_name, base_name)) {
    free(dir_name);
    free(base_name);
    return false;
  }

  struct dir *dir = dir_open_path(dir_name);
  bool success = (dir != NULL
                  && free_map_allocate(1, &inode_sector)
                  && inode_create(inode_sector, initial_size, is_dir)
                  && dir_add(dir, base_name, inode_sector, is_dir));
  if (!success && inode_sector != 0) 
    free_map_release(inode_sector, 1);
  dir_close(dir);
  free(dir_name);
  free(base_name);

  return success;
}

/** Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  char *dir_name = malloc(strlen(name) + 1);
  char *base_name = malloc(strlen(name) + 1);

  if (!split_path(name, dir_name, base_name)) {
    free(dir_name);
    free(base_name);
    return NULL;
  }

  struct dir *dir = dir_open_path(dir_name);
  struct inode *inode = NULL;

  if (dir != NULL) {
    dir_lookup(dir, base_name, &inode);
    dir_close(dir);
  }

  free(dir_name);
  free(base_name);

  return file_open(inode);
}
/** Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  char *dir_name = malloc(strlen(name) + 1);
  char *base_name = malloc(strlen(name) + 1);

  if (!split_path(name, dir_name, base_name)) {
    free(dir_name);
    free(base_name);
    return false;
  }

  //printf("(filesys_remove) dir_name[%s] | base_name[%s] \n", dir_name, base_name);
  struct dir *dir = dir_open_path(dir_name);
  bool success = false;

  if (dir != NULL) {
    success = dir_remove(dir, base_name);
    dir_close(dir);
  }

  free(dir_name);
  free(base_name);

  return success;
}

/** Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}

//--------------------
// New helper functions 
//------------------------
/** Splits a path into directory and base name components.
   Returns true if successful, false otherwise. */
bool
split_path (const char *path, char *dir, char *base)
{
  if (path[0] == '\0') return false;
  
  char *last_slash = strrchr(path, '/');
  if (last_slash == NULL) {
    strlcpy(base, path, NAME_MAX + 1);
    dir[0] = '\0';
  } else {
    size_t dir_len = last_slash - path;
    if (dir_len > NAME_MAX) return false;
    memcpy(dir, path, dir_len);
    dir[dir_len] = '\0';
    strlcpy(base, last_slash + 1, NAME_MAX + 1);
  }
  return true;
}


/* Opens the directory for the given path */
struct dir *dir_open_path(const char *path) {
  // Copy of path to tokenize
  char s[strlen(path) + 1];
  strlcpy(s, path, sizeof(s));

  // Determine starting directory based on whether the path is absolute or relative
  struct dir *curr = (path[0] == '/') ? dir_open_root() : 
                     (thread_current()->cwd ? dir_reopen(thread_current()->cwd) : dir_open_root());
  
  if (curr == NULL) return NULL;

  // Tokenize and traverse the path
  char *token, *save_ptr;
  for (token = strtok_r(s, "/", &save_ptr); token != NULL; token = strtok_r(NULL, "/", &save_ptr)) {
    struct inode *inode = NULL;
    
    // Check if the directory does not exist
    if (!dir_lookup(curr, token, &inode)) {  
      dir_close(curr);
      return NULL;
    }
    
    struct dir *next = dir_open(inode);
    if (next == NULL) { 
       // Failed to open next directory, close
      dir_close(curr);
      return NULL;
    }
    
    dir_close(curr);
    curr = next;
  }

  // Check that the directory has not been moved
  if (inode_is_removed(dir_get_inode(curr))) {
    dir_close(curr);
    return NULL;
  }

  return curr;
}
/** Extracts the next part of the path. */
static bool
get_next_part (char part[NAME_MAX + 1], const char **srcp)
{
  const char *src = *srcp;
  char *dst = part;

  /* Skip leading slashes. If it’s all slashes, we’re done. */
  while (*src == '/')
    src++;
  if (*src == '\0')
    return false;

  /* Copy up to NAME_MAX characters from SRC to DST. Add null terminator. */
  while (*src != '/' && *src != '\0') {
    if (dst < part + NAME_MAX)
      *dst++ = *src;
    src++;
  }
  *dst = '\0';

  /* Advance source pointer. */
  *srcp = src;
  return true;
}