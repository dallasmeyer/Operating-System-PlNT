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

// structs
struct lock file_lock;
struct lock process_lock;

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
  lock_init(&process_lock);
}

static void syscall_handler(struct intr_frame *f UNUSED) {
  // TODO: implement system call handler

  printf("system call!\n");

  // Get stack pointer via "esp" of intr_frame
  int *stack_p = f->esp;

  // check if virtual address
  if (!is_user_vaddr(stack_p)) {
    printf("syscall_handler(): Not a virtual address!");
  }

  // Dereference the stack pointer into the system call function number
  int syscall_funct = *stack_p;
  // Dev print statement:

  switch (syscall_funct) {

  //~~~~~ Project 2 system calls ~~~~~
  // Case 1: halt the operating system
  case SYS_HALT:
    printf("(syscall) syscall_funct is [SYS_HALT]\n");
    halt();
    break;

  // Case 2: terminate this process
  case SYS_EXIT:
    exit(0);
    break;

  // Case 3: Start another process
  case SYS_EXEC:
    f->eax = exec(*(stack_p + 1));
    break;

  // Case 4: Wait for a child process to die
  case SYS_WAIT:
    // wait();
    break;

  // Case 5: Create a file
  case SYS_CREATE:
    f->eax = create(*(stack_p + 4), *(stack_p + 5));
    break;

  // Case 6: Delete a file
  case SYS_REMOVE:
    remove(*(stack_p + 4));
    break;

  // Case 7: Open a file
  case SYS_OPEN:
    break;

  // Case 8: Obtain a files size
  case SYS_FILESIZE:
    break;

  // Case 9: Read from a file
  case SYS_READ:
    break;

  // Case 10: Write to a file
  case SYS_WRITE: {
    // setting file parameters
    int fd = *(stack_p + 5);
    const char *buffer = (const char *)*(stack_p + 6);
    unsigned length = *(stack_p + 7);

    f->eax = write(fd, buffer, length);
    break;
  }

  // Case 11: Change a position in a file
  case SYS_SEEK:
    break;

  // Case 12: Report a current position in a file
  case SYS_TELL:
    break;

  // Case 13: Close a file
  case SYS_CLOSE:
    break;

  //~~~~~ Project 2 System Calls ~~~~~
  // Default to exitting the process
  default:
    break;
  }
}

void halt(void) {
  // Terminates PintOS
  shutdown_power_off();
}

void exit(int status) {
  // Terminates current user program
  // FIXME: Need to implement parent and child wait
  thread_exit();
}

pid_t exec(const char *cmd_line) {
  // Runs executable
  printf("exec(): executing!");

  lock_acquire(&process_lock);
  pid_t result = process_execute(cmd_line);
  lock_release(&process_lock);

  return result;
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
  printf("remove(): removing!");
  if (file == NULL) {
    return -1;
  }

  lock_acquire(&file_lock);
  bool result = filesys_remove(file);
  lock_release(&file_lock);
  return result;
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

int write(int fd, const void *buffer, unsigned size) {
  // Writes size bytes from buffer to file descriptor, fd.
  // Idea: try and write all of the all of buffer to console in a single call.

  // Check if console out, as fd = 1 for console writes.
  if (fd == 1) {
    // don't need to lock, since putbuf has locks
    putbuf(buffer, size);
    return size;
  }

  // FIXME: implement getting file struct
  // struct file f_inst =

  lock_acquire(&file_lock);
  // off_t result = file_write(file_inst, buffer, size);
  lock_release(&file_lock);

  return 0;
}

// void seek (int fd, unsigned position);
// unsigned tell (int fd);
// void close (int fd);
