obj-m += cryptodevice.o

all:
	make cryptodevice
	make programa

cryptodevice:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

programa: programa.c
	gcc programa.c -o programa

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

delete:
	rm cryptodevice.ko
	rm cryptodevice.mod.c
	rm cryptodevice.mod.o
	rm cryptodevice.o
	rm Module.symvers
	rm modules.order
	rm programa
