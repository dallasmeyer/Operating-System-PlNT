#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

// Higher level debugger
//#define debug_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define debug_printf(fmt, ...) // Uncomment to turn debugger off and comment above
//debug_printf("()\n");
// Deeper level debugger
//#define debug_extra_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define debug_extra_printf(fmt, ...) // Uncomment to turn debugger off and comment above

// NEW: for synchronizations
#include "threads/synch.h"

#define MAX_ARGS_LEN 4096 // NEW: max size for user program arguments


thread_func start_process;
static bool load (const char *cmdline, void (**eip) (void), void **esp, char **user_args, int arg_count);

/** Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */
tid_t
process_execute (const char *file_name) 
{
  char *fn_copy;
  tid_t tid;

  debug_printf("(process_execute) starting..\n");
  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);


  /* Create a new thread to execute FILE_NAME. */
  debug_printf("(process_execute) creating thread process...\n");
  thread_current()->child_loaded = 0;
  tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
  if (tid == TID_ERROR){
    // Failed to create a thread, free and return the failed thread
    palloc_free_page (fn_copy); 
    return tid;
   }
  // Wait on the new user_prog thread load
  sema_down(&thread_current()->sem_child_load);
  debug_printf("(process_execute) child load finished\n");
  // Check if the child thread loaded 
  if(!thread_current()->child_loaded){
    debug_printf("(process_execute) child failed to load\n");
    // Collect the child thread element by popping it from the list
    struct list_elem *elem_c = list_pop_front(&thread_current()->child_list); 
    struct child *c_t = list_entry(elem_c, struct child, child_elem); 
    // Delete the list element and the child
    list_remove(elem_c); 
    free(c_t);
  }

  debug_printf("(process_execute) child load finished [%s]\n", thread_current()->name);
  // Return tid when done 
  return tid;
}

/** A thread function that loads a user process and starts it
   running. */
void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;

  // NEW: parse the command line string
  char *save_ptr; 
  char *user_prog;

  // NEW: parse the first token to save the user_prog
  user_prog = strtok_r(file_name_, " ", &save_ptr);
  if(user_prog == NULL){
  	// If we failed to find the user prog 
	// or any other issue, exit
	return;

  } 
  // Added begin print that checks if args operation
   // TO-DO maybe remove this, there is a begin print somewhere else
   if (strstr(user_prog, "args") != NULL) {
     //printf("(args) begin\n");
   } else {
     //printf("(%s) begin\n", user_prog);
   }


  // NEW: parse and save the user arguments
  // allocate memory for the user arguments
  char **user_args = malloc(MAX_ARGS_LEN + strlen(user_prog) + 1); // FREE THIS
  if(user_args == NULL){
	// Failed to allocate enough space for user args
	// return -1, this print might need to change later
    // TO-DO: add thread exit here I think
  }else{
  	// Add user_prog as the first element
	// if memory allocated correctly 
	user_args[0] = user_prog;
  
  }
  // Parse the user arguments into tokens
  int arg_count = 1;
  size_t arg_size = 0;
  char *token;
  token = strtok_r(NULL, " ", &save_ptr);
  while (token != NULL){
	// Add current tokens size to running arg size + null terminator
  	arg_size += strlen(token) +1; 
	// Check that we have not exceeded the max argument size
	if (arg_size > MAX_ARGS_LEN) {
		// If we exceeded the max arguments size, stop 
		// looking for more arguments and continue with 
		// what was found 
		break;
	}
	// Add argument to user_args
	user_args[arg_count] = token;
	// Add 1 to total number of arguments
	++arg_count;
	debug_extra_printf("	arg_count: %d | token: [%s]\n", arg_count, token);
	// Check for the next token
	token = strtok_r(NULL, " ", &save_ptr);
  }
  // Null terminate user_args
  user_args[arg_count] = NULL;
  // dev-print
  debug_extra_printf("	total arg_count: %d | total_size %lu \n", arg_count, arg_size);
 

  // For loop to print out the user prog paraemters
  // if (strstr(user_prog, "args") != NULL) printf("(args) argc = %d\n", arg_count); 
  
  for(int i = 0; i < arg_count+1; i++){
    if (strstr(user_prog, "args") != NULL) {
      if (user_args[i]) {
        // printf("(args) argv[%d] = '%s'\n", i, user_args[i]); 
      } else {
        // printf("(args) argv[%d] = null\n", i); 
      }
    }
  }; 


  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (file_name, &if_.eip, &if_.esp, user_args, arg_count);
  // NEW: set the child loaded flag for parent thread that started the user program
  debug_printf("(process_start) load success is [%d]\n", success);
  thread_current()->parent->child_loaded = success;

  // Added begin print that checks if args operation
  if (strstr(user_prog, "args") != NULL) {
    // printf("(args) end\n");
  }
  
  // NEW: Free allocated memory from up above
  free(user_args);
  user_args = NULL; 

  /* If load failed, quit. */
  palloc_free_page (file_name);
  if (!success){
    thread_current()->exit_status = -1;
    debug_printf("(process_start) error loading source file\n");
    sema_up(&thread_current()->parent->sem_child_load); 
    thread_exit ();
  }

  // unblock parent thread
  debug_printf("(process_start) load successful\n");
  sema_up(&thread_current()->parent->sem_child_load); 


  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/** Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */
int
process_wait (tid_t child_tid UNUSED) 
{
  
   // Stops the main/parent thread from killing the child before it finishes its program 
  sema_down(&thread_current()->sem_child_wait);
  return -1;
}

/** Free the current process's resources. */
void
process_exit (void)
{
  debug_printf("(process_exit) Starting\n");
  struct thread *cur = thread_current ();
  uint32_t *pd;

  // Print the exit message 
  char * saveptr;
  printf("%s: exit(%d)\n",strtok_r(cur->name, " ", saveptr),cur->exit_status);
  
  debug_printf("(process_exit) destroying child threads\n");
  // NEW: destroy the children threads
  while(!list_empty(&cur->child_list)){
    // Collect the child thread element by popping it from the list
    struct list_elem *elem_c = list_pop_front(&cur->child_list); 
    struct child *c_t = list_entry(elem_c, struct child, child_elem); 
    // Delete the list element and the child
    list_remove(elem_c); 
    free(c_t);
  }
  debug_printf("(process_exit) Child threads destroyed\n");  

  // FIXME: deallocate file memory
  
  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
  debug_printf("(process_exit) finished\n");
}

/** Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/** We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/** ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/** For use with ELF types in printf(). */
#define PE32Wx PRIx32   /**< Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /**< Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /**< Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /**< Print Elf32_Half in hexadecimal. */

/** Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/** Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/** Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /**< Ignore. */
#define PT_LOAD    1            /**< Loadable segment. */
#define PT_DYNAMIC 2            /**< Dynamic linking info. */
#define PT_INTERP  3            /**< Name of dynamic loader. */
#define PT_NOTE    4            /**< Auxiliary info. */
#define PT_SHLIB   5            /**< Reserved. */
#define PT_PHDR    6            /**< Program header table. */
#define PT_STACK   0x6474e551   /**< Stack segment. */

/** Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /**< Executable. */
#define PF_W 2          /**< Writable. */
#define PF_R 4          /**< Readable. */

static bool setup_stack (void **esp, char **user_args, int arg_count); 
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/** Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *file_name, void (**eip) (void), void **esp, char **user_args, int arg_count) 
{
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL) 
    goto done;
  process_activate ();


  /* Open executable file. */
  debug_extra_printf("(process load) opening executable [%s]\n", file_name);
  file = filesys_open (file_name);
  if (file == NULL) 
    {
      printf ("load: %s: open failed\n", file_name);
      goto done; 
    }


  debug_extra_printf("(process load) verifying headers\n");
  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024) 
    {
      debug_printf("(process load) %s: error loading executable\n", file_name);
      goto done; 
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) 
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      switch (phdr.p_type) 
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file)) 
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else 
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }

  debug_extra_printf("(process load) Setting up stack\n");
  /* Set up stack. */
  if (!setup_stack (esp, user_args, arg_count))
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  if (success != true){
    // NEW: If we failed to load the file, we close it, otherwise continue
    file_close (file);
  } 
  return success;
}


/** load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/** Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/** Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page of memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (kpage);
          return false; 
        }
      memset (kpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, kpage, writable)) 
        {
          palloc_free_page (kpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/** Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool setup_stack(void **esp, char **user_args, int arg_count) {
    uint8_t *kpage;
    bool success = false;

    kpage = palloc_get_page(PAL_USER | PAL_ZERO);
    if (kpage != NULL) {
        // Install page into the user's virtual address space
        success = install_page(((uint8_t *)PHYS_BASE) - PGSIZE, kpage, true);
        if (success) {
            // Initialize the stack pointer to PHYS_BASE (top of page)
            *esp = PHYS_BASE;

            // Create a new array to hold the ptr addresses of user arguments
            char **argv = malloc((arg_count + 1) * sizeof(char *));
            if (argv == NULL) {
                // If fail to allocate, free and give up
                palloc_free_page(kpage);
                return false;
            }

            // Push the addresses of user program arguments onto user virtual memory space
            for (int i = arg_count - 1; i >= 0; i--) {
                *esp -= strlen(user_args[i]) + 1;
                memcpy(*esp, user_args[i], strlen(user_args[i]) + 1);
                argv[i] = *esp;
            }

            // Word-align the stack pointer
            *esp -= (uintptr_t)(*esp) % 4;

            // Null-terminate argv
            argv[arg_count] = NULL;

            // Push the addresses of argv[0] and argc onto the stack
            *esp -= sizeof(char *);
            memcpy(*esp, &argv[arg_count], sizeof(char *));
            for (int j = arg_count - 1; j >= 0; j--) {
                *esp -= sizeof(char *);
                memcpy(*esp, &argv[j], sizeof(char *));
            }
            char **argv_start = *esp;
            *esp -= sizeof(char **);
            memcpy(*esp, &argv_start, sizeof(char **));
            *esp -= sizeof(int);
            memcpy(*esp, &arg_count, sizeof(int));

            // Push a fake return address
            *esp -= sizeof(void *);
            *((uintptr_t *)*esp) = 0;

            // Cleanup allocated memory
            free(argv);

            success = true;
        } else {
            // Issue installing page, free it
            palloc_free_page(kpage);
        }
    }

    return success;
}

/** Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}
