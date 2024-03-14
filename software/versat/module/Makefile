LINUX_DIR := ../../../submodules/OS/submodules/linux-5.15.98
PWD:=$(shell pwd)
obj-m += versatModule.o

all:
	$(MAKE) ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(LINUX_DIR) M=$(PWD) modules

clean:

