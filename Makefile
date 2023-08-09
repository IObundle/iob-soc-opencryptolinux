CORE := iob_soc_opencryptolinux

DISABLE_LINT:=1

include submodules/LIB/setup.mk

INIT_MEM ?= 1
RUN_LINUX ?= 1

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

ifeq ($(RUN_LINUX),1)
SETUP_ARGS += RUN_LINUX
endif

sim-test:
	# IOb-SoC-Opencryptolinux only supports USE_EXTMEM=1
	#make clean && make setup && make -C ../iob_soc_opencryptolinux_V*/ sim-test
	#make clean && make setup INIT_MEM=0 && make -C ../iob_soc_opencryptolinux_V*/ sim-test
	make clean && make setup USE_EXTMEM=1 && make -C ../iob_soc_opencryptolinux_V*/ sim-test
	make clean && make setup INIT_MEM=0 USE_EXTMEM=1 && make -C ../iob_soc_opencryptolinux_V*/ sim-test

fpga-test:
	# IOb-SoC-Opencryptolinux only supports USE_EXTMEM=1
	#make clean && make setup && make -C ../iob_soc_opencryptolinux_V*/ fpga-test
	#make clean && make setup INIT_MEM=0 && make -C ../iob_soc_opencryptolinux_V*/ fpga-test
	make clean && make setup INIT_MEM=0 USE_EXTMEM=1 && make -C ../iob_soc_opencryptolinux_V*/ fpga-test

test-all:
	make clean && make setup && make -C ../iob_soc_opencryptolinux_V*/ pc-emul-test
	#make sim-test SIMULATOR=icarus
	make sim-test SIMULATOR=verilator
	make fpga-test BOARD=CYCLONEV-GT-DK
	make fpga-test BOARD=AES-KU040-DB-G
	make clean && make setup && make -C ../iob_soc_opencryptolinux_V*/ doc-test

.PHONY: sim-test fpga-test test-all
