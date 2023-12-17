#!/usr/bin/env python3

import sys

ROOT_DIR = sys.argv[1]
if len(sys.argv) > 2:
    RUN_LINUX = sys.argv[2]
else:
    RUN_LINUX = "0"

# Generate "iob_mem.config" according to which binary firmware the SoC should load to RAM

iob_mem_file = f"{ROOT_DIR}/hardware/iob_mem.config"
with open(iob_mem_file, "w") as file:
    if RUN_LINUX == "1":
        file.write(
            "fw_jump.bin 0\nImage 400000\niob_soc.dtb F80000\nrootfs.cpio.gz 1000000"
        )
    else:
        file.write("iob_soc_opencryptolinux_firmware.bin 0")


# Fixes existing bsp.h and bsp.vh for Simulation

bsp_file = f"{ROOT_DIR}/software/src/bsp.h"
with open(bsp_file, "r") as file:
    content = file.read()

if "define SIMULATION 1" in content:
    if RUN_LINUX == "1":
        bsp_file = f"{ROOT_DIR}/hardware/simulation/src/bsp.vh"
        with open(bsp_file, "w") as file:
            file.write(
                "`define BAUD 115200\n`define FREQ 100000000\n`define DDR_DATA_W 32\n`define DDR_ADDR_W 26\n`define SIMULATION 1"
            )

        bsp_file = f"{ROOT_DIR}/software/src/bsp.h"
        with open(bsp_file, "w") as file:
            file.write(
                "#define BAUD 115200\n#define FREQ 100000000\n#define DDR_DATA_W 32\n#define DDR_ADDR_W 26\n#define SIMULATION 1"
            )
    else:
        bsp_file = f"{ROOT_DIR}/hardware/simulation/src/bsp.vh"
        with open(bsp_file, "w") as file:
            file.write(
                "`define BAUD 3000000\n`define FREQ 100000000\n`define DDR_DATA_W 32\n`define DDR_ADDR_W 26\n`define SIMULATION 1"
            )

        bsp_file = f"{ROOT_DIR}/software/src/bsp.h"
        with open(bsp_file, "w") as file:
            file.write(
                "#define BAUD 3000000\n#define FREQ 100000000\n#define DDR_DATA_W 32\n#define DDR_ADDR_W 26\n#define SIMULATION 1"
            )
