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
  localparam IBUS_AXI_ID_W = 1;
  localparam IBUS_AXI_LEN_W = 8;
  localparam IBUS_AXI_ADDR_W = 32;
  localparam IBUS_AXI_DATA_W = 32;
  localparam DBUS_AXI_ID_W = 1;
  localparam DBUS_AXI_LEN_W = 8;
  localparam DBUS_AXI_ADDR_W = 32;
  localparam DBUS_AXI_DATA_W = 32;
  localparam PERIPHERAL_AXI_ID_W = 1;
  localparam PERIPHERAL_AXI_LEN_W = 8;
  localparam PERIPHERAL_AXI_ADDR_W = 32;
  localparam PERIPHERAL_AXI_DATA_W = 32;
  localparam IBUS_INTMEM_AXI_ID_W = 1;
  localparam IBUS_INTMEM_AXI_LEN_W = 8;
  localparam IBUS_INTMEM_AXI_ADDR_W = 32;
  localparam IBUS_INTMEM_AXI_DATA_W = 32;
  localparam IBUS_EXTMEM_AXI_ID_W = 1;
  localparam IBUS_EXTMEM_AXI_LEN_W = 8;
  localparam IBUS_EXTMEM_AXI_ADDR_W = 32;
  localparam IBUS_EXTMEM_AXI_DATA_W = 32;
  localparam DBUS_EXTMEM_AXI_ID_W = 1;
  localparam DBUS_EXTMEM_AXI_LEN_W = 8;
  localparam DBUS_EXTMEM_AXI_ADDR_W = 32;
  localparam DBUS_EXTMEM_AXI_DATA_W = 32;


  `include "iob_soc_opencryptolinux_pwires.vs"


  wire iBus_intmem_iob_avalid;
  wire [ADDR_W-1:0] iBus_intmem_iob_addr;
  wire [DATA_W-1:0] iBus_intmem_iob_wdata;
  wire [DATA_W/8-1:0] iBus_intmem_iob_wstrb;
  wire iBus_intmem_iob_rvalid;
  wire [DATA_W-1:0] iBus_intmem_iob_rdata;
  wire iBus_intmem_iob_ready;

  wire peripheral_iob_avalid;
  wire [ADDR_W-1:0] peripheral_iob_addr;
  wire [DATA_W-1:0] peripheral_iob_wdata;
  wire [DATA_W/8-1:0] peripheral_iob_wstrb;
  wire peripheral_iob_rvalid;
  wire [DATA_W-1:0] peripheral_iob_rdata;
  wire peripheral_iob_ready;

  //
  // SYSTEM RESET
  //

  wire boot;
  wire cpu_reset;
  wire cke_i = 1'b1;

  //
  //  CPU
  //
  // Axi instruction bus
  `include "iBus_axi_wire.vs"
  // Axi data bus
  `include "dBus_axi_wire.vs"

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
      .clint_req     ({`REQ_W{1'b0}}),
      .clint_resp    (),
      .plic_req      ({`REQ_W{1'b0}}),
      .plic_resp     (),
      .plicInterrupts(32'd0),
      // Axi instruction bus
      `include "iBus_axi_m_portmap.vs"
      // Axi data bus
      `include "dBus_axi_m_portmap.vs"
      .boot_i        (boot)
  );


  //
  // SPLIT CPU BUSES TO ACCESS INTERNAL OR EXTERNAL MEMORY
  //

  // Intructions intmem AXI bus
  `include "iBus_intmem_axi_wire.vs"
  // Intructions intmem AXI bus
  `include "iBus_extmem_axi_wire.vs"
  // Data internal AXI bus
  `include "peripheral_axi_wire.vs"
  // Data extmem AXI bus
  `include "dBus_extmem_axi_wire.vs"

  // Instructions bus interconnect {I, D} -> {internal, external}
  axi_interconnect #(
      .ID_WIDTH  (1),
      .DATA_WIDTH(DATA_W),
      .ADDR_WIDTH(ADDR_W),
      .M_ADDR_WIDTH({2{32'd31}}),
      .S_COUNT   (1),
      .M_COUNT   (2)
  ) iBus_axi_interconnect (
      .clk(clk_i),
      .rst(arst_i),

      .s_axi_awid(iBus_axi_awid),
      .s_axi_awaddr(iBus_axi_awaddr),
      .s_axi_awlen(iBus_axi_awlen),
      .s_axi_awsize(iBus_axi_awsize),
      .s_axi_awburst(iBus_axi_awburst),
      .s_axi_awlock(iBus_axi_awlock[0]),
      .s_axi_awcache(iBus_axi_awcache),
      .s_axi_awprot(iBus_axi_awprot),
      .s_axi_awqos(iBus_axi_awqos),
      .s_axi_awvalid(iBus_axi_awvalid),
      .s_axi_awready(iBus_axi_awready),
      .s_axi_wdata(iBus_axi_wdata),
      .s_axi_wstrb(iBus_axi_wstrb),
      .s_axi_wlast(iBus_axi_wlast),
      .s_axi_wvalid(iBus_axi_wvalid),
      .s_axi_wready(iBus_axi_wready),
      .s_axi_bid(iBus_axi_bid),
      .s_axi_bresp(iBus_axi_bresp),
      .s_axi_bvalid(iBus_axi_bvalid),
      .s_axi_bready(iBus_axi_bready),
      .s_axi_arid(iBus_axi_arid),
      .s_axi_araddr(iBus_axi_araddr),
      .s_axi_arlen(iBus_axi_arlen),
      .s_axi_arsize(iBus_axi_arsize),
      .s_axi_arburst(iBus_axi_arburst),
      .s_axi_arlock(iBus_axi_arlock[0]),
      .s_axi_arcache(iBus_axi_arcache),
      .s_axi_arprot(iBus_axi_arprot),
      .s_axi_arqos(iBus_axi_arqos),
      .s_axi_arvalid(iBus_axi_arvalid),
      .s_axi_arready(iBus_axi_arready),
      .s_axi_rid(iBus_axi_rid),
      .s_axi_rdata(iBus_axi_rdata),
      .s_axi_rresp(iBus_axi_rresp),
      .s_axi_rlast(iBus_axi_rlast),
      .s_axi_rvalid(iBus_axi_rvalid),
      .s_axi_rready(iBus_axi_rready),

      .m_axi_awid({iBus_intmem_axi_awid, iBus_extmem_axi_awid}),
      .m_axi_awaddr({iBus_intmem_axi_awaddr, iBus_extmem_axi_awaddr}),
      .m_axi_awlen({iBus_intmem_axi_awlen, iBus_extmem_axi_awlen}),
      .m_axi_awsize({iBus_intmem_axi_awsize, iBus_extmem_axi_awsize}),
      .m_axi_awburst({iBus_intmem_axi_awburst, iBus_extmem_axi_awburst}),
      .m_axi_awlock({iBus_intmem_axi_awlock[0], iBus_extmem_axi_awlock[0]}),
      .m_axi_awcache({iBus_intmem_axi_awcache, iBus_extmem_axi_awcache}),
      .m_axi_awprot({iBus_intmem_axi_awprot, iBus_extmem_axi_awprot}),
      .m_axi_awqos({iBus_intmem_axi_awqos, iBus_extmem_axi_awqos}),
      .m_axi_awvalid({iBus_intmem_axi_awvalid, iBus_extmem_axi_awvalid}),
      .m_axi_awready({iBus_intmem_axi_awready, iBus_extmem_axi_awready}),
      .m_axi_wdata({iBus_intmem_axi_wdata, iBus_extmem_axi_wdata}),
      .m_axi_wstrb({iBus_intmem_axi_wstrb, iBus_extmem_axi_wstrb}),
      .m_axi_wlast({iBus_intmem_axi_wlast, iBus_extmem_axi_wlast}),
      .m_axi_wvalid({iBus_intmem_axi_wvalid, iBus_extmem_axi_wvalid}),
      .m_axi_wready({iBus_intmem_axi_wready, iBus_extmem_axi_wready}),
      .m_axi_bid({iBus_intmem_axi_bid, iBus_extmem_axi_bid}),
      .m_axi_bresp({iBus_intmem_axi_bresp, iBus_extmem_axi_bresp}),
      .m_axi_bvalid({iBus_intmem_axi_bvalid, iBus_extmem_axi_bvalid}),
      .m_axi_bready({iBus_intmem_axi_bready, iBus_extmem_axi_bready}),
      .m_axi_arid({iBus_intmem_axi_arid, iBus_extmem_axi_arid}),
      .m_axi_araddr({iBus_intmem_axi_araddr, iBus_extmem_axi_araddr}),
      .m_axi_arlen({iBus_intmem_axi_arlen, iBus_extmem_axi_arlen}),
      .m_axi_arsize({iBus_intmem_axi_arsize, iBus_extmem_axi_arsize}),
      .m_axi_arburst({iBus_intmem_axi_arburst, iBus_extmem_axi_arburst}),
      .m_axi_arlock({iBus_intmem_axi_arlock[0], iBus_extmem_axi_arlock[0]}),
      .m_axi_arcache({iBus_intmem_axi_arcache, iBus_extmem_axi_arcache}),
      .m_axi_arprot({iBus_intmem_axi_arprot, iBus_extmem_axi_arprot}),
      .m_axi_arqos({iBus_intmem_axi_arqos, iBus_extmem_axi_arqos}),
      .m_axi_arvalid({iBus_intmem_axi_arvalid, iBus_extmem_axi_arvalid}),
      .m_axi_arready({iBus_intmem_axi_arready, iBus_extmem_axi_arready}),
      .m_axi_rid({iBus_intmem_axi_rid, iBus_extmem_axi_rid}),
      .m_axi_rdata({iBus_intmem_axi_rdata, iBus_extmem_axi_rdata}),
      .m_axi_rresp({iBus_intmem_axi_rresp, iBus_extmem_axi_rresp}),
      .m_axi_rlast({iBus_intmem_axi_rlast, iBus_extmem_axi_rlast}),
      .m_axi_rvalid({iBus_intmem_axi_rvalid, iBus_extmem_axi_rvalid}),
      .m_axi_rready({iBus_intmem_axi_rready, iBus_extmem_axi_rready}),

      //optional signals
      .s_axi_awuser(1'b0),
      .s_axi_wuser (1'b0),
      .s_axi_aruser(1'b0),
      .m_axi_buser (2'b00),
      .m_axi_ruser (2'b00)
  );

  // instantiate axi2iob CPU data
  axi2iob #(
      .ADDR_WIDTH  (ADDR_W),
      .DATA_WIDTH  (DATA_W),
      .STRB_WIDTH  ((DATA_W / 8)),
      .AXI_ID_WIDTH(1)
  ) iBus_intmem_axi2iob (
      .clk_i(clk_i),
      .arst_i(arst_i),
      .s_axi_awid(iBus_intmem_axi_awid),
      .s_axi_awaddr(iBus_intmem_axi_awaddr),
      .s_axi_awlen(iBus_intmem_axi_awlen),
      .s_axi_awsize(iBus_intmem_axi_awsize),
      .s_axi_awburst(iBus_intmem_axi_awburst),
      .s_axi_awlock(iBus_intmem_axi_awlock[0]),
      .s_axi_awcache(iBus_intmem_axi_awcache),
      .s_axi_awprot(iBus_intmem_axi_awprot),
      .s_axi_awvalid(iBus_intmem_axi_awvalid),
      .s_axi_awready(iBus_intmem_axi_awready),
      .s_axi_wdata(iBus_intmem_axi_wdata),
      .s_axi_wstrb(iBus_intmem_axi_wstrb),
      .s_axi_wlast(iBus_intmem_axi_wlast),
      .s_axi_wvalid(iBus_intmem_axi_wvalid),
      .s_axi_wready(iBus_intmem_axi_wready),
      .s_axi_bid(iBus_intmem_axi_bid),
      .s_axi_bresp(iBus_intmem_axi_bresp),
      .s_axi_bvalid(iBus_intmem_axi_bvalid),
      .s_axi_bready(iBus_intmem_axi_bready),
      .s_axi_arid(iBus_intmem_axi_arid),
      .s_axi_araddr(iBus_intmem_axi_araddr),
      .s_axi_arlen(iBus_intmem_axi_arlen),
      .s_axi_arsize(iBus_intmem_axi_arsize),
      .s_axi_arburst(iBus_intmem_axi_arburst),
      .s_axi_arlock(iBus_intmem_axi_arlock[0]),
      .s_axi_arcache(iBus_intmem_axi_arcache),
      .s_axi_arprot(iBus_intmem_axi_arprot),
      .s_axi_arvalid(iBus_intmem_axi_arvalid),
      .s_axi_arready(iBus_intmem_axi_arready),
      .s_axi_rid(iBus_intmem_axi_rid),
      .s_axi_rdata(iBus_intmem_axi_rdata),
      .s_axi_rresp(iBus_intmem_axi_rresp),
      .s_axi_rlast(iBus_intmem_axi_rlast),
      .s_axi_rvalid(iBus_intmem_axi_rvalid),
      .s_axi_rready(iBus_intmem_axi_rready),
      // IOb-bus signals
      .iob_avalid_o(iBus_intmem_iob_avalid),
      .iob_addr_o(iBus_intmem_iob_addr),
      .iob_wdata_o(iBus_intmem_iob_wdata),
      .iob_wstrb_o(iBus_intmem_iob_wstrb),
      .iob_rvalid_i(iBus_intmem_iob_rvalid),
      .iob_rdata_i(iBus_intmem_iob_rdata),
      .iob_ready_i(iBus_intmem_iob_ready)
  );

  //
  // SPLIT INTERNAL MEMORY AND PERIPHERALS BUS
  //

  // Data bus interconnect {I, D} -> {internal, external}
  axi_interconnect #(
      .ID_WIDTH  (1),
      .DATA_WIDTH(DATA_W),
      .ADDR_WIDTH(ADDR_W),
      .M_ADDR_WIDTH({2{32'd31}}),
      .S_COUNT   (1),
      .M_COUNT   (2)
  ) dBus_axi_interconnect (
      .clk(clk_i),
      .rst(arst_i),

      .s_axi_awid(dBus_axi_awid),
      .s_axi_awaddr(dBus_axi_awaddr),
      .s_axi_awlen(dBus_axi_awlen),
      .s_axi_awsize(dBus_axi_awsize),
      .s_axi_awburst(dBus_axi_awburst),
      .s_axi_awlock(dBus_axi_awlock[0]),
      .s_axi_awcache(dBus_axi_awcache),
      .s_axi_awprot(dBus_axi_awprot),
      .s_axi_awqos(dBus_axi_awqos),
      .s_axi_awvalid(dBus_axi_awvalid),
      .s_axi_awready(dBus_axi_awready),
      .s_axi_wdata(dBus_axi_wdata),
      .s_axi_wstrb(dBus_axi_wstrb),
      .s_axi_wlast(dBus_axi_wlast),
      .s_axi_wvalid(dBus_axi_wvalid),
      .s_axi_wready(dBus_axi_wready),
      .s_axi_bid(dBus_axi_bid),
      .s_axi_bresp(dBus_axi_bresp),
      .s_axi_bvalid(dBus_axi_bvalid),
      .s_axi_bready(dBus_axi_bready),
      .s_axi_arid(dBus_axi_arid),
      .s_axi_araddr(dBus_axi_araddr),
      .s_axi_arlen(dBus_axi_arlen),
      .s_axi_arsize(dBus_axi_arsize),
      .s_axi_arburst(dBus_axi_arburst),
      .s_axi_arlock(dBus_axi_arlock[0]),
      .s_axi_arcache(dBus_axi_arcache),
      .s_axi_arprot(dBus_axi_arprot),
      .s_axi_arqos(dBus_axi_arqos),
      .s_axi_arvalid(dBus_axi_arvalid),
      .s_axi_arready(dBus_axi_arready),
      .s_axi_rid(dBus_axi_rid),
      .s_axi_rdata(dBus_axi_rdata),
      .s_axi_rresp(dBus_axi_rresp),
      .s_axi_rlast(dBus_axi_rlast),
      .s_axi_rvalid(dBus_axi_rvalid),
      .s_axi_rready(dBus_axi_rready),

      .m_axi_awid({peripheral_axi_awid, dBus_extmem_axi_awid}),
      .m_axi_awaddr({peripheral_axi_awaddr, dBus_extmem_axi_awaddr}),
      .m_axi_awlen({peripheral_axi_awlen, dBus_extmem_axi_awlen}),
      .m_axi_awsize({peripheral_axi_awsize, dBus_extmem_axi_awsize}),
      .m_axi_awburst({peripheral_axi_awburst, dBus_extmem_axi_awburst}),
      .m_axi_awlock({peripheral_axi_awlock[0], dBus_extmem_axi_awlock[0]}),
      .m_axi_awcache({peripheral_axi_awcache, dBus_extmem_axi_awcache}),
      .m_axi_awprot({peripheral_axi_awprot, dBus_extmem_axi_awprot}),
      .m_axi_awqos({peripheral_axi_awqos, dBus_extmem_axi_awqos}),
      .m_axi_awvalid({peripheral_axi_awvalid, dBus_extmem_axi_awvalid}),
      .m_axi_awready({peripheral_axi_awready, dBus_extmem_axi_awready}),
      .m_axi_wdata({peripheral_axi_wdata, dBus_extmem_axi_wdata}),
      .m_axi_wstrb({peripheral_axi_wstrb, dBus_extmem_axi_wstrb}),
      .m_axi_wlast({peripheral_axi_wlast, dBus_extmem_axi_wlast}),
      .m_axi_wvalid({peripheral_axi_wvalid, dBus_extmem_axi_wvalid}),
      .m_axi_wready({peripheral_axi_wready, dBus_extmem_axi_wready}),
      .m_axi_bid({peripheral_axi_bid, dBus_extmem_axi_bid}),
      .m_axi_bresp({peripheral_axi_bresp, dBus_extmem_axi_bresp}),
      .m_axi_bvalid({peripheral_axi_bvalid, dBus_extmem_axi_bvalid}),
      .m_axi_bready({peripheral_axi_bready, dBus_extmem_axi_bready}),
      .m_axi_arid({peripheral_axi_arid, dBus_extmem_axi_arid}),
      .m_axi_araddr({peripheral_axi_araddr, dBus_extmem_axi_araddr}),
      .m_axi_arlen({peripheral_axi_arlen, dBus_extmem_axi_arlen}),
      .m_axi_arsize({peripheral_axi_arsize, dBus_extmem_axi_arsize}),
      .m_axi_arburst({peripheral_axi_arburst, dBus_extmem_axi_arburst}),
      .m_axi_arlock({peripheral_axi_arlock[0], dBus_extmem_axi_arlock[0]}),
      .m_axi_arcache({peripheral_axi_arcache, dBus_extmem_axi_arcache}),
      .m_axi_arprot({peripheral_axi_arprot, dBus_extmem_axi_arprot}),
      .m_axi_arqos({peripheral_axi_arqos, dBus_extmem_axi_arqos}),
      .m_axi_arvalid({peripheral_axi_arvalid, dBus_extmem_axi_arvalid}),
      .m_axi_arready({peripheral_axi_arready, dBus_extmem_axi_arready}),
      .m_axi_rid({peripheral_axi_rid, dBus_extmem_axi_rid}),
      .m_axi_rdata({peripheral_axi_rdata, dBus_extmem_axi_rdata}),
      .m_axi_rresp({peripheral_axi_rresp, dBus_extmem_axi_rresp}),
      .m_axi_rlast({peripheral_axi_rlast, dBus_extmem_axi_rlast}),
      .m_axi_rvalid({peripheral_axi_rvalid, dBus_extmem_axi_rvalid}),
      .m_axi_rready({peripheral_axi_rready, dBus_extmem_axi_rready}),

      //optional signals
      .s_axi_awuser(1'b0),
      .s_axi_wuser (1'b0),
      .s_axi_aruser(1'b0),
      .m_axi_buser (2'b00),
      .m_axi_ruser (2'b00)
  );

  // instantiate axi2iob CPU data
  axi2iob #(
      .ADDR_WIDTH  (ADDR_W),
      .DATA_WIDTH  (DATA_W),
      .STRB_WIDTH  ((DATA_W / 8)),
      .AXI_ID_WIDTH(1)
  ) dBus_axi2iob (
      .clk_i(clk_i),
      .arst_i(arst_i),
      .s_axi_awid(peripheral_axi_awid),
      .s_axi_awaddr(peripheral_axi_awaddr),
      .s_axi_awlen(peripheral_axi_awlen),
      .s_axi_awsize(peripheral_axi_awsize),
      .s_axi_awburst(peripheral_axi_awburst),
      .s_axi_awlock(peripheral_axi_awlock[0]),
      .s_axi_awcache(peripheral_axi_awcache),
      .s_axi_awprot(peripheral_axi_awprot),
      .s_axi_awvalid(peripheral_axi_awvalid),
      .s_axi_awready(peripheral_axi_awready),
      .s_axi_wdata(peripheral_axi_wdata),
      .s_axi_wstrb(peripheral_axi_wstrb),
      .s_axi_wlast(peripheral_axi_wlast),
      .s_axi_wvalid(peripheral_axi_wvalid),
      .s_axi_wready(peripheral_axi_wready),
      .s_axi_bid(peripheral_axi_bid),
      .s_axi_bresp(peripheral_axi_bresp),
      .s_axi_bvalid(peripheral_axi_bvalid),
      .s_axi_bready(peripheral_axi_bready),
      .s_axi_arid(peripheral_axi_arid),
      .s_axi_araddr(peripheral_axi_araddr),
      .s_axi_arlen(peripheral_axi_arlen),
      .s_axi_arsize(peripheral_axi_arsize),
      .s_axi_arburst(peripheral_axi_arburst),
      .s_axi_arlock(peripheral_axi_arlock[0]),
      .s_axi_arcache(peripheral_axi_arcache),
      .s_axi_arprot(peripheral_axi_arprot),
      .s_axi_arvalid(peripheral_axi_arvalid),
      .s_axi_arready(peripheral_axi_arready),
      .s_axi_rid(peripheral_axi_rid),
      .s_axi_rdata(peripheral_axi_rdata),
      .s_axi_rresp(peripheral_axi_rresp),
      .s_axi_rlast(peripheral_axi_rlast),
      .s_axi_rvalid(peripheral_axi_rvalid),
      .s_axi_rready(peripheral_axi_rready),
      // IOb-bus signals
      .iob_avalid_o(peripheral_iob_avalid),
      .iob_addr_o(peripheral_iob_addr),
      .iob_wdata_o(peripheral_iob_wdata),
      .iob_wstrb_o(peripheral_iob_wstrb),
      .iob_rvalid_i(peripheral_iob_rvalid),
      .iob_rdata_i(peripheral_iob_rdata),
      .iob_ready_i(peripheral_iob_ready)
  );

  //slaves bus (includes internal memory + periphrals)
  wire [ (`IOB_SOC_OPENCRYPTOLINUX_N_SLAVES)*`REQ_W-1:0] slaves_req;
  wire [(`IOB_SOC_OPENCRYPTOLINUX_N_SLAVES)*`RESP_W-1:0] slaves_resp;

  iob_split #(
      .ADDR_W  (ADDR_W),
      .DATA_W  (DATA_W),
      .N_SLAVES(`IOB_SOC_OPENCRYPTOLINUX_N_SLAVES),
      .P_SLAVES(AddrMsb - 1)
  ) pbus_split (
      .clk_i(clk_i),
      .arst_i(arst_i),
      // master interface
      .m_req_i({
        peripheral_iob_avalid, peripheral_iob_addr, peripheral_iob_wdata, peripheral_iob_wstrb
      }),
      .m_resp_o({peripheral_iob_rdata, peripheral_iob_rvalid, peripheral_iob_ready}),
      // slaves interface
      .s_req_o(slaves_req),
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
      .i_req({
        iBus_intmem_iob_avalid, iBus_intmem_iob_addr, iBus_intmem_iob_wdata, iBus_intmem_iob_wstrb
      }),
      .i_resp({iBus_intmem_iob_rdata, iBus_intmem_iob_rvalid, iBus_intmem_iob_ready}),

      //data bus
      .d_req (slaves_req[0+:`REQ_W]),
      .d_resp(slaves_resp[0+:`RESP_W])
  );

  wire extmem_axi_arlock;
  wire extmem_axi_awlock;
  wire [7:0] extmem_axi_bid;
  wire [7:0] extmem_axi_rid;
  assign axi_awlock_o = {1'B0, extmem_axi_awlock};
  assign axi_arlock_o = {1'B0, extmem_axi_arlock};
  assign {iBus_extmem_axi_bid, dBus_extmem_axi_bid} = {extmem_axi_bid[4], extmem_axi_bid[0]};
  assign {iBus_extmem_axi_rid, dBus_extmem_axi_rid} = {extmem_axi_rid[4], extmem_axi_rid[0]};
  // Instructions bus interconnect {I, D} -> {internal, external}
  axi_interconnect #(
      .ID_WIDTH  (AXI_ID_W),
      .DATA_WIDTH(DATA_W),
      .ADDR_WIDTH(MEM_ADDR_W),
      .M_ADDR_WIDTH(MEM_ADDR_W),
      .S_COUNT   (2),
      .M_COUNT   (1)
  ) extmem_axi_interconnect (
      .clk(clk_i),
      .rst(arst_i),

      .s_axi_awid({3'b000, iBus_extmem_axi_awid, 3'b000, dBus_extmem_axi_awid}),
      .s_axi_awaddr({
        iBus_extmem_axi_awaddr[MEM_ADDR_W-1:0], dBus_extmem_axi_awaddr[MEM_ADDR_W-1:0]
      }),
      .s_axi_awlen({iBus_extmem_axi_awlen, dBus_extmem_axi_awlen}),
      .s_axi_awsize({iBus_extmem_axi_awsize, dBus_extmem_axi_awsize}),
      .s_axi_awburst({iBus_extmem_axi_awburst, dBus_extmem_axi_awburst}),
      .s_axi_awlock({iBus_extmem_axi_awlock[0], dBus_extmem_axi_awlock[0]}),
      .s_axi_awcache({iBus_extmem_axi_awcache, dBus_extmem_axi_awcache}),
      .s_axi_awprot({iBus_extmem_axi_awprot, dBus_extmem_axi_awprot}),
      .s_axi_awqos({iBus_extmem_axi_awqos, dBus_extmem_axi_awqos}),
      .s_axi_awvalid({iBus_extmem_axi_awvalid, dBus_extmem_axi_awvalid}),
      .s_axi_awready({iBus_extmem_axi_awready, dBus_extmem_axi_awready}),
      .s_axi_wdata({iBus_extmem_axi_wdata, dBus_extmem_axi_wdata}),
      .s_axi_wstrb({iBus_extmem_axi_wstrb, dBus_extmem_axi_wstrb}),
      .s_axi_wlast({iBus_extmem_axi_wlast, dBus_extmem_axi_wlast}),
      .s_axi_wvalid({iBus_extmem_axi_wvalid, dBus_extmem_axi_wvalid}),
      .s_axi_wready({iBus_extmem_axi_wready, dBus_extmem_axi_wready}),
      .s_axi_bid(extmem_axi_bid),
      .s_axi_bresp({iBus_extmem_axi_bresp, dBus_extmem_axi_bresp}),
      .s_axi_bvalid({iBus_extmem_axi_bvalid, dBus_extmem_axi_bvalid}),
      .s_axi_bready({iBus_extmem_axi_bready, dBus_extmem_axi_bready}),
      .s_axi_arid({3'b000, iBus_extmem_axi_arid, 3'b000, dBus_extmem_axi_arid}),
      .s_axi_araddr({
        iBus_extmem_axi_araddr[MEM_ADDR_W-1:0], dBus_extmem_axi_araddr[MEM_ADDR_W-1:0]
      }),
      .s_axi_arlen({iBus_extmem_axi_arlen, dBus_extmem_axi_arlen}),
      .s_axi_arsize({iBus_extmem_axi_arsize, dBus_extmem_axi_arsize}),
      .s_axi_arburst({iBus_extmem_axi_arburst, dBus_extmem_axi_arburst}),
      .s_axi_arlock({iBus_extmem_axi_arlock[0], dBus_extmem_axi_arlock[0]}),
      .s_axi_arcache({iBus_extmem_axi_arcache, dBus_extmem_axi_arcache}),
      .s_axi_arprot({iBus_extmem_axi_arprot, dBus_extmem_axi_arprot}),
      .s_axi_arqos({iBus_extmem_axi_arqos, dBus_extmem_axi_arqos}),
      .s_axi_arvalid({iBus_extmem_axi_arvalid, dBus_extmem_axi_arvalid}),
      .s_axi_arready({iBus_extmem_axi_arready, dBus_extmem_axi_arready}),
      .s_axi_rid(extmem_axi_rid),
      .s_axi_rdata({iBus_extmem_axi_rdata, dBus_extmem_axi_rdata}),
      .s_axi_rresp({iBus_extmem_axi_rresp, dBus_extmem_axi_rresp}),
      .s_axi_rlast({iBus_extmem_axi_rlast, dBus_extmem_axi_rlast}),
      .s_axi_rvalid({iBus_extmem_axi_rvalid, dBus_extmem_axi_rvalid}),
      .s_axi_rready({iBus_extmem_axi_rready, dBus_extmem_axi_rready}),

      .m_axi_awid(axi_awid_o),
      .m_axi_awaddr(internal_axi_awaddr_o),
      .m_axi_awlen(axi_awlen_o),
      .m_axi_awsize(axi_awsize_o),
      .m_axi_awburst(axi_awburst_o),
      .m_axi_awlock(extmem_axi_awlock),
      .m_axi_awcache(axi_awcache_o),
      .m_axi_awprot(axi_awprot_o),
      .m_axi_awqos(axi_awqos_o),
      .m_axi_awvalid(axi_awvalid_o),
      .m_axi_awready(axi_awready_i),
      .m_axi_wdata(axi_wdata_o),
      .m_axi_wstrb(axi_wstrb_o),
      .m_axi_wlast(axi_wlast_o),
      .m_axi_wvalid(axi_wvalid_o),
      .m_axi_wready(axi_wready_i),
      .m_axi_bid(axi_bid_i),
      .m_axi_bresp(axi_bresp_i),
      .m_axi_bvalid(axi_bvalid_i),
      .m_axi_bready(axi_bready_o),
      .m_axi_arid(axi_arid_o),
      .m_axi_araddr(internal_axi_araddr_o),
      .m_axi_arlen(axi_arlen_o),
      .m_axi_arsize(axi_arsize_o),
      .m_axi_arburst(axi_arburst_o),
      .m_axi_arlock(extmem_axi_arlock),
      .m_axi_arcache(axi_arcache_o),
      .m_axi_arprot(axi_arprot_o),
      .m_axi_arqos(axi_arqos_o),
      .m_axi_arvalid(axi_arvalid_o),
      .m_axi_arready(axi_arready_i),
      .m_axi_rid(axi_rid_i),
      .m_axi_rdata(axi_rdata_i),
      .m_axi_rresp(axi_rresp_i),
      .m_axi_rlast(axi_rlast_i),
      .m_axi_rvalid(axi_rvalid_i),
      .m_axi_rready(axi_rready_o),

      //optional signals
      .s_axi_awuser(2'b00),
      .s_axi_wuser (2'b00),
      .s_axi_aruser(2'b00),
      .m_axi_buser (1'b0),
      .m_axi_ruser (1'b0)
  );

  assign axi_awaddr_o[AXI_ADDR_W-1:0] = internal_axi_awaddr_o + MEM_ADDR_OFFSET;
  assign axi_araddr_o[AXI_ADDR_W-1:0] = internal_axi_araddr_o + MEM_ADDR_OFFSET;

  `include "iob_soc_opencryptolinux_periphs_inst.vs"

endmodule
