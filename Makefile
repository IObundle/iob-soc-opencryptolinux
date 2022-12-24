SHELL = bash
export 

#run on external memory implies DDR use
ifeq ($(RUN_EXTMEM),1)
USE_DDR=1
endif


#
# BUILD EMBEDDED SOFTWARE
#
SW_DIR:=./software
FIRM_DIR:=$(SW_DIR)/firmware

#default baud and frequency if not given
BAUD ?=$(SIM_BAUD)
FREQ ?=$(SIM_FREQ)

fw-build:
	$(MAKE) -C $(FIRM_DIR) build-all

fw-clean:
	$(MAKE) -C $(FIRM_DIR) clean-all

fw-debug:
	make -C $(FIRM_DIR) debug

#
# EMULATE ON PC
#

PC_DIR:=$(SW_DIR)/pc-emul
pc-emul-build:
	$(MAKE) fw-build
	$(MAKE) -C $(PC_DIR)

pc-emul-run: pc-emul-build
	$(MAKE) -C $(PC_DIR) run

pc-emul-clean: fw-clean
	$(MAKE) -C $(PC_DIR) clean

pc-emul-test: pc-emul-clean
	$(MAKE) -C $(PC_DIR) test


HW_DIR=./hardware
#
# SIMULATE RTL
#
#default simulator running locally or remotely
SIMULATOR ?=verilator
SIM_DIR=$(HW_DIR)/simulation/$(SIMULATOR)
#default baud and system clock frequency
SIM_BAUD = 115200
SIM_FREQ =100000000
sim-build:
	$(MAKE) fw-build
	$(MAKE) -C $(SIM_DIR) build

sim-run: sim-build
	$(MAKE) -C $(SIM_DIR) run

sim-waves:
	make -C $(SIM_DIR) waves

sim-clean: fw-clean
	$(MAKE) -C $(SIM_DIR) clean

sim-test:
	$(MAKE) -C $(SIM_DIR) test

sim-debug:
	make -C $(SIM_DIR) debug

#
# BUILD, LOAD AND RUN ON FPGA BOARD
#
#default board running locally or remotely
BOARD ?=AES-KU040-DB-G
BOARD_DIR =$(shell find hardware -name $(BOARD))
#default baud and system clock freq for boards
BOARD_BAUD = 115200
#default board frequency
BOARD_FREQ ?=100000000
ifeq ($(BOARD), CYCLONEV-GT-DK)
BOARD_FREQ =50000000
endif

fpga-build:
	make fw-build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)
	make -C $(BOARD_DIR) build

fpga-run: fpga-build
	$(MAKE) -C $(BOARD_DIR) run TEST_LOG="$(TEST_LOG)"

fpga-clean: fw-clean
	$(MAKE) -C $(BOARD_DIR) clean

fpga-veryclean:
	make -C $(BOARD_DIR) veryclean

fpga-debug:
	make -C $(BOARD_DIR) debug

fpga-test:
	$(MAKE) -C $(BOARD_DIR) test

#
# SYNTHESIZE AND SIMULATE ASIC
#

asic-synth:
	$(MAKE) fw-build BAUD=115200
	$(MAKE) -C $(ASIC_DIR) synth

asic-sim-post-synth:
	$(MAKE) -C $(ASIC_DIR) all TEST_LOG="$(TEST_LOG)" BAUD=115200

asic-clean:
	$(MAKE) -C $(ASIC_DIR) clean-all

asic-test:
	$(MAKE) -C $(ASIC_DIR) test

#
# COMPILE DOCUMENTS
#
DOC_DIR=document/$(DOC)
doc-build:
	$(MAKE) -C $(DOC_DIR) $(DOC).pdf

doc-clean:
	$(MAKE) -C $(DOC_DIR) clean

doc-test:
	$(MAKE) -C $(DOC_DIR) test

#
# CLEAN
#

clean: pc-emul-clean sim-clean fpga-clean doc-clean

#
# TEST ALL PLATFORMS
#

test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim:
	$(MAKE) sim-test SIMULATOR=verilator
	$(MAKE) sim-test SIMULATOR=icarus

test-sim-clean:
	$(MAKE) sim-clean SIMULATOR=verilator
	$(MAKE) sim-clean SIMULATOR=icarus

test-fpga:
	$(MAKE) fpga-test BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-test BOARD=AES-KU040-DB-G

test-fpga-clean:
	$(MAKE) fpga-clean BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-clean BOARD=AES-KU040-DB-G

test-doc:
	$(MAKE) fpga-clean BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-clean BOARD=AES-KU040-DB-G
	$(MAKE) fpga-build BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-build BOARD=AES-KU040-DB-G
	$(MAKE) doc-test DOC=pb
	$(MAKE) doc-test DOC=presentation

test-doc-clean:
	$(MAKE) doc-clean DOC=pb
	$(MAKE) doc-clean DOC=presentation

test: test-clean test-pc-emul test-sim test-fpga test-doc

test-clean: test-sim-clean test-fpga-clean test-doc-clean# test-pc-emul-clean 

debug:
	@echo $(UART_DIR)
	@echo $(CACHE_DIR)


.PHONY: fw-build fw-clean fw-debug\
	pc-emul-build pc-emul-run pc-emul-clean pc-emul-test \
	sim-build sim-run sim-clean sim-test \
	fpga-build fpga-run fpga-clean fpga-test \
	doc-build doc-clean doc-test \
	clean \
	test-pc-emul test-pc-emul-clean \
	test-sim test-sim-clean \
	test-fpga test-fpga-clean \
	test-doc test-doc-clean \
	test test-clean \
	debug
