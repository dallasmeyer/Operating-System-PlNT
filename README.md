# Pintos

[Pintos](http://pintos-os.org) is a teaching operating system for 32-bit x86.
The project is real enough to teach operating systems, but small enough to be
usable for an undergraduate curriculum.  This repository makes a number of
further simplifications to ensure that the project can be completed in a 10 week
quarter.  The repository also includes helpful simulators and tools for making
testing and debugging easier for students.

## Acknowledgments

The main source code, documentation, and assignments were developed by Ben Pfaff
and others from Stanford University (see the Pintos [LICENSE]).  The source tree
here includes some modifications and enhancements from Peking University (PKU).
Following provenance one additional step, the PKU source tree is adapted from
Dr. Ryan Huang (now at University of Michigan).

## Docs folder
Contains a in-depth report of the programming edits made for each part of the PintOS labs:

• User Programs – Programmed syscalls/handler to provide kernel-level privileges. Used page-fault interrupts to exit bad-read/write threads.

• Virtual Memory – I designed a swap-table to store dirty/unused pages onto the disk. Partner did user paging and growable stack.

• File system – Reimplemented to allow file-growth and use of direct, indirect, dbl-indirect block indexing, minimizing external fragmentation
