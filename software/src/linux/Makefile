SRC:=$(wildcard *.c) $(wildcard McEliece/*.c) $(wildcard McEliece/common/*.c)
HDR:=$(wildcard *.h) $(wildcard McEliece/*.h)

# Need to use nix-shell started from the OS submodules to compile
crypto: $(SRC) $(HDR)
	riscv64-unknown-linux-gnu-gcc -std=gnu99 -march=rv32imac -mabi=ilp32 -Wcast-align=strict -Os -s -ffunction-sections $(SRC) -o crypto -IMcEliece -IMcEliece/common -I. -Wl,-gc-sections -Wl,--strip-all

clean:
	rm crypto
