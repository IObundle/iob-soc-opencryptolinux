#!/usr/bin/env python3
# Creates periphs_tmp.h

import os


def create_periphs_tmp(addr_w, peripherals_list, out_file):
    n_slaves = len(peripherals_list) + 1  # +1 for boot_ctr

    template_contents = []
    # Peripheral base addresses
    for instance in peripherals_list:
        template_contents.extend(
            f"#define {instance.name}_BASE ((IOB_SOC_OPENCRYPTOLINUX_{instance.name}<<({addr_w}-4-IOB_SOC_OPENCRYPTOLINUX_N_SLAVES_W))|(0xf<<({addr_w}-4)))\n"
        )

    # PLIC and CLINT base addresses
    template_contents.extend(
        f"#define CLINT0_BASE ((IOB_SOC_OPENCRYPTOLINUX_N_SLAVES-2<<({addr_w}-4-IOB_SOC_OPENCRYPTOLINUX_N_SLAVES_W))|(0xf<<({addr_w}-4)))\n"
    )
    template_contents.extend(
        f"#define PLIC0_BASE ((IOB_SOC_OPENCRYPTOLINUX_N_SLAVES-1<<({addr_w}-4-IOB_SOC_OPENCRYPTOLINUX_N_SLAVES_W))|(0xf<<({addr_w}-4)))\n"
    )

    # Write output file
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    periphs_tmp_file = open(out_file, "w")
    periphs_tmp_file.writelines(template_contents)
    periphs_tmp_file.close()
