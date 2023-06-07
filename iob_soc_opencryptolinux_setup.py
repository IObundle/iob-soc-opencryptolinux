#!/usr/bin/env python3

import os, sys

sys.path.insert(0, os.getcwd() + "/submodules/LIB/scripts")

import setup
from mk_configuration import update_define

sys.path.insert(0, os.path.dirname(__file__) + "/submodules/IOBSOC/scripts")
import iob_soc

name = "iob_soc_opencryptolinux"
version = "V0.70"
flows = "pc-emul emb sim doc fpga"
if setup.is_top_module(sys.modules[__name__]):
    setup_dir = os.path.dirname(__file__)
    build_dir = f"../{name}_{version}"

submodules = {
    "hw_setup": {
        "headers": [
            "iob_wire",
            "axi_wire",
            "axi_m_m_portmap",
            "axi_m_port",
            "axi_m_m_portmap",
            "axi_m_portmap",
        ],
        "modules": [
            "VEXRISCV",
            "CACHE",
            "UART16550",
            "PLIC",
            "CLINT",
            "iob_merge",
            "iob_split",
            "iob_rom_sp.v",
            "iob_ram_dp_be.v",
            "iob_ram_dp_be_xil.v",
            "iob_pulse_gen.v",
            "iob_counter.v",
            "iob_ram_2p_asym.v",
            "iob_reg.v",
            "iob_reg_re.v",
            "iob_ram_sp_be.v",
            "iob_ram_dp.v",
            "iob_reset_sync",
        ],
    },
    "sim_setup": {
        "headers": ["axi_s_portmap", "iob_tasks.vh"],
        "modules": ["axi_ram.v"],
    },
    "sw_setup": {
        "headers": [],
        "modules": ["CACHE", "UART16550", "PLIC", "CLINT", "iob_str"],
    },
}

blocks = [
    {
        "name": "cpu",
        "descr": "CPU module",
        "blocks": [
            {"name": "cpu", "descr": "Vexriscv CPU"},
        ],
    },
    {
        "name": "bus_split",
        "descr": "Split modules for buses",
        "blocks": [
            {
                "name": "ibus_split",
                "descr": "Split CPU instruction bus into internal and external memory buses",
            },
            {
                "name": "dbus_split",
                "descr": "Split CPU data bus into internal and external memory buses",
            },
            {
                "name": "int_dbus_split",
                "descr": "Split internal data bus into internal memory and peripheral buses",
            },
            {
                "name": "pbus_split",
                "descr": "Split peripheral bus into a bus for each peripheral",
            },
        ],
    },
    {
        "name": "memories",
        "descr": "Memory modules",
        "blocks": [
            {"name": "int_mem0", "descr": "Internal SRAM memory"},
            {"name": "ext_mem0", "descr": "External DDR memory"},
        ],
    },
    {
        "name": "peripherals",
        "descr": "peripheral modules",
        "blocks": [
            {
                "name": "UART0",
                "type": "UART16550",
                "descr": "Default UART interface",
                "params": {},
            },
            {
                "name": "PLIC0",
                "type": "PLIC",
                "descr": "PLIC peripheral",
                "params": {"N_SOURCES": "32", "N_TARGETS": "2"},
            },
            {
                "name": "CLINT0",
                "type": "CLINT",
                "descr": "CLINT peripheral",
                "params": {},
            },
        ],
    },
]

confs = [
    # macros
    {
        "name": "USE_EXTMEM",
        "type": "M",
        "val": "1",
        "min": "1",
        "max": "32",
        "descr": "Always use external memory in the SoC.",
    },
    {
        "name": "E",
        "type": "M",
        "val": "31",
        "min": "1",
        "max": "32",
        "descr": "Address selection bit for external memory",
    },
    {
        "name": "P",
        "type": "M",
        "val": "30",
        "min": "1",
        "max": "32",
        "descr": "Address selection bit for peripherals",
    },
    {
        "name": "B",
        "type": "M",
        "val": "29",
        "min": "1",
        "max": "32",
        "descr": "Address selection bit for boot ROM",
    },
    # parameters
    {
        "name": "BOOTROM_ADDR_W",
        "type": "P",
        "val": "12",
        "min": "1",
        "max": "32",
        "descr": "Boot ROM address width",
    },
    {
        "name": "SRAM_ADDR_W",
        "type": "P",
        "val": "15",
        "min": "1",
        "max": "32",
        "descr": "SRAM address width",
    },
    {
        "name": "N_CORES",
        "type": "P",
        "val": "1",
        "min": "1",
        "max": "32",
        "descr": "Number of CPU cores used in the SoC.",
    },
    # INTERRUPTS ARCHITECTURE
    {
        "name": "N_SOURCES",
        "type": "P",
        "val": "32",
        "min": "1",
        "max": "32",
        "descr": "Number of peripherals that can generate an external interrupt to be interpreted by the PLIC.",
    },
    {
        "name": "N_TARGETS",
        "type": "P",
        "val": "2",
        "min": "1",
        "max": "32",
        "descr": "Number of HARTs in the SoC.",
    },
    # mandatory parameters (do not change them!)
    {
        "name": "ADDR_W",
        "type": "P",
        "val": "32",
        "min": "1",
        "max": "32",
        "descr": "Address bus width",
    },
    {
        "name": "DATA_W",
        "type": "P",
        "val": "32",
        "min": "1",
        "max": "32",
        "descr": "Data bus width",
    },
    {
        "name": "AXI_ID_W",
        "type": "P",
        "val": "0",
        "min": "1",
        "max": "32",
        "descr": "AXI ID bus width",
    },
    {
        "name": "AXI_ADDR_W",
        "type": "P",
        "val": "`MEM_ADDR_W",
        "min": "1",
        "max": "32",
        "descr": "AXI address bus width",
    },
    {
        "name": "AXI_DATA_W",
        "type": "P",
        "val": "`IOB_SOC_OPENCRYPTOLINUX_DATA_W",
        "min": "1",
        "max": "32",
        "descr": "AXI data bus width",
    },
    {
        "name": "AXI_LEN_W",
        "type": "P",
        "val": "4",
        "min": "1",
        "max": "4",
        "descr": "AXI burst length width",
    },
]

regs = []

ios = [
    {
        "name": "general",
        "descr": "General interface signals",
        "ports": [
            {
                "name": "clk_i",
                "type": "I",
                "n_bits": "1",
                "descr": "System clock input",
            },
            {
                "name": "arst_i",
                "type": "I",
                "n_bits": "1",
                "descr": "System reset, synchronous and active high",
            },
            {"name": "trap_o", "type": "O", "n_bits": "1", "descr": "CPU trap signal"},
        ],
    },
    {
        "name": "axi_m_port",
        "descr": "General interface signals",
        "ports": [],
        "if_defined": "USE_EXTMEM",
    },
]

peripheral_portmap = [
    # Map interrupt port to internal wire
    (
        {"corename": "UART0", "if_name": "interrupt", "port": "interrupt", "bits": []},
        {"corename": "internal", "if_name": "UART0", "port": "", "bits": []},
    ),
    # Map other rs232 ports to external interface (system IO)
    (
        {"corename": "UART0", "if_name": "rs232", "port": "txd", "bits": []},
        {"corename": "external", "if_name": "UART0", "port": "", "bits": []},
    ),
    (
        {"corename": "UART0", "if_name": "rs232", "port": "rxd", "bits": []},
        {"corename": "external", "if_name": "UART0", "port": "", "bits": []},
    ),
    (
        {"corename": "UART0", "if_name": "rs232", "port": "cts", "bits": []},
        {"corename": "external", "if_name": "UART0", "port": "", "bits": []},
    ),
    (
        {"corename": "UART0", "if_name": "rs232", "port": "rts", "bits": []},
        {"corename": "external", "if_name": "UART0", "port": "", "bits": []},
    ),
    # Map `mtip` of CLINT0 to an internal wire named `CLINT0_mtip`
    (
        {"corename": "CLINT0", "if_name": "clint_io", "port": "mtip", "bits": []},
        {"corename": "internal", "if_name": "CLINT0", "port": "", "bits": []},
    ),
    # Map `msip` of CLINT0 to an internal wire named `CLINT0_msip`
    (
        {"corename": "CLINT0", "if_name": "clint_io", "port": "msip", "bits": []},
        {"corename": "internal", "if_name": "CLINT0", "port": "", "bits": []},
    ),
    # Map `msip` of CLINT0 to an internal wire named `CLINT0_msip`
    (
        {"corename": "CLINT0", "if_name": "clint_io", "port": "rt_clk", "bits": []},
        {"corename": "internal", "if_name": "CLINT0", "port": "", "bits": []},
    ),
    # Map `mtip` of PLIC0 to an internal wire named `PLIC0_irq`
    (
        {"corename": "PLIC0", "if_name": "plic_io", "port": "irq", "bits": []},
        {"corename": "internal", "if_name": "PLIC0", "port": "", "bits": []},
    ),
    # Map `msip` of PLIC0 to an internal wire named `PLIC0_src`
    (
        {"corename": "PLIC0", "if_name": "plic_io", "port": "src", "bits": []},
        {"corename": "internal", "if_name": "PLIC0", "port": "", "bits": []},
    ),
]

# Add IOb-SoC modules. These will copy and generate common files from the IOb-SoC repository.
iob_soc.add_iob_soc_modules(sys.modules[__name__])


def custom_setup():
    # Add the following arguments:
    # "INIT_MEM": if should setup with init_mem or not
    # "USE_EXTMEM": if should setup with extmem or not
    for arg in sys.argv[1:]:
        if arg == "INIT_MEM":
            update_define(confs, "INIT_MEM", True)
        if arg == "USE_EXTMEM":
            update_define(confs, "USE_EXTMEM", True)

    for conf in confs:
        if (conf["name"] == "USE_EXTMEM") and conf["val"]:
            submodules["hw_setup"]["headers"].append(
                {
                    "file_prefix": "ddr4_",
                    "interface": "axi_wire",
                    "wire_prefix": "ddr4_",
                    "port_prefix": "ddr4_",
                }
            )


# Main function to setup this system and its components
def main():
    custom_setup()
    # Setup this system
    setup.setup(sys.modules[__name__])


if __name__ == "__main__":
    main()
