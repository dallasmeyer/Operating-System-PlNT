#include "userprog/syscall.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include <stdio.h>
#include <syscall-nr.h>

// used to toggle print statements
// #define debug_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define debug_printf(fmt, ...) // Define as empty if debugging is disabled


static void syscall_handler(struct intr_frame *);
bool valid_addr(void * vaddr);

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
  lock_init(&process_lock);
}

bool valid_addr(void * vaddr){
  // Check if the virtual address is in the user address space 
  if (!is_user_vaddr(vaddr)) {
      debug_printf("invalid vaddr\n");
      return false;
  }
  // Get the page directory of the current thread
  struct thread *t = thread_current();
  if (t == NULL || t->pagedir == NULL) {
      debug_printf("invalid thread/pagedir\n");
      return false;
  }
  // Check if the page is to the correct virtual address
  void *pagedir = t->pagedir;
  if (pagedir_get_page(pagedir, vaddr) == NULL) {
      debug_printf("invalid page\n");
      return false;
  }

  if (vaddr >= PHYS_BASE) {
      debug_printf("Invalid vaddr: above user address space\n");
      // Terminate the process with exit code -1
      thread_current()->exit_status = -1;
      thread_exit();
  }
  
  // Check if the virtual address is not a null pointer
  if (vaddr == NULL) {
      debug_printf("Nonetype vaddr\n");
      return false;
  }

  
  // Return true otherwise
  debug_printf("No issue found in vaddr\n");
  return true;
}


static void syscall_handler(struct intr_frame *f UNUSED) {
  // TODO: implement system call handler

  // debug_printf("(systcall) handler starting...\n");

  // Get stack pointer via "esp" of intr_frame
  int *stack_p = f->esp;
  debug_printf("Stack pointer in syscall_handler: 0x%x\n", (uintptr_t)f->esp);

  // check if virtual address
  if (!valid_addr(stack_p)) {
    debug_printf("syscall_handler(): Not a virtual address\n");
    exit(-1);
    return;
  }

  // Dereference the stack pointer into the system call function number
  int syscall_funct = *stack_p;
  // Dev print statement:
  
  if ((syscall_funct != SYS_EXIT) && (syscall_funct != SYS_WRITE)) {
    if (!valid_addr(*(stack_p+1)) || !valid_addr(stack_p+1)) {
      exit(-1);
    }
  }

  const char *file;

  switch (syscall_funct) {

  //~~~~~ Project 2 system calls ~~~~~
  // Case 1: halt the operating system
    case SYS_HALT:
      debug_printf("(syscall) syscall_funct is [SYS_HALT]\n");
      halt();
      break;

	  // Case 2: terminate this process
	  case SYS_EXIT: 
      debug_printf("(syscall) syscall_funct is [SYS_EXIT]\n");
      exit(*(stack_p+1));
      debug_printf("(syscall) syscall_funct is [SYS_EXIT] complete\n");
      break; 
	  
	  // Case 3: Start another process
	  case SYS_EXEC: 
      debug_printf("(syscall) syscall_funct is [SYS_EXEC]\n");
      f->eax = exec(*(stack_p + 1));
      break; 

	  // Case 4: Wait for a child process to die
	  case SYS_WAIT: 
      debug_printf("(syscall) syscall_funct is [SYS_WAIT]\n");
      break; 

	  // Case 5: Create a file
	  case SYS_CREATE: 
      debug_printf("(syscall) syscall_funct is [SYS_CREATE]\n");
      debug_printf("s1:%s,s2:%u,s3:%u,s4:%u\n", *(stack_p+1), *(stack_p+2), *(stack_p+3), *(stack_p+4));
      // if (!valid_addr(stack_p+1) || !valid_addr(stack_p+2) || !valid_addr(*(stack_p+2))) {exit(-1);}

      file = *(stack_p+1);
      unsigned size = *(stack_p+2);
      
      // FIXME: gets stuck at inode_create or dir_add in filesys_create
      bool result = create((const char * ) file,size);
      if (!result) { 
        exit(-1);
      }
      break; 

	  // Case 6: Delete a file
	  case SYS_REMOVE: 
      debug_printf("(syscall) syscall_funct is [SYS_REMOVE]\n");
      remove(*(stack_p+1));
      break; 

	  // Case 7: Open a file 
	  case SYS_OPEN: 
      debug_printf("(syscall) syscall_funct is [SYS_OPEN]\n");
      file = *(stack_p+1);
      f->eax = open(file);
      break; 

	  // Case 8: Obtain a files size
	  case SYS_FILESIZE:
      debug_printf("(syscall) syscall_funct is [SYS_FILESIZE]\n");
      break; 

	  // Case 9: Read from a file 
	  case SYS_READ:
      debug_printf("(syscall) syscall_funct is [SYS_READ]\n");
      if (!valid_addr((stack_p+2))  || !valid_addr((stack_p+3))) {exit(-1);}
      
      int fd = *(stack_p+1);
      void * buf = *(stack_p+2);
      unsigned sz = *(stack_p+3);
      f->eax = read(fd, buf, sz);
      break; 

	  // Case 10: Write to a file 
	  case SYS_WRITE: 
      debug_printf("(syscall) syscall_funct is [SYS_WRITE]\n");
      if (!valid_addr((stack_p+2))  || !valid_addr((stack_p+3))) {exit(-1);}
      
      fd = *(stack_p+1);
      buf = *(stack_p+2);
      sz = *(stack_p+3);
      write(fd, buf, sz);
      break; 

	  // Case 11: Change a position in a file
	  case SYS_SEEK: 
      debug_printf("(syscall) syscall_funct is [SYS_SEEK]\n");
      if (!valid_addr((stack_p+2))) exit (-1);
      
      seek(*(stack_p+1),*(stack_p+2));
      break; 

	  // Case 12: Report a current position in a file
	  case SYS_TELL:
      debug_printf("(syscall) syscall_funct is [SYS_TELL]\n"); 
      break; 

	  // Case 13: Close a file
	  case SYS_CLOSE: 
      debug_printf("(syscall) syscall_funct is [SYS_CLOSE]\n");
      break; 

	  //~~~~~ Project 2 System Calls ~~~~~
  	  // Default to exiting the process 
	  default: 
    debug_printf("(syscall) syscall_funct is DEFAULT [SYS_EXT]\n");
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
  thread_current()->exit_status = status;
  thread_exit();
}

pid_t exec(const char *cmd_line) {
  // Runs executable
  debug_printf("exec(): executing!");

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
  struct thread *cur = thread_current();
  
  if (file == NULL) {
    return -1;
  }

  // using locks to prevent race conditions
  debug_printf("create(): attempting to acquire file lock!\n");
  lock_acquire(&file_lock);
  debug_printf("create(): attempting to create!\n");
  int result = filesys_create(file, initial_size);
  debug_printf("create(): result = %d!\n", result);
  lock_release(&file_lock);
  debug_printf("create(): released file lock!\n");
  
  return result;
}

bool remove(const char *file) {
  // Deletes the file
  debug_printf("remove(): removing!");
  if (file == NULL) {
    return -1;
  }

  lock_acquire(&file_lock);
  bool result = filesys_remove(file);
  lock_release(&file_lock);
  return result;
}


struct file_inst *locate_file (int fd) {
  // get current thread
  struct thread * cur = thread_current();
  struct list_elem * list_e;

  // iterate through list of files until locate correct file descriptor element
  for (list_e = list_begin(&cur->list_files); list_e != list_end(&cur->list_files); list_e = list_next(list_e)) {
    struct file_inst * fd_e = list_entry(list_e, struct file_inst, file_list_e);
    if (fd_e->fd == fd) {
      return fd_e;
    }
  }

  debug_printf("locate_file(): returned NULL!\n");

  return NULL;
}

int open(const char *file) {
  // Opens the file, returning non-negative integer, -1, or the fd
  
  lock_acquire(&file_lock);
  struct file *file_p = filesys_open(file);
  lock_release(&file_lock);

  if (file_p == NULL) {
    return -1;
  }

  struct inode *i;
  char * f_name;

  // allocate memory for new file element, and instantiate struct
  struct file_inst * file_elem = malloc(sizeof(struct file_inst));
  file_elem->fd = ++thread_current()->fd_ct;
  file_elem->file_p = file_p;
  list_push_front(&thread_current()->list_files,&file_elem->file_list_e);

  return file_elem->fd;
}

int filesize(int fd) {
  // TODO:
  return 0;
}

int read(int fd, void *buffer, unsigned size) {
  // Read size bytes from fd into buffer,
  
  // check if keyboard input, as fd = 0 for user writes.
  int cur_len = 0;
  if (fd == 0) {
    // continue taking keyboard input
    while (cur_len < size) {
      *((char *) buffer + cur_len) = input_getc();
      cur_len++;
    }

    return cur_len;
  }

  // locate file
  struct file_inst * fd_e = locate_file(fd);
  if (fd_e == NULL) exit(-1);

  // read file
  lock_acquire(&file_lock);
  int result = file_read(fd_e->file_p, buffer, size);
  lock_release(&file_lock);

  return result;
}

int write(int fd, const void *buffer, unsigned size) {
  // Writes size bytes from buffer to file descriptor, fd.
  // Idea: try and write all of the all of buffer to console in a single call.

  // Check if console out, as fd = 1 for console writes.
  
  // debug_printf("fd:%d\n", fd);
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }

  struct file_inst * fd_e = locate_file(fd);

  if (fd_e == NULL) {
    return -1;
    debug_printf("write(): fd_e NULL!\n");
  }
  
  lock_acquire(&file_lock);
  // write to the file
  int result = file_write(fd_e->file_p, buffer, size);
  lock_release(&file_lock);

  debug_printf("result:%d\n", result);

  return result;
}

void seek (int fd, unsigned position) {
  struct file_inst * file_elem = locate_file(fd);
  if (file_elem == NULL) exit(-1);

  // using seek function
  lock_acquire(&file_lock);
  file_seek(file_elem->file_p, position);
  lock_release(&file_lock);
}
// unsigned tell (int fd);
// void close (int fd);
