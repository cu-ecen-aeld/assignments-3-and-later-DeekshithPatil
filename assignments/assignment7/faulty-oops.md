# Faulty oops analysis

## faulty.c script

ssize_t faulty_write (struct file *filp, const char __user *buf, size_t count, \
		loff_t *pos) \
{
	/* make a simple fault by dereferencing a NULL pointer */ \
	*(int *)0 = 0; \
	return 0; \
} \

As we can see the line "*(in t*)0 = 0" is dereferencing a NULL pointer. \
This will generate an error everytime faulty_write() is called. ]

## Invoking faulty_write() from terminal
To produce the oops error, we need to generate a function call to faulty_write(). \
This is can be done using the echo command and writing to the device file as shown below, \

echo “hello_world” > /dev/faulty \

## Output of echo “hello_world” > /dev/faulty
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000 \
Mem abort info: \
  ESR = 0x96000045 \
  EC = 0x25: DABT (current EL), IL = 32 bits \
  SET = 0, FnV = 0 \
  EA = 0, S1PTW = 0 \
  FSC = 0x05: level 1 translation fault \
Data abort info: \
  ISV = 0, ISS = 0x00000045 \
  CM = 0, WnR = 1 \
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000042033000 \
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000 \
Internal error: Oops: 96000045 [#1] SMP \
Modules linked in: hello(O) faulty(O) scull(O) \
CPU: 0 PID: 154 Comm: sh Tainted: G           O      5.15.16 #1 \
Hardware name: linux,dummy-virt (DT) \
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--) \
pc : faulty_write+0x14/0x20 [faulty] \
lr : vfs_write+0xa8/0x2b0 \
sp : ffffffc010c9bd80 \
x29: ffffffc010c9bd80 x28: ffffff800209a580 x27: 0000000000000000 \
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000 \
x23: 0000000040001000 x22: 0000000000000012 x21: 000000558ee21940 \
x20: 000000558ee21940 x19: ffffff8001fd0800 x18: 0000000000000000 \
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000 \
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000 \
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000 \
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000 \
x5 : 0000000000000001 x4 : ffffffc0086a7000 x3 : ffffffc010c9bdf0 \
x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000 \
Call trace: \
 faulty_write+0x14/0x20 [faulty] \
 ksys_write+0x68/0x100 \
 __arm64_sys_write+0x20/0x30 \
 invoke_syscall+0x54/0x130 \
 el0_svc_common.constprop.0+0x44/0xf0 \
 do_el0_svc+0x40/0xa0 \
 el0_svc+0x20/0x60 \
 el0t_64_sync_handler+0x1a4/0x1b0 \
 el0t_64_sync+0x1a0/0x1a4 \
Code: d2800001 d2800000 d503233f d50323bf (b900003f) \
---[ end trace 8cc8c10c8d129ec5 ]---   

## Analysis of Output
The output is a dump of all the system registers and function stack calls made. \
We can understand more about the error by looking at the function calls made. \

The error has occured at faulty_write() function (or label). \
The function is of 0x20 bytes and the error has occured at the instruction at 0x14 bytes. \

## Output of objdump
Objdump can be used to analyse the "faulty.ko" file. \
The output of objdump on "faulty.ko" is as follows: \

Disassembly of section .text:   

0000000000000000 <faulty_write>:
   0:	d503245f 	bti	c \
   4:	d2800001 	mov	x1, #0x0                   	// #0 \
   8:	d2800000 	mov	x0, #0x0                   	// #0 \
   c:	d503233f 	paciasp \
  10:	d50323bf 	autiasp \
  14:	b900003f 	str	wzr, [x1] \
  18:	d65f03c0 	ret \
  1c:	d503201f 	nop   

As we can see, the instruction at 0x14 indicates that we are doing a read from content pointed by x1. \
But the instruction at 0x04, the mov instruction is loading 0x0 into x1. \
Therefore, we are attempting to dereference a NULL pointer. \
