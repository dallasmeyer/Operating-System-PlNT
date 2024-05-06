#include "userprog/syscall.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include <stdio.h>
#include <syscall-nr.h>

static void syscall_handler(struct intr_frame *);

// declaring the system calls
// void halt(void);
// void exit(int status);
// pid_t exec(const char *cmd_line);
// int wait(pid_t pid);
// bool create(const char *file, unsigned initial_size);
// bool remove(const char *file);
// int open(const char *file);
// int filesize(int fd);
// int read(int fd, void *buffer, unsigned size);
// int write(int fd, const void *buffer, unsigned size);
// void seek(int fd, unsigned position);
// unsigned tell(int fd);
// void close(int fd);

// structs
struct lock file_lock;

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

static void syscall_handler(struct intr_frame *f UNUSED) {
  // TODO: implement system call handler

  printf("system call!\n");
  thread_exit();

  // Get stack pointer via "esp" of intr_frame
  int *stack_p = f->esp;

  // check if virtual address
  if (!is_user_vaddr(stack_p)) {
    printf("syscall_handler(): Not a virtual address!");
  }

  // returned values go into "eax" register
}

void halt(void) {
  // Terminates PintOS
  shutdown_power_off();
}

void exit(int status) {
  // Terminates current user program
  // TODO:
}

pid_t exec(const char *cmd_line) {
  // Runs executable
  // TODO:
  return 0;
}

int wait(pid_t pid) {
  // Waits for child "pid" and retrieves its exit status
  // FIXME: Need to correctly implement process_wait in process.c
  int id = 0;
  return process_wait(id);
}

bool create(const char *file, unsigned initial_size) {
  // Creates a new file
  // file: file name, initial_size: size in bytes
  if (file == NULL) {
    return -1;
  }

  // using locks to prevent race conditions
  lock_acquire(&file_lock);
  int result = filesys_create(file, initial_size);
  lock_release(&file_lock);
  return result;
}

bool remove(const char *file) {
  // Deletes the file
  // TODO:
}

int open(const char *file) {
  // Opens the file, returning non-negative integer, -1, or the fd
  // TODO:
  return 0;
}

int filesize(int fd) {
  // TODO:
  return 0;
}

int read(int fd, void *buffer, unsigned size) {
  // Read size bytes from fd into buffer,
  // TODO:
  return 0;
}

// int write (int fd, const void *buffer, unsigned size)
// void seek (int fd, unsigned position);
// unsigned tell (int fd);
// void close (int fd);
