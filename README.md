# Famine
Famine is an ELF64 program able to infect other ELF64 programs (located in specified folders), in Linux.
Famine is writen in C language with some inline ASM syscalls.

Tested on WSL2 and Ubuntu 22.04 with gcc 11.2.0, ld 2.38.

# Disclaimer
Please DONT run this on your machine !, Use a VM. 
Program is not harmfull by itself, mainly because it looks for targets in /tmp/test and /tmp/test2 folders, if you change those, you are on your own !!

## Method:
### 1. Targets Localisation
The program first looks for infectable programs in folders /tmp/test and /tmp/test2 (both built by Makefile).
### 2. Find injection point
When it finds so, it starts looking for a location in the target ELF64, looking for a big enought "code cave" to inject itself.
The code cave should be located after a PT_LOAD segment, then it already have every authorisation to run normally.
If it does not find such a large cave (most probable case sadly...), it will append itself on the end of the target program.
It this case then, it will needs to update the target's program header in order to recycle a note.ABI.tag section and a PT_NOTE segment to turn it as a PT_LOAD segment, so this code will be loaded and executable.
### 3. Injection
When Famine have found a location where to inject himself, the program will calculates its own size from __executable_start and _end labels (https://stackoverflow.com/questions/7370407/get-the-start-and-end-address-of-text-section-in-an-executable).
It also have to prepend a JUMP instruction to self decrypt function. (E9 + function offset from start) so the injected program knows where to start. The decrypt function will do its work (decrypt from main to end), and then will jump to main, after the registry save (which have been done at begining of decrypt function).
Also, at the very end of injected code, we have to append a JUMP to return to original program. 
To do so, we need a reference to locate the site so we use a dummy PUSH 0x42424242 as the very last instruction.
Right before, we insert a JMP 0x41414141. The 0x41414141 is located 7 bits before the 0x42424242 value (because of the size of a push instruction : 3 bits + the size of the 0x41414141 value : 4). We can then safely replace 0x41414141 by the target program's entrypoint address (relative, negative offset).the same method is used in decrypt function to get the key and the size to decrypt.
Because the original call of the Famine program don't need this return jump, we also prepended some return instructions (leave and ret as c9c3) so the very first program call (./Famine) returns gracefully. When it injects itself into another program, it will simply have to replace both c9 and c3 by 0x90 instructions (NOP as No OPeration), so program pointer will just increment and will end on our previously updated jump instruction, where we replaced the correct address to return back to main, and execute the original program normaly.
Once every of this steps are done, the program use a homemade Rabbit encryptor to encrypt the payload from main's start to end.
Obsiously, we wont encrypt the decrypt function...
### 4. Subtleties
As you may notice, this program is not writen in ASM as many other viruses, but in C. As you can guess, it did not initialy worked because of PLT, code relocation, library calls etc... 
To bypass those limitations, we used only self written utility functions (strcmp, memset, memmove, etc) and only system calls for the other functions such as mmap (to replace malloc, plus munmap as free), write, fstat, getdents, etc... 
We used C/ASM inlines to call those function so no PLT is made by compiler, and the code remains as portable as if it was all writen in ASM...
ref: https://stackoverflow.com/questions/9506353/how-to-invoke-a-system-call-via-syscall-or-sysenter-in-inline-assembly
Another thing needed is to save registers ans stack pointer location (rsp) before the start on the injected code, and at the end, restore all these ones to let the program run normaly !
Also, the program encrypt its content before it is injected, and a decrypt routine have been made to auto decrypt the program at run time. It prevents static analysis on the file.
### 5. Schema :
```
	Original Famine Program (to be injected):
__executable_start		main				_end			return back		dummy push
		|________________|___________________|leave ret _ jmp 0x41414141 _ push 0x42424242|
		|	clear part	 | 							encrypted part						  |

	First Contamination:
target program with replaced entry point and complete injection:
					target main	   target end Famine start	Famine main		Famine end			return back		dummy push
		|________________|____________________|JMP decrypt_________|________________|0x90 0x90 _ jmp target main _ push 0x42424242|
						 |						1|---------------------------------->2a|---------->2b|
						3|<------------------------------------------------------------|<------------|
	
	Next contaminations: Idem, with adapted jmp values for new target.

```
