HEX+=iob_soc_opencryptolinux_boot.hex iob_soc_opencryptolinux_firmware.hex
include ../../software/sw_build.mk

VTOP:=iob_soc_opencryptolinux_tb

# get header files (needed for iob_soc_opencryptolinux_tb.v)
VHDR+=iob_uart_conf.vh
iob_uart_conf.vh: ../src/iob_uart16550_conf.vh
	cp $< $@
	sed -i 's/UART16550/UART/g' $@
VHDR+=iob_uart_swreg_def.vh
iob_uart_swreg_def.vh: ../src/iob_uart16550_swreg_def.vh
	cp $< $@
	sed -i 's/UART16550/UART/g' $@

# SOURCES
ifeq ($(SIMULATOR),verilator)

# get header files (needed for iob_soc_opencryptolinux_tb.cpp)
VHDR+=iob_uart_swreg.h
iob_uart_swreg.h: ../../software/src/iob_uart16550_swreg.h
	cp $< $@
	sed -i 's/UART16550/UART/g' $@

# verilator top module
VTOP:=iob_soc_opencryptolinux_sim_wrapper

endif

CONSOLE_CMD=../../scripts/console.py -L

TEST_LIST+=test1
test1:
	make -C ../../ fw-clean SIMULATOR=$(SIMULATOR) && make -C ../../ sim-clean SIMULATOR=$(SIMULATOR) && make run SIMULATOR=$(SIMULATOR)
