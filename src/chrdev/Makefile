obj-m += chrdev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	sudo insmod chrdev.ko
	sudo dmesg -c

unload:
	sudo rmmod chrmod.ko
	sudo dmesg -c
