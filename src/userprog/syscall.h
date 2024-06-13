#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"
#include <debug.h>
#include <stdbool.h>

void syscall_init(void);

typedef int pid_t;

struct file_inst
{
  // set file pointer
  struct file * file_p;
  // file descriptor
  int fd;
  // list element
  struct list_elem file_list_e;
};

/** Projects 2 and later. */
struct lock file_lock;
struct lock process_lock;

void halt(void);
void exit(int status);
pid_t exec(const char *file);
int wait(pid_t);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned length);
int write(int fd, const void *buffer, unsigned length);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
// Directory functions 
bool mkdir(const char *dir);
bool readdir (int fd, char *name);
bool isdir(int fd);
int inumber(int fd);
bool chdir (const char *dir);
#endif /**< userprog/syscall.h */
