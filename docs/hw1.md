Homework #1 | Keith Irby | CSE 134 Embedded Operating Systems

Q1: The physical address of the first instruction can be found in loader.S after debugging with gdb and "debugpintos". The physical address found was "0xeab7f". 

Q2: The bootloader always reads the first sector of a device to check for a boot sector. 


Q3: The pintos bootloader knows it successfully found the pintos kernel by finding a bootable Master Boot Record in the first sector of a hard disk. Then it can extract bootable kernel parition from the hard disk and transfer control control to the kernel.

Q4: If the bootloader fails to find a Master Boot Record the OS will not open.

Q5: The loader transfers control to pintos by extracting the entry point from the loaded kernel and trafering control to the kernel.














----------- Sources 
https://www.cs.jhu.edu/~huang/cs318/fall21/project/project0.html
https://www.cs.jhu.edu/~huang/cs318/fall21/project/pintos_1.html#SEC1  
https://ics.uci.edu/~ardalan/courses/os/pintos/pintos_2.html
https://web.stanford.edu/class/cs140/projects/pintos/pintos_6.html

   
