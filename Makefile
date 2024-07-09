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

ifeq ($(DMA_DEMO),1)
SETUP_ARGS += OCL_DMA_DEMO
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
	-ln -fs minicom_test1.txt $(BUILD_DIR)/hardware/fpga/minicom_linux_script.txt
	make fpga-connect RUN_LINUX=1 BOOT_FLOW=$(BOOT_FLOW)

dma-linux-fpga-connect: build_dir_name
	-rm $(BUILD_DIR)/hardware/fpga/test.log
	-ln -fs minicom_dma_test.txt $(BUILD_DIR)/hardware/fpga/minicom_linux_script.txt
	make fpga-connect RUN_LINUX=1 BOOT_FLOW=$(BOOT_FLOW)

.PHONY: setup sim-run sim-test fpga-test test-all test-linux-fpga-connect dma-linux-fpga-connect

#
# Linux targets
#

LINUX_OS_DIR ?= submodules/OS
# Relative path from OS directory to OpenCryptoLinux (OCL) directory
REL_OS2OCL :=`realpath . --relative-to=$(LINUX_OS_DIR)`

ifeq ($(DMA_DEMO),1)
DTS_FILE_STR := DTS_FILE=$(REL_OS2OCL)/software/iob_soc_dma_demo.dts
endif

build-linux-dts:
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-dts MACROS_FILE=$(REL_OS2OCL)/hardware/simulation/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/simulation $(DTS_FILE_STR)'
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-dts MACROS_FILE=$(REL_OS2OCL)/hardware/fpga/vivado/AES-KU040-DB-G/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/fpga/vivado/AES-KU040-DB-G $(DTS_FILE_STR)'
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-dts MACROS_FILE=$(REL_OS2OCL)/hardware/fpga/quartus/CYCLONEV-GT-DK/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/fpga/quartus/CYCLONEV-GT-DK $(DTS_FILE_STR)'

build-linux-opensbi:
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-opensbi MACROS_FILE=$(REL_OS2OCL)/hardware/simulation/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/simulation'
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-opensbi MACROS_FILE=$(REL_OS2OCL)/hardware/fpga/vivado/AES-KU040-DB-G/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/fpga/vivado/AES-KU040-DB-G'
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-opensbi MACROS_FILE=$(REL_OS2OCL)/hardware/fpga/quartus/CYCLONEV-GT-DK/linux_build_macros.txt OS_BUILD_DIR=$(REL_OS2OCL)/hardware/fpga/quartus/CYCLONEV-GT-DK'

.PHONY: build-linux-dts build-linux-opensbi

COMBINED_BUILDROOT_DIR=..
combine-buildroot:
	rm -rf $(COMBINED_BUILDROOT_DIR)/buildroot
	cp -r $(LINUX_OS_DIR)/software/buildroot $(COMBINED_BUILDROOT_DIR)/
	cp -r ./software/buildroot $(COMBINED_BUILDROOT_DIR)/

ifeq ($(DMA_DEMO),1)
BUILDROOT_DEPS += build-linux-dma-demo
endif

build-linux-buildroot: combine-buildroot build-linux-drivers $(BUILDROOT_DEPS)
	make -C $(LINUX_OS_DIR) build-buildroot OS_SUBMODULES_DIR=$(REL_OS2OCL)/.. OS_SOFTWARE_DIR=../`realpath $(COMBINED_BUILDROOT_DIR) --relative-to=..` OS_BUILD_DIR=$(REL_OS2OCL)/software/src

.PHONY: combine-buildroot build-linux-buildroot

# System peripherals
ifeq ($(DMA_DEMO),1)
MODULE_NAMES += iob_dma
MODULE_NAMES += iob_axistream_in
MODULE_NAMES += iob_axistream_out
endif
MODULE_NAMES += iob_timer

build-driver-headers:
	@$(foreach module,$(MODULE_NAMES), \
		./$(LINUX_OS_DIR)/scripts/drivers.py $(module) -o `realpath $(COMBINED_BUILDROOT_DIR)`/buildroot/board/IObundle/iob-soc/rootfs-overlay/root/dma_demo/; \
	)

.PHONY: build-driver-headers

build-linux-drivers:
	@$(foreach module,$(MODULE_NAMES), \
		printf "\n\n\nMaking $(module)\n\n\n"; \
		$(eval MODULE_LINUX_DIR=$(shell realpath $(shell find . -type f -name '$(module).py' -printf '%h' -quit))/software/linux) \
		nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-linux-drivers \
			MODULE_DRIVER_DIR=$(MODULE_LINUX_DIR)/drivers \
			OS_SUBMODULES_DIR=$(REL_OS2OCL)/.. \
			CALLING_DIR=`realpath $(CURDIR)` \
			MODULE_NAME=$(module) \
			ROOTFS_OVERLAY_DIR=`realpath $(COMBINED_BUILDROOT_DIR)`/buildroot/board/IObundle/iob-soc/rootfs-overlay/ \
			PYTHON_DIR=`realpath $(LIB_DIR)`/scripts'; \
	)


clean-linux-drivers:
	@$(foreach module,$(MODULE_NAMES), \
		$(eval MODULE_LINUX_DIR=$(shell realpath $(shell find . -type f -name '$(module).py' -printf '%h' -quit))/software/linux) \
		nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) clean-linux-drivers \
			MODULE_DRIVER_DIR=$(MODULE_LINUX_DIR)/drivers \
			OS_SUBMODULES_DIR=$(REL_OS2OCL)/.. \
			CALLING_DIR=`realpath $(CURDIR)` \
			MODULE_NAME=$(module) \
			ROOTFS_OVERLAY_DIR=`realpath $(COMBINED_BUILDROOT_DIR)`/buildroot/board/IObundle/iob-soc/rootfs-overlay/ \
			PYTHON_DIR=`realpath $(LIB_DIR)`/scripts'; \
	)

.PHONY: build-linux-drivers clean-linux-drivers

build-linux-kernel:
	-rm ../linux-5.15.98/arch/riscv/boot/Image
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'make -C $(LINUX_OS_DIR) build-linux-kernel OS_SUBMODULES_DIR=$(REL_OS2OCL)/.. OS_SOFTWARE_DIR=../`realpath . --relative-to=..`/software OS_BUILD_DIR=$(REL_OS2OCL)/software/src'

.PHONY: build-linux-kernel

# Main target to build all linux related files
build-linux-files:
	make build-linux-dts
	make build-linux-opensbi
	make build-linux-kernel
	make build-linux-buildroot

.PHONY: build-linux-files

#
# Linux software targets
#

INCLUDE = -I.
SRC = *.c
FLAGS = -Wall -O2
FLAGS += -Werror
#FLAGS += -static
FLAGS += -march=rv32imac
FLAGS += -mabi=ilp32
BIN = dma_demo
CC = riscv64-unknown-linux-gnu-gcc
build-linux-dma-demo: build-driver-headers
	nix-shell $(LINUX_OS_DIR)/default.nix --run 'cd $(COMBINED_BUILDROOT_DIR)/buildroot/board/IObundle/iob-soc/rootfs-overlay/root/dma_demo/ && \
	$(CC) $(FLAGS) $(INCLUDE) -o $(BIN) $(SRC)'

.PHONY: build-linux-dma-demo
