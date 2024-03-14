LINUX_DIR := ../../../submodules/OS/submodules/linux-5.15.98
PWD:=$(shell pwd)
obj-m += versatModule.o

# Need to use nix-shell from the OS submodules
all:
	$(MAKE) ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(LINUX_DIR) M=$(PWD) modules

clean:
	find . ! -name 'Makefile' -and ! -name 'versatModule.c' -type f -exec rm -f {} +

