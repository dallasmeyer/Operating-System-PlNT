Homework #1 | Keith Irby | CSE 134 Embedded Operating Systems

### Q1 
The physical address of the first instruction after running gdb and continueing can be found in loader.S. The physical address found was "0xc0020ee0". 

### Q2

The bootloader always reads the first sector of a device to check for a boot sector. The interrupt it uses is "0x13". 



### Q3 


The pintos bootloader knows it successfully found the pintos kernel by finding a bootable Master Boot Record in the first sector of a hard disk. Then it can extract bootable kernel parition from the hard disk and transfer control control to the kernel. This is shown in loader.S code segement below.

```
# Is it a Pintos kernel partition?
    cmpb $0x20, %es:4(%si)
    jne next_partition
```

### Q4

If the bootloader fails to find a pintos Master Boot Record the OS will not open and will report a "Not Found" string and a interrupt of "0x18".




### Q5

 The loader transfers control to the already loaded pintos kernel by extracting the entry point and passing it to the loaded kernel. The exact code used by loader.S is shown below. But the passing is done in the start.S file.

```
#### Transfer control to the kernel that we loaded.  We read the start
#### address out of the ELF header (see [ELF1]) and convert it from a
#### 32-bit linear address into a 16:16 segment:offset address for
#### real mode, then jump to the converted address.  The 80x86 doesn't
#### have an instruction to jump to an absolute segment:offset kept in
#### registers, so in fact we store the address in a temporary memory
#### location, then jump indirectly through that location.  To save 4
#### bytes in the loader, we reuse 4 bytes of the loader's code for
#### this temporary pointer.

    mov $0x2000, %ax
    mov %ax, %es
    mov %es:0x18, %dx
    mov %dx, start
    movw $0x2000, start + 2
    ljmp *start
```

### Q6(a)

The call stack is found using the "backtrace" command or "bt" for short in gdb. The one found for the first inovcation is shown above for the function "palloc_get_page()" found in "palloc.c" but debugged from "kernel.o" inside the build folder. 

```
(gdb) bt
#0  palloc_get_page (flags=(PAL_ASSERT | PAL_ZERO)) at ../../threads/palloc.c:112
#1  0xc00203aa in paging_init () at ../../threads/init.c:168
#2  0xc002031b in pintos_init () at ../../threads/init.c:100
#3  0xc002013d in start () at ../../threads/start.S:180
```

### Q6(b)

The return value of the palloc_get_page() function is "0xc00efd8". 

```
(gdb) info registers $ebp
ebp            0xc000efd8          0xc000efd8
```


### Q7(a)

The call stack is found the same way as Q7(a), using the "bt" command, and is shown below.

```
(gdb) bt
#0  palloc_get_page (flags=PAL_ZERO) at ../../threads/palloc.c:112
#1  0xc0020a81 in thread_create (name=0xc002c4d5 "idle", priority=0, function=0xc0020eb0 <idle>,
    aux=0xc000efbc) at ../../threads/thread.c:178
#2  0xc0020976 in thread_start () at ../../threads/thread.c:111
#3  0xc0020334 in pintos_init () at ../../threads/init.c:119
#4  0xc002013d in start () at ../../threads/start.S:180
```


### Q7(b) 


The return value of the palloc_get_page() function is "0xc000ef98". 

```
(gdb) info registers $ebp
ebp            0xc000ef98          0xc000ef98
```












### Sources 
https://www.cs.jhu.edu/~huang/cs318/fall21/project/project0.html
https://www.cs.jhu.edu/~huang/cs318/fall21/project/pintos_1.html#SEC1  
https://ics.uci.edu/~ardalan/courses/os/pintos/pintos_2.html
https://web.stanford.edu/class/cs140/projects/pintos/pintos_6.html
https://www.cse.unsw.edu.au/~learn/debugging/modules/gdb_call_stack/

   
