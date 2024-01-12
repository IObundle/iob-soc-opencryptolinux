#!/usr/bin/env python3

import sys

ROOT_DIR = sys.argv[1]
SOC_NAME = sys.argv[2]
if len(sys.argv) > 3:
    RUN_LINUX = sys.argv[3]
else:
    RUN_LINUX = "0"


# If line contains "line_content", replace entire line with "new_line_content"
# If "line_content" is empty, append "new_line_content" to end of file
def replace_line(filename, line_content, new_line_content):
    with open(filename, "r") as file:
        lines = file.readlines()
    if line_content:
        for i in range(len(lines)):
            if line_content in lines[i]:
                lines[i] = new_line_content
    else:
        lines.append(new_line_content)

    with open(filename, "w") as file:
        file.writelines(lines)


# Generate "iob_mem.config" according to which binary firmware the SoC should load to RAM

iob_mem_file = f"{ROOT_DIR}/hardware/{SOC_NAME}_mem.config"
with open(iob_mem_file, "w") as file:
    if RUN_LINUX == "1":
        file.write(
            "fw_jump.bin 0\nImage 400000\niob_soc.dtb F80000\nrootfs.cpio.gz 1000000"
        )
    else:
        file.write(f"{SOC_NAME}_firmware.bin 0")


# Fixes existing bsp.h and bsp.vh for Simulation

bsp_file = f"{ROOT_DIR}/software/src/bsp.h"
with open(bsp_file, "r") as file:
    content = file.read()

if "define SIMULATION 1" in content:
    if RUN_LINUX == "1":
        # bsp_file = f"{ROOT_DIR}/hardware/simulation/src/bsp.vh"
        # replace_line(bsp_file, "`define BAUD", "`define BAUD 115200\n")

        # bsp_file = f"{ROOT_DIR}/software/src/bsp.h"
        # replace_line(bsp_file, "#define BAUD", "#define BAUD 115200\n")

        conf_file = f"{ROOT_DIR}/hardware/src/{SOC_NAME}_conf.vh"
        replace_line(conf_file, "", "`define IOB_SOC_OPENCRYPTOLINUX_RUN_LINUX 1\n")

        conf_file = f"{ROOT_DIR}/software/src/{SOC_NAME}_conf.h"
        replace_line(
            conf_file,
            "#define H_IOB_SOC_OPENCRYPTOLINUX_CONF_H",
            "#define H_IOB_SOC_OPENCRYPTOLINUX_CONF_H\n#define IOB_SOC_OPENCRYPTOLINUX_RUN_LINUX 1\n",
        )
    else:
        # bsp_file = f"{ROOT_DIR}/hardware/simulation/src/bsp.vh"
        # replace_line(bsp_file, "`define BAUD", "`define BAUD 3000000\n")

        # bsp_file = f"{ROOT_DIR}/software/src/bsp.h"
        # replace_line(bsp_file, "#define BAUD", "#define BAUD 3000000\n")

        conf_file = f"{ROOT_DIR}/hardware/src/{SOC_NAME}_conf.vh"
        replace_line(conf_file, "`define IOB_SOC_OPENCRYPTOLINUX_RUN_LINUX", "")

        conf_file = f"{ROOT_DIR}/software/src/{SOC_NAME}_conf.h"
        replace_line(conf_file, "#define IOB_SOC_OPENCRYPTOLINUX_RUN_LINUX", "")
