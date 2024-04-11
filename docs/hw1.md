## 1) What is the physical address of the first instruction that is executed by the BIOS? hint: it is the address of $eip when you start PintOS
Physical address: 0xC0020EE0

## 2) How does the bootloader read disk sectors? In particular, what BIOS interrupt does it use?
Bootloader reads the disk sector by looking at the first sector to see if its a boot sector.
Uses INT 0x13

## 3) How does the bootloader decide if it successfully finds the PintOS kernel?
In loader.S, the bootloader checks if the partition is equal to partition of type 0x20.

code snippet:
```
	# Is it a Pintos kernel partition?
	cmpb $0x20, %es:4(%si)
	jne next_partition
```

## 4) What happens if the bootloader is unsuccessful in finding the PintOS kernel
If the bootloader doesn't find the kernel, it prints a string "Not found" and sends a interrupt 0x18.

Code snippet:
```
	# Didn't find a Pintos kernel partition anywhere, give up.
	call puts
	.string "\rNot found\r"

	# Notify BIOS that boot failed.  See [IntrList].
	int $0x18
```

## 5) How exactly does the bootloader transfer control to PintOS?
The bootloader transfers control to PintOS by setting up an address at 0x20000 -- after loading the kernel --  temporarily storing it and setting up the entry point at start().

Code snippet referenced:
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

## 6) Tracing the behavior of the palloc_get_page() function from PintOS. Set a breakpoin on this function and identify the following values the first time that it is called:
### (a) What is the call stack when the function is called?
Stack:
palloc_get_pate()
paging_init()
pintos_init()
start()

Console output:
```
(gdb) bt
#0  palloc_get_page (flags=(PAL_ASSERT | PAL_ZERO)) at ../../threads/palloc.c:112
#1  0xc00203aa in paging_init () at ../../threads/init.c:168
#2  0xc002031b in pintos_init () at ../../threads/init.c:100
#3  0xc002013d in start () at ../../threads/start.S:180
```

### (b) What is the return value of the function, in hexadecimal format, on its first invocation?
Value is 0xc000efd8
```
(gdb) info registers $ebp
ebp            0xc000efd8          0xc000efd8
```


## 7) Tracing the behavior of the palloc_get_page() function from PintOS. Set a breakpoint on this function and identify the following values the third time that it is called:
### (a) What is the call stack when the function is called?
Stack:
palloc_get_pate()
thread_create()
thread_start()
pintos_init()
start()

```
(gdb) bt
#0  palloc_get_page (flags=PAL_ZERO) at ../../threads/palloc.c:112
#1  0xc0020a81 in thread_create (name=0xc002c4d5 "idle", priority=0, function=0xc0020eb0 <idle>,
    aux=0xc000efbc) at ../../threads/thread.c:178
#2  0xc0020976 in thread_start () at ../../threads/thread.c:111
#3  0xc0020334 in pintos_init () at ../../threads/init.c:119
#4  0xc002013d in start () at ../../threads/start.S:180
```
### (b) What is the return value of the function, in hexadecimal format, on its first invocation?
Value is 0xc000ef98
```
(gdb) info registers $ebp
ebp            0xc000ef98          0xc000ef98
```
