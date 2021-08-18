
# testing build module edt-ft5x06
# Makefile for the touchscreen drivers.
#


obj-m	+= edt-ft5x06.o

all:

		make -C /lib/modules/$(shell uname -r)/build M=${PWD} modules

clean:
		make -C /lib/modules/$(shell uname -r)/build M=${PWD} clean

