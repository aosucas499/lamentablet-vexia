
# testing build module edt-ft5x06
# Makefile for the touchscreen drivers.
#


obj-m	+= ft5x06_ts.o focaltech_ctl.c ft5x06_ex_fun.c
all:

		make -C /lib/modules/$(shell uname -r)/build M=${PWD} modules

clean:
		make -C /lib/modules/$(shell uname -r)/build M=${PWD} clean


