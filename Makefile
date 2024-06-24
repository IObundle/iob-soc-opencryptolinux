CORE := iob_soc_opencryptolinux

SIMULATOR ?= icarus
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

INIT_MEM ?= 0
RUN_LINUX ?= 0
USE_EXTMEM := 1
BOOT_FLOW ?= CONSOLE_TO_EXTMEM

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

setup:
	make build-setup SETUP_ARGS="$(SETUP_ARGS)"

sim-run:
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) && make -C ../$(CORE)_V*/ sim-run SIMULATOR=$(SIMULATOR)'

sim-test-linux:
	nix-shell --run "make clean"
	nix-shell --run "make setup INIT_MEM=1"
	nix-shell --run "make -C ../iob_soc_o* sim-run SIMULATOR=verilator RUN_LINUX=1"

sim-test:
	nix-shell --run "make clean"
	nix-shell --run "make setup INIT_MEM=1"
	nix-shell --run "make -C ../iob_soc_o* sim-run"
	nix-shell --run "make clean"
	nix-shell --run "make setup INIT_MEM=0"
	nix-shell --run "make -C ../iob_soc_o* sim-run SIMULATOR=verilator"

sim-test-spi:
	nix-shell --run "make -C submodules/SPI/ clean build-setup && make -C submodules/iob_spi_master_V0.10/ sim-run"
	nix-shell --run "make -C submodules/SPI/ clean"

fpga-run:
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM)'
	nix-shell --run 'make -C ../$(CORE)_V*/ fpga-fw-build BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX)'
	make -C ../$(CORE)_V*/ fpga-run BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) BOOT_FLOW=$(BOOT_FLOW)

fpga-build:
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM)'
	nix-shell --run 'make -C ../$(CORE)_V*/ fpga-fw-build BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX)'
	make -C ../$(CORE)_V*/ fpga-build BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) 

fpga-connect:
	nix-shell --run 'make -C ../$(CORE)_V*/ fpga-fw-build BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX)'
	# Should run under 'bash', running with 'fish' as a shell gives an error
	make -C ../$(CORE)_V*/ fpga-run BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) BOOT_FLOW=$(BOOT_FLOW)

fpga-test:
	make clean setup fpga-run INIT_MEM=0

test-all:
	make sim-test
	make fpga-test BOARD=CYCLONEV-GT-DK
	make fpga-test BOARD=AES-KU040-DB-G
	make clean && make setup && make -C ../iob_soc_opencryptolinux_V*/ doc-test

test-linux-fpga-connect: build_dir_name
	-rm $(BUILD_DIR)/hardware/fpga/test.log
	-ln -s minicom_test1.txt $(BUILD_DIR)/hardware/fpga/minicom_linux_script.txt
	make fpga-connect RUN_LINUX=1 BOOT_FLOW=$(BOOT_FLOW)

dma-linux-fpga-connect: build_dir_name
	-rm $(BUILD_DIR)/hardware/fpga/test.log
	-ln -s minicom_dma_test.txt $(BUILD_DIR)/hardware/fpga/minicom_linux_script.txt
	make fpga-connect RUN_LINUX=1 BOOT_FLOW=$(BOOT_FLOW)

.PHONY: setup sim-run sim-test fpga-test test-all test-linux-fpga-connect dma-linux-fpga-connect

LINUX_OS_DIR ?= submodules/OS
# Relative path from OS directory to OpenCryptoLinux (OCL) directory
REL_OS2OCL :=`realpath . --relative-to=$(LINUX_OS_DIR)`

build-linux-dts:
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-dts MACROS_FILE=$(REL_OS2OCL)/hardware/simulation/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/simulation OS_SOFTWARE_DIR=$(REL_OS2OCL)/software'
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-dts MACROS_FILE=$(REL_OS2OCL)/hardware/fpga/vivado/AES-KU040-DB-G/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/fpga/vivado/AES-KU040-DB-G OS_SOFTWARE_DIR=$(REL_OS2OCL)/software'
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-dts MACROS_FILE=$(REL_OS2OCL)/hardware/fpga/quartus/CYCLONEV-GT-DK/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/fpga/quartus/CYCLONEV-GT-DK OS_SOFTWARE_DIR=$(REL_OS2OCL)/software'

.PHONY: build-linux-dts

MODULE_NAMES += iob_dma
MODULE_NAMES += iob_axistream_in
MODULE_NAMES += iob_axistream_out

build-driver-headers:
	@$(foreach module,$(MODULE_NAMES), \
		./$(LINUX_OS_DIR)/scripts/drivers.py $(module) -o software/tests/dma/src/; \
	)

.PHONY: build-driver-headers

INCLUDE = -I.
SRC = *.c
FLAGS = -Wall -O2
FLAGS += -Werror
#FLAGS += -static
FLAGS += -march=rv32imac
FLAGS += -mabi=ilp32
BIN = ../dma_demo
CC = riscv64-unknown-linux-gnu-gcc
build-linux-dma-demo: build-driver-headers
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'cd software/tests/dma/src && \
	$(CC) $(FLAGS) $(INCLUDE) -o $(BIN) $(SRC)'

.PHONY: build-linux-dma-demo
