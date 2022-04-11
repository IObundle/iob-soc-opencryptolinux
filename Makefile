ROOT_DIR:=.
include ./config.mk

#
# BUILD EMBEDDED SOFTWARE
#

fw-build:
	make -C $(FIRM_DIR) build-all

fw-clean:
	make -C $(FIRM_DIR) clean-all


#
# EMULATE ON PC
#

pc-emul-build: fw-build
	make -C $(PC_DIR) build

pc-emul-run: pc-emul-build
	make -C $(PC_DIR) run

pc-emul-clean: fw-clean
	make -C $(PC_DIR) clean

pc-emul-test: fw-build
	make -C $(PC_DIR) test

pc-emul-test-clean: fw-clean
	make -C $(PC_DIR) test-clean



#
# SIMULATE RTL
#

sim-build: fw-build
	make -C $(SIM_DIR) build

sim-run: sim-build
	make -C $(SIM_DIR) run

sim-clean: fw-clean
	make -C $(SIM_DIR) clean

sim-test:
	make -C $(SIM_DIR) test

#
# BUILD, LOAD AND RUN ON FPGA BOARD
#

fpga-build: fw-build
	make -C $(BOARD_DIR) build

fpga-clean:
	make -C $(BOARD_DIR) clean

fpga-run:
	make -C $(BOARD_DIR) all TEST_LOG="$(TEST_LOG)"

fpga-test:
	make -C $(BOARD_DIR) test


fpga-test-clean:
	make -C $(BOARD_DIR) test-clean


#
# SYNTHESIZE AND SIMULATE ASIC
#

asic-synth: fw-build
	make -C $(ASIC_DIR) synth

asic-sim-post-synth:
	make -C $(ASIC_DIR) all TEST_LOG="$(TEST_LOG)"

asic-test:
	make -C $(ASIC_DIR) test

asic-clean:
	make -C $(ASIC_DIR) clean-all

#
# COMPILE DOCUMENTS
#
doc-build:
ifeq ($(DOC),pb)
	make fpga-build BOARD=CYCLONEV-GT-DK
	make fpga-build BOARD=AES-KU040-DB-G
endif
	make -C $(DOC_DIR) $(DOC).pdf

doc-clean:
	make -C $(DOC_DIR) clean

doc-test:
	make -C $(DOC_DIR) test

doc-test-clean:
	make -C $(DOC_DIR) test-clean



#
# TEST ALL PLATFORMS
#

test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim:
	make sim-test SIMULATOR=verilator
	make sim-test SIMULATOR=icarus

test-sim-clean:
	make sim-clean SIMULATOR=verilator
	make sim-clean SIMULATOR=icarus

test-fpga:
	make fpga-test BOARD=CYCLONEV-GT-DK
	make fpga-test BOARD=AES-KU040-DB-G

test-fpga-clean:
	make fpga-clean BOARD=CYCLONEV-GT-DK
	make fpga-clean BOARD=AES-KU040-DB-G

test-asic:
	make asic-test ASIC_NODE=umc130
	make asic-test ASIC_NODE=skywater

test-asic-clean:
	make asic-clean ASIC_NODE=umc130
	make asic-clean ASIC_NODE=skywater

test-doc:
	make fpga-clean-all
	make fpga-build-all
	make doc-test DOC=pb
	make doc-test DOC=presentation

test-doc-clean:
	make doc-clean DOC=pb
	make doc-clean DOC=presentation

test: test-clean test-pc-emul test-sim test-fpga test-doc

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean test-doc-clean


debug:
	@echo $(UART_DIR)
	@echo $(CACHE_DIR)


.PHONY: fw-build fw-clean \
	pc-emul-build pc-emul-run pc-emul-clean \
	pc-emul-test pc-emul-test-clean\
	sim-build sim-run sim-clean sim-test sim-test-clean\
	fpga-build fpga-run fpga-clean fpga-test fpga-test-clean\
	asic-synth asic-synth-clean \
	asic-sim-post-synth asic-sim-post-synth-clean \
	asic-test asic-test-clean\
	doc-build doc-clean doc-test doc-test-clean\
	test-pc-emul test-pc-emul-clean\
	test-sim test-sim-clean\
	test-fpga test-fpga-clean\
	test-asic test-asic-clean\
	test-doc test-doc-clean\
	test test-clean
	debug
