KERNELDIR ?= /lib/modules/6.8.0-85-generic/build
PWD := $(shell pwd)

obj-m += module2.o

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean