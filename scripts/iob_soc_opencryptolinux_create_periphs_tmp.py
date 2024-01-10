#!/usr/bin/env python3
# Creates periphs_tmp.h

import os
import math


def create_periphs_tmp(name, addr_w, peripherals_list, out_file):
    n_slaves = len(peripherals_list) + 1  # +1 for boot_ctr

    template_contents = []
    # Peripheral base addresses
    for instance in peripherals_list:
        template_contents.extend(
            f"#define {instance.name}_BASE (({name.upper()}_{instance.name}<<({addr_w}-4-{name.upper()}_N_SLAVES_W))|(0xf<<({addr_w}-4)))\n"
        )

    # PLIC and CLINT base addresses
    template_contents.extend(
        f"#define CLINT0_BASE (({name.upper()}_N_SLAVES-2<<({addr_w}-4-{name.upper()}_N_SLAVES_W))|(0xf<<({addr_w}-4)))\n"
    )
    template_contents.extend(
        f"#define PLIC0_BASE (({name.upper()}_N_SLAVES-1<<({addr_w}-4-{name.upper()}_N_SLAVES_W))|(0xf<<({addr_w}-4)))\n"
    )

    # Write output file
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    periphs_tmp_file = open(out_file, "w")
    periphs_tmp_file.writelines(template_contents)
    periphs_tmp_file.close()


def get_periphs_hardcoded_addr(name, addr_w, peripherals_list):
    addr_w = int(addr_w)
    n_slaves = len(peripherals_list) + 1  # +1 for boot_ctr
    n_slaves_w = math.ceil(math.log(n_slaves + 2, 2))  # +2 for PLIC and CLINT

    periphs_addr = {}
    # Peripheral base addresses
    for idx, instance in enumerate(peripherals_list):
        periphs_addr[instance.name] = hex(
            (idx << (addr_w - 4 - n_slaves_w)) | (0xF << (addr_w - 4))
        )

    # PLIC and CLINT base addresses
    periphs_addr["CLINT0"] = hex(
        (n_slaves << (addr_w - 4 - n_slaves_w)) | (0xF << (addr_w - 4))
    )
    periphs_addr["PLIC0"] = hex(
        ((n_slaves + 1) << (addr_w - 4 - n_slaves_w)) | (0xF << (addr_w - 4))
    )

    return periphs_addr
