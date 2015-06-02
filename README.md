# Einherjar - ᛖᛁᚾᚺᛖᚱᛃᚨᚱ
Operating system for PowerPC Macs.

Once upon a time there existed inspirational [computers...](https://en.wikipedia.org/wiki/List_of_Macintosh_models_grouped_by_CPU_type#PowerPC)

If one them weren't passed down to you, do a time travel or something and buy one.  
But if you can't, test it with qemu.

Status
------

This OS project is based on HelenOS and is in its early stages.
Einherjar is a single address space operating system. Thus applications should be written in secure languages.
Research may be done to make an exokernel version to allow insecure languages.

Working features:
- Handling the openfirmware
- Interrupts
- Physical and virtual memory manager
- Console (screen and keyboard)
- Threading

Screenshot
----------
![alt text](https://raw.githubusercontent.com/narke/Einherjar/master/docs/screenshots/einherjar.png "Einherjar")


Testing with qemu
-----------------

Note:
When you will type 'make' you will have several choices, choose like the following:

Option | Suggested selection
--- | ---
Platform | ppc32
Ramdisk | tmpfs
Compiler | gcc\_cross
'Kernel console support' will appear | Keep it
Input device class | generic
Output device class | generic
'Support for VIA CUDA controller' and 'framebuffer suport' will appear | Keep them
OHCI root hub port power switching | no

Once you are done select 'Done', the compiling process begins...
 
Cross-compiled versions of gcc, ld and likewise tools should be built.

Get the source code:
	git clone https://github.com/narke/Einherjar.git

Compile:

	make

Run:

	qemu-system-ppc -m 256 -boot d -cdrom einherjar.iso
