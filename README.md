# IOb-SoC-OpenCryptoLinux

OpenCryptoLinux aims to develop an open, secure, and user-friendly SoC template capable of running the Linux operating system, with cryptography functions running on a RISC-V processor. The processor will control a low-cost Coarse-Grained Reconfigurable Arrays (CGRAS) for enhanced security, performance, and energy efficiency. Running Linux on this SoC allows non-hardware experts to use this platform, democratizing it. This project will help build an Internet of Things (IoT) that does not compromise security and privacy. The project will be fully open-source, which guarantees public scrutiny and quality. It will use other open-source solutions funded by the NLnet Foundation, such as the RISC-V processors from SpinalHDL and the OpenCryptoHW project.



## Table of Contents
1. [Setup Development Environment](#setup)
2. [Simulate the system](#simulation)
3. [Emulate the system on PC (WIP)](#emulation)
4. [Build and run on FPGA board](#fpga)
5. [Compile the documentation](#doc)
6. [Testing (WIP)](#testing)
7. [Required Software](#requirements)
8. [Acknowledgement](#acknowledgement)



## Setup Development Environment <a name="setup"></a>

### Operating Systems

IOb-SoC-OpenCryptoLinux can be used in Linux Operating Systems. The following instructions work
for CentOS 7 and Ubuntu 18.04 or 20.04 LTS.

### Clone the repository

The first step is to clone this repository. IOb-SoC uses git sub-module trees, and
GitHub will ask for your password for each downloaded module if you clone by *https*. To avoid this,
setup GitHub access with *ssh* and type:
```
git clone --recursive git@github.com:IObundle/iob-soc-opencryptolinux.git
cd iob-soc-opencryptolinux
```

Alternatively, you can still clone this repository using *https* if you cache
your credentials before cloning the repository, using: ``git config --global
credential.helper 'cache --timeout=<time_in_seconds>'``


### Configure your SoC

To configure your system edit the `config.mk` file, which can be found at the
repository root. This file has the system configuration variables;
hopefully, each variable is explained by a comment.


### Set environment variables for local or remote building and running

The various simulators, FPGA compilers and FPGA boards may run locally or
remotely. For running a tool remotely, you need to set two environmental
variables: the server logical name and the server user name. Consider placing
these settings in your `.bashrc` file, so that they apply to every session.


### Set up the remote simulator server

Using open-source simulator Verilator as an example, note that in
`hardware/simulation/verilator/Makefile`, the variable for the server logical name,
`SIM_SERVER`, is set to `VSIM_SERVER`, and the variable for the user name,
`SIM_USER`, is set to `VSIM_USER`. If you do not set these variables the 
simulator will run locally. To run the simulator on server 
*mysimserver.myorg.com* as user *vsimuser*, set the following environmental 
variables beforehand:

```Bash
export VSIM_SERVER=mysimserver.myorg.com
export VSIM_USER=vsimuser
```

### Set up the remote FPGA toolchain and board servers

Using the CYCLONEV-GT-DK board as an example, note that in
`hardware/fpga/quartus/CYCLONEV-GT-DK/Makefile` the variable for the FPGA tool
server logical name, `FPGA_SERVER`, is set to `QUARTUS_SERVER`, and the 
variable for the user name, `FPGA_USER`, is set to `QUARTUS_USER`; the 
variable for the board server, `BOARD_SERVER`, is set to `CYC5_SERVER`, and 
the variable for the board user, `BOARD_USER`, is set to `CYC5_USER`. As in the 
previous example, set these variables as follows:

```Bash
export QUARTUS_SERVER=quartusserver.myorg.com
export QUARTUS_USER=quartususer
export CYC5_SERVER=cyc5server.myorg.com
export CYC5_USER=cyc5username
```

In each remote server, the environment variables for the executable paths and license
servers used must be defined as in the following example:

```Bash
export QUARTUSPATH=/path/to/quartus
export VIVADOPATH=/path/to/vivado
...
export LM_LICENSE_FILE=port@licenseserver.myorg.com;lic_or_dat_file
```


## Simulate the system <a name="simulation"></a>

To simulate IOb-SoC, the simulator must be installed, either locally or
remotely, and must have a run directory under the `hardware/simulation`
directory, such as the `hardware/simulation/verilator` directory. To simulate,
type:

```
make [sim-run] [SIMULATOR=<simulator directory name>] [<control parameters>]
```

`<simulator directory name>` is the name of the simulator's run directory,

`<control parameters>` are system configuration parameters passed in the
command line, overriding those in the `config.mk` file. Example control
parameters are `INIT_MEM=0 RUN_EXTMEM=1`. For example,
```
make sim-run SIMULATOR=verilator RUN_LINUX=0 RUN_EXTMEM=1
```

To visualise simulation waveforms use the `VCD=1` control parameter. It will
open the Gtkwave waveform visualisation program.

To clean simulation generated files, type:
```
make sim-clean [SIMULATOR=<simulator directory name>] 
# Example
make sim-clean SIMULATOR=verilator
```

For more details, read the Makefile in each simulator directory. The Makefile
includes the Makefile segment `simulation.mk`, which contains statements that
apply to any simulator. In turn, `simulation.mk` includes the Makefile segment
`hardware.mk`, which contains targets common to all hardware tools. The 
`hardware.mk` includes `config.mk`,  which contains main system parameters. The
Makefile in the simulator's directory, with the segments recursively included as
described, is construed as a single large Makefile.

## Emulate the system on PC (WIP) <a name="emulation"></a>

If there are embedded software compilation or runtime issues you can
*emulate* the system on a PC to debug the issues. To emulate IOb-SoC's embedded
software on a PC, type:

```
make pc-emul [<control parameters>]
```
where `<control parameters>` are system configuration parameters passed in the
command line, overriding those in the `config.mk` file. Example control
parameters are `INIT_MEM=0 RUN_EXTMEM=1`. For example,
```
make pc-emul INIT_MEM=0 RUN_EXTMEM=1
```

To clean the PC compilation generated files, type:
```
make pc-emul-clean
```

For more details, read the Makefile in the `software/pc-emul` directory. As
explained for the simulation make file, note the Makefile segments recursively
included.


## Build and run on FPGA board <a name="fpga"></a>

To build and run IOb-SoC on an FPGA board, the FPGA design tools must be
installed, either locally or remotely, the board must be attached to the local
host or to a remote host, and each board must have a build directory under the
`hardware/fpga/<tool>` directory, for example the `hardware/fpga/vivado/BASYS3`
directory. The FPGA tools and board hosts may be different.

To build only, type
``` 
make fpga-build [BOARD=<board directory name>] [<control parameters>]
``` 
where `<board directory name>` is the name of the board's run directory, and
`<control parameters>` are system configuration parameters passed in the command
line, overriding those in the `config.mk` file. For example, 
``` 
make fpga-build BOARD=BASYS3 INIT_MEM=0 RUN_EXTMEM=1
``` 

For more details read the Makefile in the board directory, and follow the
recursively included Makefile segments as explained before.

To build and run, type:
``` 
make fpga-run [BOARD=<board directory name>] [<control parameters>]
``` 

The FPGA is loaded with the configuration bitstream before running. However,
this step is skipped if the bitstream checksum matches that of the last loaded
bitstream, kept in file `/tmp/<board directory name>.load`. If, for some reason,
the run gets stuck, you may interrupt it with `Ctr-C`. Then, you may try again
forcing the bitstream to be reloaded using control parameter `FORCE=1`.

If many users are trying to run the same FPGA board they will be queued in file
`/tmp/<board directory name>.queue`. Users will orderly load their bitstream
onto the board and start running it. After a successful run or `Ctr-C` 
interrupt, the user is de-queued.


To clean the FPGA compilation generated files, type
``` 
make fpga-clean [BOARD=<board directory name>]
``` 

## Compile the documentation <a name="doc"></a>

To compile documents, the LaTeX document preparation software must be
installed. Each document that can be compiled has a build directory under the
`document` directory. Currently there are two document build directories:
`presentation` and `pb` (product brief). The document to build is specified by
the DOC control parameter. To compile the document, type:
```
make doc [DOC=<document directory name>]
```


To clean the document's build directory, type:
```
make doc-clean [DOC=<document directory name>]
```

For more details, read the Makefile in each document's directory, and follow the
recursively included Makefile segments as explained before.


## Testing (WIP) <a name="testing"></a>

### Simulation test

To run a series of simulation tests on the simulator selected by the SIMULATOR
variable, type:

```
make sim-test [SIMULATOR=<simulator directory>]
```

The above command produces a test log file called `test.log` in the simulator's
directory. The `test.log` file is compared with the `test.expected` file, which
resides in the same directory; if they differ, the test fails; otherwise, it
passes.

To run the series of simulation tests on all supported simulators, type:

```
make test-sim 
```

To clean the files produced when testing all simulators, type:

```
make test-sim-clean
```


### Board test

To compile and run a series of board tests on the board selected by the `BOARD`
variable, type:

```
make fpga-test [BOARD=<board directory name>]
```

The above command produces a test log file called `test.log` in the board's
directory. The `test.log` file is compared with the `test.expected` file, which
resides in the same directory; if they differ, the test fails; otherwise, it
passes.

To run the series of board tests on all supported boards, type:

```
make test-fpga 
```

To clean the files produced when testing all boards, type:
```
make test-fpga-clean
```



### Documentation test

To compile and test the document selected by the `DOC`, variable, type:

```
make doc-test [DOC=<document directory name>]
```

The resulting Latex .aux file is compared with a known-good .aux file. If the
match the test passes; otherwise it fails.

To test all supported documents, type:

```
make test-doc
```

To clean the files produced when testing all documents, type:
```
make test-doc-clean
```

### Total test

To run all simulation, FPGA board and documentation tests, type:
```
make test
```

### Cleaning

The following command will clean the selected simulation, board and document
directories, locally and in the remote servers:

```
make test-clean
```

## Required Software <a name="requirements"></a>
- RISC-V GNU Compiler Toolchain 2022.06.10 - 

    This will take a while. After it is done, type:
    ```
    export PATH=$PATH:/path/to/riscv/bin
    ```

    The above command should be added to your `~/.bashrc` file, so that
    you do not have to type it on every session.
- Icarus Verilog v11.0 (stable)
- Verilator v4.226-27

# Acknowledgement <a name="acknowledgement"></a>
This project is funded through the NGI Assure Fund, a fund established by NLnet
with financial support from the European Commission's Next Generation Internet
programme, under the aegis of DG Communications Networks, Content and Technology
under grant agreement No 957073.
