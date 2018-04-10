ifneq ($(KERNELRELEASE),)
	obj-m := mma755_i2c.o
else
	KDIR ?= /usr/src/linux-headers-`uname -r`/

all:
	$(MAKE) -C $(KDIR) M=$$PWD

PHONT: .clean

clean:
	rm *.o *.ko *.mod.c *.order *.symvers

endif
