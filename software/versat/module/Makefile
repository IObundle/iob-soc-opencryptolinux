LINUX_DIR := ../../../submodules/OS/submodules/linux-5.15.98
PWD:=$(shell pwd)
obj-m += versat.o

# Need to use nix-shell started from the OS submodules to compile
all: module
	bash ./copy.sh

module:
	$(MAKE) ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(LINUX_DIR) M=$(PWD) modules

clean:
	find . ! -name 'Makefile' -and ! -name '*.c' -and ! -name '*.h' -and ! -name '*.sh' -and ! -name '*.ko' -type f -exec rm -f {} +
	rm -f versat.mod.c
