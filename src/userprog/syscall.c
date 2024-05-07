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
bool valid_addr(void * vaddr);

// structs
struct lock file_lock;
struct lock process_lock;

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
  lock_init(&process_lock);
}

bool valid_addr(void * vaddr){
  // Check if the virtual address is in the user address space 
  if (!is_user_vaddr(vaddr)) {
      printf("invalid vaddr\n");
      return false;
  }
  // Get the page directory of the current thread
  struct thread *t = thread_current();
  if (t == NULL || t->pagedir == NULL) {
      printf("invalid thread/pagedir\n");
      return false;
  }
  // Check if the page is to the correct virtual address
  void *pagedir = t->pagedir;
  if (pagedir_get_page(pagedir, vaddr) == NULL) {
      printf("invalid page\n");
      return false;
  }
  // Check if the virtual address is not a null pointer
  if (vaddr == NULL) {
      printf("Nonetype vaddr\n");
      return false;
  }
  // Return true otherwise
  printf("No issue found in vaddr\n");
  return true;
}


static void syscall_handler(struct intr_frame *f UNUSED) {
  // TODO: implement system call handler

  printf("(systcall) handler starting...\n");

  // Get stack pointer via "esp" of intr_frame
  int *stack_p = f->esp;
  printf("Stack pointer in syscall_handler: 0x%x\n", (uintptr_t)f->esp);

  // check if virtual address
  if (!valid_addr(stack_p)) {
    printf("syscall_handler(): Not a virtual address\n");
    exit(-1);
    return;
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

	  // Case 2: terminate this process
	  case SYS_EXIT: 
    printf("(syscall) syscall_funct is [SYS_EXIT]\n");
	  break; 
	  
	  // Case 3: Start another process
	  case SYS_EXEC: 
    printf("(syscall) syscall_funct is [SYS_EXEC]\n");
	  break; 

	  // Case 4: Wait for a child process to die
	  case SYS_WAIT: 
    printf("(syscall) syscall_funct is [SYS_WAIT]\n");
	  break; 

	  // Case 5: Create a file
	  case SYS_CREATE: 
    printf("(syscall) syscall_funct is [SYS_CREATE]\n");
	  break; 

	  // Case 6: Delete a file
	  case SYS_REMOVE: 
    printf("(syscall) syscall_funct is [SYS_REMOVE]\n");
	  break; 

	  // Case 7: Open a file 
	  case SYS_OPEN: 
    printf("(syscall) syscall_funct is [SYS_OPEN]\n");
	  break; 

	  // Case 8: Obtain a files size
	  case SYS_FILESIZE:
    printf("(syscall) syscall_funct is [SYS_FILESIZE]\n");
	  break; 

	  // Case 9: Read from a file 
	  case SYS_READ:
    printf("(syscall) syscall_funct is [SYS_READ]\n");
	  break; 

	  // Case 10: Write to a file 
	  case SYS_WRITE: 
    printf("(syscall) syscall_funct is [SYS_WRITE]\n");
	  break; 

	  // Case 11: Change a position in a file
	  case SYS_SEEK: 
    printf("(syscall) syscall_funct is [SYS_SEEK]\n");
	  break; 

	  // Case 12: Report a current position in a file
	  case SYS_TELL:
    printf("(syscall) syscall_funct is [SYS_TELL]\n"); 
	  break; 

	  // Case 13: Close a file
	  case SYS_CLOSE: 
    printf("(syscall) syscall_funct is [SYS_CLOSE]\n");
	  break; 

	  //~~~~~ Project 2 System Calls ~~~~~
  	  // Default to exiting the process 
	  default: 
    printf("(syscall) syscall_funct is DEFAULT [SYS_EXT]\n");
    exit(-1);
	  break; 
  
  }
}

void halt(void) {
  // Terminates PintOS
  shutdown_power_off();
}

void exit(int status) {
  // Terminates current user program
  // TODO:

  thread_current()->exit_status = status;
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
