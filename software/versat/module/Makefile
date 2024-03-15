LINUX_DIR := ../../../submodules/OS/submodules/linux-5.15.98
PWD:=$(shell pwd)
obj-m += versatModule.o

SRC:=testModule.c
#SRC+=iob-versat.c arena.c versat_crypto.c

# Need to use nix-shell started from the OS submodules to compile
all: test module
	bash ./copy.sh

module:
	$(MAKE) ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(LINUX_DIR) M=$(PWD) modules

test:
	riscv64-unknown-linux-gnu-gcc -std=gnu99 -march=rv32imac -mabi=ilp32 -Wcast-align=strict -Os $(SRC) -o testModule -Wl,--strip-debug

clean:
	find . ! -name 'Makefile' -and ! -name '*.c' -and ! -name '*.h' -and ! -name '*.sh' -type f -exec rm -f {} +
	rm versatModule.mod.c
