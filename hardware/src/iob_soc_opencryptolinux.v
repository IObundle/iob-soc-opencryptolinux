`timescale 1 ns / 1 ps

`include "bsp.vh"
`include "iob_soc_opencryptolinux_conf.vh"
`include "iob_soc_opencryptolinux.vh"
`include "iob_utils.vh"

`include "iob_soc_opencryptolinux_periphs_swreg_def.vs"

module iob_soc_opencryptolinux #(
    `include "iob_soc_opencryptolinux_params.vs"
) (
    `include "iob_soc_opencryptolinux_io.vs"
);

  localparam integer Bbit = `IOB_SOC_OPENCRYPTOLINUX_B;
  localparam integer AddrMsb = `REQ_W - 2;
  localparam integer MEM_ADDR_OFFSET = 0;

  `include "iob_soc_opencryptolinux_pwires.vs"

  //
  // SYSTEM RESET
  //

  wire               boot;
  wire               cpu_reset;
  wire               cke_i = 1'b1;

  //
  //  CPU
  //

  // instruction bus
  wire [ `REQ_W-1:0] cpu_i_req;
  wire [`RESP_W-1:0] cpu_i_resp;

  // data cat bus
  wire [ `REQ_W-1:0] cpu_d_req;
  wire [`RESP_W-1:0] cpu_d_resp;

  assign cpu_trap_o = 1'b0;

  //instantiate the cpu
  iob_VexRiscv #(
      .ADDR_W    (ADDR_W),
      .DATA_W    (DATA_W),
      .USE_EXTMEM(1)
  ) cpu_0 (
      .clk_i         (clk_i),
      .cke_i         (cke_i),
      .arst_i        (arst_i),
      .cpu_reset_i   (cpu_reset),
      .boot_i        (boot),
      .clint_req     ({`REQ_W{1'b0}}),
      .clint_resp    (),
      .plic_req      ({`REQ_W{1'b0}}),
      .plic_resp     (),
      .plicInterrupts(32'd0),
      //instruction bus
      .ibus_req      (cpu_i_req),
      .ibus_resp     (cpu_i_resp),
      //data bus
      .dbus_req      (cpu_d_req),
      .dbus_resp     (cpu_d_resp)
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
      .P_SLAVES(AddrMsb)
  ) ibus_split (
      .clk_i   (clk_i),
      .arst_i  (arst_i),
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

  iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(2),       //E,{P,I}
      .P_SLAVES(AddrMsb)
  ) dbus_split (
      .clk_i   (clk_i),
      .arst_i  (arst_i),
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

  //slaves bus (includes internal memory + periphrals)
  wire [ (`IOB_SOC_OPENCRYPTOLINUX_N_SLAVES)*`REQ_W-1:0] slaves_req;
  wire [(`IOB_SOC_OPENCRYPTOLINUX_N_SLAVES)*`RESP_W-1:0] slaves_resp;

  iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(`IOB_SOC_OPENCRYPTOLINUX_N_SLAVES),
      .P_SLAVES(AddrMsb - 1)
  ) pbus_split (
      .clk_i   (clk_i),
      .arst_i  (arst_i),
      // master interface
      .m_req_i (int_d_req),
      .m_resp_o(int_d_resp),
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
      .B_BIT         (`B_BIT)
  ) int_mem0 (
      .clk_i    (clk_i),
      .arst_i   (arst_i),
      .cke_i    (cke_i),
      .boot     (boot),
      .cpu_reset(cpu_reset),

      // instruction bus
      .i_req (int_mem_i_req),
      .i_resp(int_mem_i_resp),

      //data bus
      .d_req (slaves_req[0+:`REQ_W]),
      .d_resp(slaves_resp[0+:`RESP_W])
  );

  //
  // EXTERNAL DDR MEMORY
  //
  ext_mem #(
      .ADDR_W    (ADDR_W),
      .DATA_W    (DATA_W),
      .AXI_ID_W  (AXI_ID_W),
      .AXI_LEN_W (AXI_LEN_W),
      .AXI_ADDR_W(MEM_ADDR_W),
      .AXI_DATA_W(DATA_W)
  ) ext_mem0 (
      // instruction bus
      .i_req (ext_mem_i_req),
      .i_resp(ext_mem_i_resp),

      //data bus
      .d_req (ext_mem_d_req),
      .d_resp(ext_mem_d_resp),

      //AXI INTERFACE
      //address write
      .axi_awid_o   (axi_awid_o[0+:AXI_ID_W]),
      .axi_awaddr_o (internal_axi_awaddr_o[0+:AXI_ADDR_W]),
      .axi_awlen_o  (axi_awlen_o[0+:AXI_LEN_W]),
      .axi_awsize_o (axi_awsize_o[0+:3]),
      .axi_awburst_o(axi_awburst_o[0+:2]),
      .axi_awlock_o (axi_awlock_o[0+:2]),
      .axi_awcache_o(axi_awcache_o[0+:4]),
      .axi_awprot_o (axi_awprot_o[0+:3]),
      .axi_awqos_o  (axi_awqos_o[0+:4]),
      .axi_awvalid_o(axi_awvalid_o[0+:1]),
      .axi_awready_i(axi_awready_i[0+:1]),
      //write
      .axi_wdata_o  (axi_wdata_o[0+:AXI_DATA_W]),
      .axi_wstrb_o  (axi_wstrb_o[0+:(AXI_DATA_W/8)]),
      .axi_wlast_o  (axi_wlast_o[0+:1]),
      .axi_wvalid_o (axi_wvalid_o[0+:1]),
      .axi_wready_i (axi_wready_i[0+:1]),
      //write response
      .axi_bid_i    (axi_bid_i[0+:AXI_ID_W]),
      .axi_bresp_i  (axi_bresp_i[0+:2]),
      .axi_bvalid_i (axi_bvalid_i[0+:1]),
      .axi_bready_o (axi_bready_o[0+:1]),
      //address read
      .axi_arid_o   (axi_arid_o[0+:AXI_ID_W]),
      .axi_araddr_o (internal_axi_araddr_o[0+:AXI_ADDR_W]),
      .axi_arlen_o  (axi_arlen_o[0+:AXI_LEN_W]),
      .axi_arsize_o (axi_arsize_o[0+:3]),
      .axi_arburst_o(axi_arburst_o[0+:2]),
      .axi_arlock_o (axi_arlock_o[0+:2]),
      .axi_arcache_o(axi_arcache_o[0+:4]),
      .axi_arprot_o (axi_arprot_o[0+:3]),
      .axi_arqos_o  (axi_arqos_o[0+:4]),
      .axi_arvalid_o(axi_arvalid_o[0+:1]),
      .axi_arready_i(axi_arready_i[0+:1]),
      //read
      .axi_rid_i    (axi_rid_i[0+:AXI_ID_W]),
      .axi_rdata_i  (axi_rdata_i[0+:AXI_DATA_W]),
      .axi_rresp_i  (axi_rresp_i[0+:2]),
      .axi_rlast_i  (axi_rlast_i[0+:1]),
      .axi_rvalid_i (axi_rvalid_i[0+:1]),
      .axi_rready_o (axi_rready_o[0+:1]),

      .clk_i (clk_i),
      .cke_i (cke_i),
      .arst_i(arst_i)
  );

  assign axi_awaddr_o[AXI_ADDR_W-1:0] = internal_axi_awaddr_o + MEM_ADDR_OFFSET;
  assign axi_araddr_o[AXI_ADDR_W-1:0] = internal_axi_araddr_o + MEM_ADDR_OFFSET;

  `include "iob_soc_opencryptolinux_periphs_inst.vs"

endmodule
