ifneq ($(KERNELRELEASE),)
	obj-m := mma755_i2c.o
else
	KDIR ?= /usr/src/linux-headers-4.8.0-44-generic/

all:
	$(MAKE) -C $(KDIR) M=$$PWD

PHONT: .clean

clean:
	rm *.o *.ko *.mod.c *.order *.symvers

endif
