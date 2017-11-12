obj-m += cryptodevice.o

all:
	make cryptodevice

cryptodevice:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

delete:
	rm cryptodevice.ko
	rm cryptodevice.mod.c
	rm cryptodevice.mod.o
	rm cryptodevice.o
	rm Module.symvers
	rm modules.order