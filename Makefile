CORE := iob_soc_opencryptolinux

SIMULATOR ?= icarus
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

INIT_MEM ?= 0
RUN_LINUX ?= 0
USE_EXTMEM := 1
RUN_VERSAT ?= 1
VCD ?= 0

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
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

sim-run:
	nix-shell --run "make setup INIT_MEM=1 VCD=$(VCD)"
	nix-shell --run "make -C ../iob_soc_o* sim-run SIMULATOR=verilator VCD=$(VCD)"

fpga-run:
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM)'
	nix-shell --run 'make -C ../$(CORE)_V*/ fpga-fw-build BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX)'
	make -C ../$(CORE)_V*/ fpga-run BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) 

fpga-run-only:
	cp -r ./software/src ../$(CORE)_V*/software
	make -C ../$(CORE)_V*/ fpga-fw-build fpga-run BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) 

fpga-connect:
	nix-shell --run 'make -C ../$(CORE)_V*/ fpga-fw-build BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX)'
	# Should run under 'bash', running with 'fish' as a shell gives an error
	make -C ../$(CORE)_V*/ fpga-run BOARD=$(BOARD) RUN_LINUX=$(RUN_LINUX) 

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
	make fpga-connect RUN_LINUX=1

.PHONY: sim-test fpga-test test-all test-linux-fpga-connect
