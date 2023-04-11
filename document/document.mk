# (c) 2022-Present IObundle, Lda, all rights reserved
#
# This makefile segment is used at build-time in $(BUILD_DIR)/doc/Makefile
#

#Set ASICSYNTH to 1 to include an ASIC synthesis section
ASICSYNTH?=0

#include implementation results; requires EDA tools
#default is 0 as EDA tools may not be accessible
RESULTS ?= 1
#results for intel FPGA
INT_FAMILY ?=CYCLONEV-GT-DK
#results for xilinx fpga
XIL_FAMILY ?=AES-KU040-DB-G

#tests
TEST_LIST+=test1
test1: pb.pdf
	cat pb.aux >> test.log

TEST_LIST+=test2
test2: presentation.pdf
	cat presentation.aux >> test.log

.PHONY: $(TEST_LIST)
