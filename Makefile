obj-m += conv_mod.o
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) modules
	sudo insmod conv_mod.ko
	sudo chmod 666 /dev/conv_d
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) clean
