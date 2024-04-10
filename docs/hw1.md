# 1) What is the physical address of the first instruction that is executed by the BIOS? hint: it is the address of $eip when you start PintOS


# 2) How does the bootloader read disk sectors? In particular, what BIOS interrupt does it use?

# 3) How does the bootloader decide if it successfully finds the PintOS kernel?

# 4) What happens if the bootloader is unsuccessful in finding the PintOS kernel

# 5) How exactly does the bootloader transfer control to PintOS?

# 6) Tracing the behavior of the palloc_get_page() function from PintOS. Set a breakpoin on this function and identify the following values the first time that it is called:
(a) What is the call stack when the function is called?

(b) What is the return value of the function, in hexadecimal format, on its first invocation?

# 7) Tracing the behavior of the palloc_get_page() function from PintOS. Set a breakpoint on this function and identify the following values the third time that it is called:
(a) What is the call stack when the function is called?

(b) What is the return value of the function, in hexadecimal format, on its first invocation?


