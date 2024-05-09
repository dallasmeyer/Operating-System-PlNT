#include "userprog/syscall.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <syscall-nr.h>

// used to toggle print statements
//#define debug_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define debug_printf(fmt, ...) // Define as empty if debugging is disabled
//#define debug_extra_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define debug_extra_printf(fmt, ...) // Uncomment to turn debugger off and comment above

static void syscall_handler(struct intr_frame *);
bool valid_addr(void * vaddr);
bool valid_str(char *str);

void syscall_init(void) {
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
  lock_init(&process_lock);
}


bool valid_addr(void *vaddr) {
      // Check if the virtual address is not a null pointer
    if (vaddr == NULL) {
        debug_printf("Nonetype vaddr\n");
        return false;
    }

    // Check if the virtual address is in the user address space
    if (!is_user_vaddr(vaddr)) {
        debug_printf("invalid vaddr\n");
        return false;
    }

    // Check if the page is valid    
    if (pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
        debug_printf("Invalid vaddr page\n");
        return false;
    }

    // Return true otherwise
    debug_extra_printf("No issue found in vaddr\n");
    return true;
}

bool valid_str(char *str){
    // Check if the str is in the user address space
    if (!is_user_vaddr(str)) {
        debug_printf("invalid str addr\n");
        return false;
    }

    // Check if the string is in the correct page
    char *page_str = pagedir_get_page(thread_current()->pagedir, (void *) str);
    if(page_str == NULL){
      // Checking if the first part is in the correct page
      debug_printf("invalid str page NULL\n");
      return false;
    }else{
      // Checking if the last part + null terminator is in the correct page
      char *end_str = str + strlen(page_str) + 1;
      if (!is_user_vaddr(end_str) || pagedir_get_page(thread_current()->pagedir, end_str) == NULL) {
        return false;
  	  }
    }
    // Otherwise return true
    return true;
}

static void syscall_handler(struct intr_frame *f UNUSED) {
  // Get stack pointer via "esp" of intr_frame
  int *stack_p = f->esp;
  debug_printf("(syscall_handler) Stack pointer : 0x%x and funct [%d]\n", 
      (uintptr_t)f->esp, *stack_p);

  int stack_size = (uint32_t)PHYS_BASE - (uint32_t)f->esp;
  debug_printf("  (syscall_handler) call stack size [%d]\n", 
    stack_size);

  // Check if stack pointer is within user address space and is mapped to a valid page
  if (stack_size < 1 || !valid_addr(stack_p)) {
    debug_printf("syscall_handler(): Invalid call stack ptr\n");
    exit(-1);
    return;
  }

  // Dereference the stack pointer into the system call function number
  int syscall_funct = *stack_p;

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
      if(!valid_addr(stack_p+1)) {exit(-1);}
      exit(*(stack_p+1));
      debug_printf("(syscall) syscall_funct is [SYS_EXIT] complete\n");
      break; 
	  
	  // Case 3: Start another process
	  case SYS_EXEC: 
      if(!valid_addr((stack_p + 1)) || !valid_str(*(stack_p + 1))) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_EXEC]\n");
      f->eax = exec(*(stack_p + 1));
      break; 

	  // Case 4: Wait for a child process to die
	  case SYS_WAIT: 
      //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_WAIT]\n");
      if(!valid_addr(stack_p+1)) {exit(-1);}
      f->eax = wait(*(stack_p+1));
      break; 

	  // Case 5: Create a file
	  case SYS_CREATE: 
      debug_printf("(syscall) syscall_funct is [SYS_CREATE]\n");
      if(!valid_addr(stack_p+1) || !valid_addr(stack_p+2)) 
      {exit(-1);}
      debug_printf("s1:%s,s2:%u\n", *(stack_p+1), *(stack_p+2));
      char *file = *(stack_p+1);
      unsigned size = *(stack_p+2);
      
      // FIXME: gets stuck at inode_create or dir_add in filesys_create
      bool result = create((const char * ) file,size);
      if (!result) { 
        exit(-1);
      }
      break; 

	  // Case 6: Delete a file
	  case SYS_REMOVE: 
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_REMOVE]\n");
      remove(*(stack_p+1));
      break; 

	  // Case 7: Open a file 
	  case SYS_OPEN: 
      debug_printf("(syscall) syscall_funct is [SYS_OPEN]\n");
      if(!valid_addr(stack_p+1)){exit(-1);}
      file = *(stack_p+1);
      f->eax = open(file);
      break; 

	  // Case 8: Obtain a files size
	  case SYS_FILESIZE:
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_FILESIZE]\n");
      break; 

	  // Case 9: Read from a file 
	  case SYS_READ:
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_READ]\n");
      if (!valid_addr((stack_p+1))  || !valid_addr((stack_p+2))  
          || !valid_addr((stack_p+3)))
          {exit(-1);}
      
      int fd = *(stack_p+1);
      void * buf = *(stack_p+2);
      unsigned sz = *(stack_p+3);
      f->eax = read(fd, buf, sz);
      break; 

	  // Case 10: Write to a file 
	  case SYS_WRITE: 
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_WRITE]\n");
      if (!valid_addr((stack_p+1)) || !valid_addr((stack_p+2))  
          || !valid_addr((stack_p+3)) || !valid_addr(*(stack_p+2))) {exit(-1);}
      debug_printf("s1:%u,s2:%u,s3:%u\n", *(stack_p+1), *(stack_p+2), *(stack_p+3));
      fd = *(stack_p+1);
      buf = *(stack_p+2);
      sz = *(stack_p+3);
      //if(!valid_addr(buf+sz))
      f->eax = write(fd, buf, sz);
      break; 

	  // Case 11: Change a position in a file
	  case SYS_SEEK: 
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_SEEK]\n");
      if (!valid_addr((stack_p+2)) || !valid_addr((stack_p+2))) exit (-1);
      
      seek(*(stack_p+1),*(stack_p+2));
      break; 

	  // Case 12: Report a current position in a file
	  case SYS_TELL:
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_TELL]\n"); 
      break; 

	  // Case 13: Close a file
	  case SYS_CLOSE: 
       //if(!valid_add()) {exit(-1);}
      debug_printf("(syscall) syscall_funct is [SYS_CLOSE]\n");
      break; 

	  //~~~~~ Project 2 System Calls ~~~~~
  	  // Default to exiting the process 
	  default: 
    debug_printf("(syscall) syscall_funct is DEFAULT [SYS_EXIT]\n");
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
  debug_printf("(exit) [%s] Exiting program\n", thread_current()->name);
  debug_printf("  (exit) with status [%d]\n", status);
  
  if (thread_current()->child_loaded != -1){
   thread_current()->exit_status = status;
  }else{
    thread_current()->exit_status = -1;
  }
  
  thread_current()->parent->child_done = 1; 
  if(thread_current()->parent->child_waiting == thread_current()->tid){
    // If the parent thread is waiting on us, release them
    debug_printf("(exit) Releasing parent\n");
    sema_up(&thread_current()->parent->sem_child_wait); 
  }
  
  thread_exit();
}

pid_t exec(const char *cmd_line) {
  // Runs executable
  debug_printf("exec(): executing!\n");

  lock_acquire(&process_lock);
  pid_t result = process_execute(cmd_line);
  lock_release(&process_lock);

  return result;
}

int wait(pid_t pid) {
  // Waits for child "pid" and retrieves its exit status
  tid_t tid = process_wait(pid); 
  return tid;
}

bool create(const char *file, unsigned initial_size) {
  // Creates a new file
  // file: file name, initial_size: size in bytes
  struct thread *cur = thread_current();
  debug_printf("create(): thread is [%s]\n", cur->name);

  if (file == NULL) {
    return -1;
  }

  // using locks to prevent race conditions
  debug_printf("create(): attempting to acquire file lock\n");
  lock_acquire(&file_lock);
  int result = filesys_create(file, initial_size);
  lock_release(&file_lock);
  debug_printf("create(): result = %d!\n", result); 
  
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
  debug_printf("(open) Opening file\n");
  lock_acquire(&file_lock);
  struct file *file_p = filesys_open(file);
  lock_release(&file_lock);
  // Return if we failed to open the file
  if (file_p == NULL) {
    debug_printf("(open) failed to open file\n");
    return -1;
  }

   // Allocate memory for new file element and instantiate the struct
    debug_printf("(open) allocating memory\n");
    struct file_inst *file_elem = malloc(sizeof(struct file_inst));
    if (file_elem == NULL) {
        // Close and return if we failed to allocate
        file_close(file_p); 
        return -1;
    }
  debug_printf("(open) initializing element\n");
  // Initialize file element
  file_elem->fd = ++thread_current()->fd_ct;
  file_elem->file_p = file_p;
  // Insert the file elements
  debug_printf("(open) inserting item\n");
  list_push_front(&thread_current()->list_files,&file_elem->file_list_e);
  debug_printf("(open) Finished\n");
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
    while (cur_len < STDIN_FILENO) {
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
  
  debug_printf("(write) fd:%d\n", fd);
  if (fd == STDOUT_FILENO) {
    putbuf(buffer, size);
    return size;
  }
  debug_printf("(write) fd:%d Done\n", fd);

  struct file_inst * fd_e = locate_file(fd);

  if (fd_e == NULL) {
    return -1;
    debug_printf("(write) fd_e NULL!\n");
  }
  
  lock_acquire(&file_lock);
  // write to the file
  int result = file_write(fd_e->file_p, buffer, size);
  lock_release(&file_lock);

  debug_printf("(write) result:%d\n", result);

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
