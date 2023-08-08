#########################################
#            Embedded targets           #
#########################################
ROOT_DIR ?=..
# Local embedded makefile settings for custom bootloader and firmware targets.

#Function to obtain parameter named $(1) in verilog header file located in $(2)
#Usage: $(call GET_MACRO,<param_name>,<vh_path>)
GET_MACRO = $(shell grep "define $(1)" $(2) | rev | cut -d" " -f1 | rev)

#Function to obtain parameter named $(1) from iob_soc_opencryptolinux_conf.vh
GET_IOB_SOC_OPENCRYPTOLINUX_CONF_MACRO = $(call GET_MACRO,IOB_SOC_OPENCRYPTOLINUX_$(1),$(ROOT_DIR)/hardware/src/iob_soc_opencryptolinux_conf.vh)

iob_soc_opencryptolinux_boot.hex: ../../software/iob_soc_opencryptolinux_boot.bin
	../../scripts/makehex.py $< $(call GET_IOB_SOC_OPENCRYPTOLINUX_CONF_MACRO,BOOTROM_ADDR_W) > $@

#OS
ifeq ($(call GET_IOB_SOC_OPENCRYPTOLINUX_CONF_MACRO,RUN_LINUX),1)
OS_DIR = ../../software/src
OPENSBI_DIR = $(OS_DIR)/fw_jump.bin
DTB_DIR = $(OS_DIR)/iob_soc.dtb
DTB_ADDR:=00F80000
LINUX_DIR = $(OS_DIR)/Image
LINUX_ADDR:=00400000
ROOTFS_DIR = $(OS_DIR)/rootfs.cpio.gz
ROOTFS_ADDR:=01000000
FIRM_ARGS = $(OPENSBI_DIR)
FIRM_ARGS += $(DTB_DIR) $(DTB_ADDR)
FIRM_ARGS += $(LINUX_DIR) $(LINUX_ADDR)
FIRM_ARGS += $(ROOTFS_DIR) $(ROOTFS_ADDR)
FIRM_ADDR_W = $(call GET_IOB_SOC_OPENCRYPTOLINUX_CONF_MACRO,OS_ADDR_W)
else
FIRM_ARGS = $<
FIRM_ADDR_W = $(call GET_IOB_SOC_OPENCRYPTOLINUX_CONF_MACRO,SRAM_ADDR_W)
endif
iob_soc_opencryptolinux_firmware.hex: iob_soc_opencryptolinux_firmware.bin
	../../scripts/makehex.py $(FIRM_ARGS) $(FIRM_ADDR_W) > $@
	../../scripts/hex_split.py iob_soc_opencryptolinux_firmware .

iob_soc_opencryptolinux_firmware.bin: ../../software/iob_soc_opencryptolinux_firmware.bin
	cp $< $@

../../software/%.bin:
	make -C ../../ fw-build

UTARGETS+=build_iob_soc_opencryptolinux_software

TEMPLATE_LDS=src/$@.lds

IOB_SOC_OPENCRYPTOLINUX_INCLUDES=-I. -Isrc 

IOB_SOC_OPENCRYPTOLINUX_LFLAGS=-Wl,-Bstatic,-T,$(TEMPLATE_LDS),--strip-debug

# FIRMWARE SOURCES
IOB_SOC_OPENCRYPTOLINUX_FW_SRC=src/iob_soc_opencryptolinux_firmware.S
IOB_SOC_OPENCRYPTOLINUX_FW_SRC+=src/iob_soc_opencryptolinux_firmware.c
IOB_SOC_OPENCRYPTOLINUX_FW_SRC+=src/printf.c
IOB_SOC_OPENCRYPTOLINUX_FW_SRC+=src/iob_str.c
# PERIPHERAL SOURCES
IOB_SOC_OPENCRYPTOLINUX_FW_SRC+=$(wildcard src/iob-*.c)
IOB_SOC_OPENCRYPTOLINUX_FW_SRC+=$(filter-out %_emul.c, $(wildcard src/*swreg*.c))

# BOOTLOADER SOURCES
IOB_SOC_OPENCRYPTOLINUX_BOOT_SRC+=src/iob_soc_opencryptolinux_boot.S
IOB_SOC_OPENCRYPTOLINUX_BOOT_SRC+=src/iob_soc_opencryptolinux_boot.c
IOB_SOC_OPENCRYPTOLINUX_BOOT_SRC+=$(filter-out %_emul.c, $(wildcard src/iob*uart*.c))
IOB_SOC_OPENCRYPTOLINUX_BOOT_SRC+=$(filter-out %_emul.c, $(wildcard src/iob*cache*.c))

build_iob_soc_opencryptolinux_software: iob_soc_opencryptolinux_firmware iob_soc_opencryptolinux_boot

iob_soc_opencryptolinux_firmware:
	make $@.elf INCLUDES="$(IOB_SOC_OPENCRYPTOLINUX_INCLUDES)" LFLAGS="$(IOB_SOC_OPENCRYPTOLINUX_LFLAGS) -Wl,-Map,$@.map" SRC="$(IOB_SOC_OPENCRYPTOLINUX_FW_SRC)" TEMPLATE_LDS="$(TEMPLATE_LDS)"

iob_soc_opencryptolinux_boot:
	make $@.elf INCLUDES="$(IOB_SOC_OPENCRYPTOLINUX_INCLUDES)" LFLAGS="$(IOB_SOC_OPENCRYPTOLINUX_LFLAGS) -Wl,-Map,$@.map" SRC="$(IOB_SOC_OPENCRYPTOLINUX_BOOT_SRC)" TEMPLATE_LDS="$(TEMPLATE_LDS)"


.PHONE: build_iob_soc_opencryptolinux_software

# Include the UUT configuration if iob-soc is used as a Tester
ifneq ($(wildcard $(ROOT_DIR)/software/uut_build_for_iob_soc_opencryptolinux.mk),)
include $(ROOT_DIR)/software/uut_build_for_iob_soc_opencryptolinux.mk
endif

#########################################
#         PC emulation targets          #
#########################################
# Local pc-emul makefile settings for custom pc emulation targets.

# Include directory with iob_soc_opencryptolinux_system.h
EMUL_INCLUDE+=-I. -Isrc

# SOURCES
EMUL_SRC+=src/iob_soc_opencryptolinux_firmware.c
EMUL_SRC+=src/printf.c
EMUL_SRC+=src/iob_str.c

# PERIPHERAL SOURCES
EMUL_SRC+=$(wildcard src/iob-*.c)

EMUL_TEST_LIST+=pcemul_test1
pcemul_test1:
	make run_emul TEST_LOG="> test.log"


CLEAN_LIST+=clean1
clean1:
	@rm -rf iob_soc_opencryptolinux_conf.h
