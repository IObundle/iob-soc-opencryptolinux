HEX+=iob_soc_opencryptolinux_boot.hex iob_soc_opencryptolinux_firmware.hex

ROOT_DIR :=../..
include ../../software/sw_build.mk

VTOP:=iob_soc_opencryptolinux_tb

# SOURCES
ifeq ($(SIMULATOR),verilator)

# verilator top module
VTOP:=iob_soc_opencryptolinux_sim_wrapper

endif

CONSOLE_CMD ?=../../scripts/console.py -L

GRAB_TIMEOUT ?= 3600

TEST_LIST+=test1
test1:
	make -C ../../ fw-clean SIMULATOR=$(SIMULATOR) && make -C ../../ sim-clean SIMULATOR=$(SIMULATOR) && make run SIMULATOR=$(SIMULATOR)

# Include the UUT configuration if iob-soc is used as a Tester
ifneq ($(wildcard uut_build_for_iob_soc_opencryptolinux.mk),)
include uut_build_for_iob_soc_opencryptolinux.mk
endif
