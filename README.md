This project uses the P I N T O S teaching operating system for 32-bit x86.
The original project was developed to be as close to a real operating system, but more simplified to be used in a upper-division undergrad course.

## Docs folder
Contains a in-depth report of the programming edits made for each part of the labs:

• User Programs – Programmed syscalls/handler to provide kernel-level privileges. Used page-fault interrupts to exit bad-read/write threads.

• Virtual Memory – I designed a swap-table to store dirty/unused pages onto the disk. Partner did user paging and growable stack.

• File system – Reimplemented to allow file-growth and use of direct, indirect, dbl-indirect block indexing, minimizing external fragmentation

## Sources folder
Contains the project code.

Oversimplification of notable folders:

* /userprog - system call and process handling. Also contains virtual memory implementation (process.c)

* /filesys - file system directory and block setups
