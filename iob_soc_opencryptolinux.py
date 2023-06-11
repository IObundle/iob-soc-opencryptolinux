#!/usr/bin/env python3
import os
import copy

from iob_soc import iob_soc
from iob_vexriscv import iob_vexriscv
from iob_uart16550 import iob_uart16550
from iob_plic import iob_plic
from iob_clint import iob_clint


class iob_soc_opencryptolinux(iob_soc):
    name = "iob_soc_opencryptolinux"
    version = "V0.70"
    flows = "pc-emul emb sim doc fpga"
    setup_dir = os.path.dirname(__file__)

    @classmethod
    def _create_instances(cls):
        super()._create_instances()

        # Verilog modules instances if we have them in the setup list (they may not be in the list if a subclass decided to remove them).
        if iob_vexriscv in cls.submodule_setup_list:
            cls.cpu = iob_vexriscv.instance("cpu_0")

        # Instantiate OpenCryptoLinux peripherals
        if iob_uart16550 in cls.submodule_setup_list:
            cls.peripherals.append(
                iob_uart16550.instance("UART0", "Default UART interface")
            )
        if iob_plic in cls.submodule_setup_list:
            cls.peripherals.append(
                iob_plic.instance(
                    "PLIC0",
                    "PLIC peripheral",
                    parameters={"N_SOURCES": "32", "N_TARGETS": "2"},
                )
            )
        if iob_clint in cls.submodule_setup_list:
            cls.peripherals.append(iob_clint.instance("CLINT0", "CLINT peripheral"))

    @classmethod
    def _create_submodules_list(cls):
        super()._create_submodules_list()

        # Remove picorv32 and uart from iob-soc
        i = 0
        while i < len(cls.submodule_setup_list):
            if type(cls.submodule_setup_list[i]) == type and cls.submodule_setup_list[
                i
            ].name in ["iob_picorv32", "iob_uart"]:
                cls.submodule_setup_list.pop(i)
                continue
            i += 1

        # Submodules
        cls.submodule_setup_list += [
            iob_vexriscv,
            iob_plic,
            iob_clint,
            iob_uart16550,
        ]

    @classmethod
    def _setup_confs(cls, extra_confs=[]):
        # Append confs or override them if they exist
        super()._setup_confs(
            [
                {
                    "name": "USE_EXTMEM",
                    "type": "M",
                    "val": "1",
                    "min": "1",
                    "max": "32",
                    "descr": "Always use external memory in the SoC.",
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
            ]
            + extra_confs
        )

        # Remove unnecessary confs of IOb-SoC
        i = 0
        while i < len(cls.confs):
            if cls.confs[i]["name"] in ["USE_MUL_DIV", "USE_COMPRESSED"]:
                cls.confs.pop(i)
                continue
            i += 1

    @classmethod
    def _setup_portmap(cls):
        cls.peripheral_portmap += [
            # Map interrupt port to internal wire
            (
                {
                    "corename": "UART0",
                    "if_name": "interrupt",
                    "port": "interrupt",
                    "bits": [],
                },
                {"corename": "internal", "if_name": "UART0", "port": "", "bits": []},
            ),
            # Map other rs232 ports to external interface (system IO)
            (
                {"corename": "UART0", "if_name": "rs232", "port": "txd", "bits": []},
                {"corename": "external", "if_name": "UART", "port": "", "bits": []},
            ),
            (
                {"corename": "UART0", "if_name": "rs232", "port": "rxd", "bits": []},
                {"corename": "external", "if_name": "UART", "port": "", "bits": []},
            ),
            (
                {"corename": "UART0", "if_name": "rs232", "port": "cts", "bits": []},
                {"corename": "external", "if_name": "UART", "port": "", "bits": []},
            ),
            (
                {"corename": "UART0", "if_name": "rs232", "port": "rts", "bits": []},
                {"corename": "external", "if_name": "UART", "port": "", "bits": []},
            ),
            # Map `mtip` of CLINT0 to an internal wire named `CLINT0_mtip`
            (
                {
                    "corename": "CLINT0",
                    "if_name": "clint_io",
                    "port": "mtip",
                    "bits": [],
                },
                {"corename": "internal", "if_name": "CLINT0", "port": "", "bits": []},
            ),
            # Map `msip` of CLINT0 to an internal wire named `CLINT0_msip`
            (
                {
                    "corename": "CLINT0",
                    "if_name": "clint_io",
                    "port": "msip",
                    "bits": [],
                },
                {"corename": "internal", "if_name": "CLINT0", "port": "", "bits": []},
            ),
            # Map `msip` of CLINT0 to an internal wire named `CLINT0_msip`
            (
                {
                    "corename": "CLINT0",
                    "if_name": "clint_io",
                    "port": "rt_clk",
                    "bits": [],
                },
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
