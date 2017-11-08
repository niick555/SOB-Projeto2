#
# Makefile for the Linux minix filesystem routines.
#

obj-$(CONFIG_MINIX_FS) += minix_module.o

minix_module-objs := bitmap.o itree_v1.o itree_v2.o namei.o inode.o file.o dir.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

delete:
	find . -name "*.ko" -type f -delete
	find . -name "*.o" -type f -delete
	find . -name "*.mod.c" -type f -delete
	find . -name "*.symvers" -type f -delete
	find . -name "*.order" -type f -delete