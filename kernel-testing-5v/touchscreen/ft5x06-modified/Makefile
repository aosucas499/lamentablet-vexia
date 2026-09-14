# Makefile - ft5x06 BayTrail touchscreen driver
obj-m := ft5x06-ts.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	cp ft5x06-ts.ko /lib/modules/$(shell uname -r)/kernel/drivers/input/touchscreen/
	depmod -a

