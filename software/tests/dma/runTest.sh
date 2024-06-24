#!/bin/sh 

# Load Linux Kernel Modules
insmod dma/drivers/iob_dma.ko
insmod dma/drivers/iob_axistream_in.ko
insmod dma/drivers/iob_axistream_out.ko

# Run DMA demo
./dma/dma_demo
