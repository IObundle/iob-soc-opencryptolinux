CORE := iob_soc_opencryptolinux

SIMULATOR ?= icarus
BOARD ?= AES-KU040-DB-G
GRAB_TIMEOUT ?= 1800

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

INIT_MEM ?= 0
RUN_LINUX ?= 1
USE_EXTMEM := 1

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

ifeq ($(RUN_LINUX),1)
SETUP_ARGS += RUN_LINUX
endif

setup:
	make build-setup SETUP_ARGS="$(SETUP_ARGS)"

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
	nix-shell --run 'BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) make -C ../$(CORE)_V*/ fpga-fw-build'
	BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) GRAB_TIMEOUT=$(GRAB_TIMEOUT) make -C ../$(CORE)_V*/ fpga-run 

fpga-connect:
	nix-shell --run 'BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) make -C ../$(CORE)_V*/ fpga-fw-build'
#   Should run under 'bash', running with 'fish' as a shell gives an error
	BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) GRAB_TIMEOUT=$(GRAB_TIMEOUT) make -C ../$(CORE)_V*/ fpga-run 

fpga-test:
	# IOb-SoC-Opencryptolinux always uses external memory
	make clean setup fpga-run INIT_MEM=0

test-all:
	make sim-test
	make fpga-test BOARD=CYCLONEV-GT-DK
	make fpga-test BOARD=AES-KU040-DB-G
	make clean && make setup && make -C ../iob_soc_opencryptolinux_V*/ doc-test

.PHONY: sim-test fpga-test test-all
