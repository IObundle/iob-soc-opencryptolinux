`timescale 1 ns / 1 ps

`include "bsp.vh"
`include "iob_soc_opencryptolinux_conf.vh"
`include "iob_soc_opencryptolinux.vh"
`include "iob_lib.vh"

`include "iob_soc_opencryptolinux_periphs_swreg_def.vs"

module iob_soc_opencryptolinux #(
   `include "iob_soc_opencryptolinux_params.vs"
) (
   `include "iob_soc_opencryptolinux_io.vs"
);

   localparam integer E_BIT = `IOB_SOC_OPENCRYPTOLINUX_E;
   localparam integer P_BIT = `IOB_SOC_OPENCRYPTOLINUX_P;
   localparam integer B_BIT = `IOB_SOC_OPENCRYPTOLINUX_B;
   localparam integer NSlaves = `IOB_SOC_OPENCRYPTOLINUX_N_SLAVES;

   `include "iob_soc_opencryptolinux_pwires.vs"

   //
   // SYSTEM RESET
   //

   wire boot;
   wire cpu_reset;

   wire cke_i;
   assign cke_i = 1'b1;

   wire [  1+SRAM_ADDR_W-2+DATA_W+DATA_W/8-1:0] ext_mem0_i_req;
   wire [1+`MEM_ADDR_W+1-2+DATA_W+DATA_W/8-1:0] ext_mem0_d_req;

   //
   //  CPU
   //

   // instruction bus
   wire [                           `REQ_W-1:0] cpu_i_req;
   wire [                          `RESP_W-1:0] cpu_i_resp;

   // data cat bus
   wire [                           `REQ_W-1:0] cpu_d_req;
   wire [                          `RESP_W-1:0] cpu_d_resp;

   assign PLIC0_src     = {{31{1'b0}}, UART0_interrupt};
   assign CLINT0_rt_clk = 1'b0;

   //instantiate the cpu
   iob_VexRiscv #(
      .ADDR_W    (ADDR_W),
      .DATA_W    (DATA_W),
      .E_BIT     (E_BIT),
      .P_BIT     (P_BIT),
      .USE_EXTMEM(1)
   ) cpu_0 (
      .clk_i             (clk_i),
      .cke_i             (cke_i),
      .arst_i            (cpu_reset),
      .boot_i            (boot),
      //instruction bus
      .ibus_req          (cpu_i_req),
      .ibus_resp         (cpu_i_resp),
      //data bus
      .dbus_req          (cpu_d_req),
      .dbus_resp         (cpu_d_resp),
      // interupts
      .timerInterrupt    (CLINT0_mtip[0]),
      .softwareInterrupt (CLINT0_msip[0]),
      .externalInterrupts(PLIC0_irq[1:0])
   );


   //
   // SPLIT CPU BUSES TO ACCESS INTERNAL OR EXTERNAL MEMORY
   //

   //internal memory instruction bus
   wire [ `REQ_W-1:0] int_mem_i_req;
   wire [`RESP_W-1:0] int_mem_i_resp;
   //external memory instruction bus
   wire [ `REQ_W-1:0] ext_mem_i_req;
   wire [`RESP_W-1:0] ext_mem_i_resp;

   // INSTRUCTION BUS
   iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(2),
      .P_SLAVES(`E_BIT)
   ) ibus_split (
      .clk_i   (clk_i),
      .arst_i  (cpu_reset),
      // master interface
      .m_req_i (cpu_i_req),
      .m_resp_o(cpu_i_resp),
      // slaves interface
      .s_req_o ({ext_mem_i_req, int_mem_i_req}),
      .s_resp_i({ext_mem_i_resp, int_mem_i_resp})
   );


   // DATA BUS

   //internal data bus
   wire [ `REQ_W-1:0] int_d_req;
   wire [`RESP_W-1:0] int_d_resp;
   //external memory data bus
   wire [ `REQ_W-1:0] ext_mem_d_req;
   wire [`RESP_W-1:0] ext_mem_d_resp;

   //E,{P,I}
   iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(2),
      .P_SLAVES(E_BIT)
   ) dbus_split (
      .clk_i   (clk_i),
      .arst_i  (cpu_reset),
      // master interface
      .m_req_i (cpu_d_req),
      .m_resp_o(cpu_d_resp),
      // slaves interface
      .s_req_o ({ext_mem_d_req, int_d_req}),
      .s_resp_i({ext_mem_d_resp, int_d_resp})
   );

   //
   // SPLIT INTERNAL MEMORY AND PERIPHERALS BUS
   //

   //internal memory data bus
   wire [ `REQ_W-1:0] int_mem_d_req;
   wire [`RESP_W-1:0] int_mem_d_resp;
   //peripheral bus
   wire [ `REQ_W-1:0] pbus_req;
   wire [`RESP_W-1:0] pbus_resp;

   //P,I
   iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(2),
      .P_SLAVES(P_BIT)
   ) int_dbus_split (
      .clk_i   (clk_i),
      .arst_i  (cpu_reset),
      // master interface
      .m_req_i (int_d_req),
      .m_resp_o(int_d_resp),
      // slaves interface
      .s_req_o ({pbus_req, int_mem_d_req}),
      .s_resp_i({pbus_resp, int_mem_d_resp})
   );


   //
   // SPLIT PERIPHERAL BUS
   //

   //slaves bus
   wire [ NSlaves*`REQ_W-1:0] slaves_req;
   wire [NSlaves*`RESP_W-1:0] slaves_resp;

   iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(NSlaves),
      .P_SLAVES(P_BIT - 1)
   ) pbus_split (
      .clk_i   (clk_i),
      .arst_i  (cpu_reset),
      // master interface
      .m_req_i (pbus_req),
      .m_resp_o(pbus_resp),
      // slaves interface
      .s_req_o (slaves_req),
      .s_resp_i(slaves_resp)
   );


   //
   // INTERNAL SRAM MEMORY
   //

   int_mem #(
      .ADDR_W        (ADDR_W),
      .DATA_W        (DATA_W),
      .HEXFILE       ("iob_soc_opencryptolinux_firmware"),
      .BOOT_HEXFILE  ("iob_soc_opencryptolinux_boot"),
      .SRAM_ADDR_W   (SRAM_ADDR_W),
      .BOOTROM_ADDR_W(BOOTROM_ADDR_W),
      .B_BIT         (B_BIT)
   ) int_mem0 (
      .clk_i    (clk_i),
      .arst_i   (arst_i),
      .cke_i    (cke_i),
      .boot     (boot),
      .cpu_reset(cpu_reset),
      // instruction bus
      .i_req    (int_mem_i_req),
      .i_resp   (int_mem_i_resp),
      //data bus
      .d_req    (int_mem_d_req),
      .d_resp   (int_mem_d_resp)
   );


   assign ext_mem0_i_req = {ext_mem_i_req[`AVALID(0)], ext_mem_i_req[`ADDRESS(0, SRAM_ADDR_W)-2], ext_mem_i_req[`WRITE(0)]};
   assign ext_mem0_d_req = {ext_mem_d_req[`AVALID(0)], ext_mem_d_req[`ADDRESS(0, `MEM_ADDR_W+1)-2], ext_mem_d_req[`WRITE(0)]};
   //
   // EXTERNAL DDR MEMORY
   //
   ext_mem #(
      .ADDR_W     (ADDR_W),
      .DATA_W     (DATA_W),
      .FIRM_ADDR_W(SRAM_ADDR_W),
      .MEM_ADDR_W (`MEM_ADDR_W),
      .DDR_ADDR_W (`DDR_ADDR_W),
      .DDR_DATA_W (`DDR_DATA_W),
      .AXI_ID_W   (AXI_ID_W),
      .AXI_LEN_W  (AXI_LEN_W),
      .AXI_ADDR_W (AXI_ADDR_W),
      .AXI_DATA_W (AXI_DATA_W)
   ) ext_mem0 (
      // instruction bus
      .i_req        (ext_mem0_i_req),
      .i_resp       (ext_mem_i_resp),
      // data bus
      .d_req        (ext_mem0_d_req),
      .d_resp       (ext_mem_d_resp),
      //AXI INTERFACE
      .axi_awid_o   (axi_awid_o),      //Address write channel ID.
      .axi_awaddr_o (axi_awaddr_o),    //Address write channel address.
      .axi_awlen_o  (axi_awlen_o),     //Address write channel burst length.
      .axi_awsize_o (axi_awsize_o),    //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .axi_awburst_o(axi_awburst_o),   //Address write channel burst type.
      .axi_awlock_o (axi_awlock_o),    //Address write channel lock type.
      .axi_awcache_o(axi_awcache_o),   //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .axi_awprot_o (axi_awprot_o),    //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .axi_awqos_o  (axi_awqos_o),     //Address write channel quality of service.
      .axi_awvalid_o(axi_awvalid_o),   //Address write channel valid.
      .axi_awready_i(axi_awready_i),   //Address write channel ready.
      .axi_wdata_o  (axi_wdata_o),     //Write channel data.
      .axi_wstrb_o  (axi_wstrb_o),     //Write channel write strobe.
      .axi_wlast_o  (axi_wlast_o),     //Write channel last word flag.
      .axi_wvalid_o (axi_wvalid_o),    //Write channel valid.
      .axi_wready_i (axi_wready_i),    //Write channel ready.
      .axi_bid_i    (axi_bid_i),       //Write response channel ID.
      .axi_bresp_i  (axi_bresp_i),     //Write response channel response.
      .axi_bvalid_i (axi_bvalid_i),    //Write response channel valid.
      .axi_bready_o (axi_bready_o),    //Write response channel ready.
      .axi_arid_o   (axi_arid_o),      //Address read channel ID.
      .axi_araddr_o (axi_araddr_o),    //Address read channel address.
      .axi_arlen_o  (axi_arlen_o),     //Address read channel burst length.
      .axi_arsize_o (axi_arsize_o),    //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .axi_arburst_o(axi_arburst_o),   //Address read channel burst type.
      .axi_arlock_o (axi_arlock_o),    //Address read channel lock type.
      .axi_arcache_o(axi_arcache_o),   //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .axi_arprot_o (axi_arprot_o),    //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .axi_arqos_o  (axi_arqos_o),     //Address read channel quality of service.
      .axi_arvalid_o(axi_arvalid_o),   //Address read channel valid.
      .axi_arready_i(axi_arready_i),   //Address read channel ready.
      .axi_rid_i    (axi_rid_i),       //Read channel ID.
      .axi_rdata_i  (axi_rdata_i),     //Read channel data.
      .axi_rresp_i  (axi_rresp_i),     //Read channel response.
      .axi_rlast_i  (axi_rlast_i),     //Read channel last word.
      .axi_rvalid_i (axi_rvalid_i),    //Read channel valid.
      .axi_rready_o (axi_rready_o),    //Read channel ready.
      .clk_i        (clk_i),
      .cke_i        (cke_i),
      .arst_i       (cpu_reset)
   );

   `include "iob_soc_opencryptolinux_periphs_inst.vs"

endmodule
