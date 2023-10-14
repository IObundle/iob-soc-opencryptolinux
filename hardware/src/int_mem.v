`timescale 1 ns / 1 ps

`include "iob_soc_opencryptolinux_conf.vh"
`include "iob_utils.vh"

module int_mem #(
    parameter ADDR_W         = 0,
    parameter DATA_W         = 0,
    parameter STRB_W         = (DATA_W / 8),
    parameter AXI_ID_W       = 1,
    parameter BOOT_HEXFILE   = "none",
    parameter BOOTROM_ADDR_W = 1,
    parameter B_BIT          = 2,
    parameter SRAM_ADDR_W    = (B_BIT - 1)
) (

    output wire                boot,
    output wire                cpu_reset,
    /*
     * AXI instruction bus interface
     */
    input  wire [AXI_ID_W-1:0] i_axi_awid,
    input  wire [  ADDR_W-1:0] i_axi_awaddr,
    input  wire [         7:0] i_axi_awlen,
    input  wire [         2:0] i_axi_awsize,
    input  wire [         1:0] i_axi_awburst,
    input  wire                i_axi_awlock,
    input  wire [         3:0] i_axi_awcache,
    input  wire [         2:0] i_axi_awprot,
    input  wire                i_axi_awvalid,
    output wire                i_axi_awready,
    input  wire [  DATA_W-1:0] i_axi_wdata,
    input  wire [  STRB_W-1:0] i_axi_wstrb,
    input  wire                i_axi_wlast,
    input  wire                i_axi_wvalid,
    output wire                i_axi_wready,
    output wire [AXI_ID_W-1:0] i_axi_bid,
    output wire [         1:0] i_axi_bresp,
    output wire                i_axi_bvalid,
    input  wire                i_axi_bready,
    input  wire [AXI_ID_W-1:0] i_axi_arid,
    input  wire [  ADDR_W-1:0] i_axi_araddr,
    input  wire [         7:0] i_axi_arlen,
    input  wire [         2:0] i_axi_arsize,
    input  wire [         1:0] i_axi_arburst,
    input  wire                i_axi_arlock,
    input  wire [         3:0] i_axi_arcache,
    input  wire [         2:0] i_axi_arprot,
    input  wire                i_axi_arvalid,
    output wire                i_axi_arready,
    output wire [AXI_ID_W-1:0] i_axi_rid,
    output wire [  DATA_W-1:0] i_axi_rdata,
    output wire [         1:0] i_axi_rresp,
    output wire                i_axi_rlast,
    output wire                i_axi_rvalid,
    input  wire                i_axi_rready,

    /*
     * AXI data bus interface
     */
    input  wire [AXI_ID_W-1:0] d_axi_awid,
    input  wire [  ADDR_W-1:0] d_axi_awaddr,
    input  wire [         7:0] d_axi_awlen,
    input  wire [         2:0] d_axi_awsize,
    input  wire [         1:0] d_axi_awburst,
    input  wire                d_axi_awlock,
    input  wire [         3:0] d_axi_awcache,
    input  wire [         2:0] d_axi_awprot,
    input  wire                d_axi_awvalid,
    output wire                d_axi_awready,
    input  wire [  DATA_W-1:0] d_axi_wdata,
    input  wire [  STRB_W-1:0] d_axi_wstrb,
    input  wire                d_axi_wlast,
    input  wire                d_axi_wvalid,
    output wire                d_axi_wready,
    output wire [AXI_ID_W-1:0] d_axi_bid,
    output wire [         1:0] d_axi_bresp,
    output wire                d_axi_bvalid,
    input  wire                d_axi_bready,
    input  wire [AXI_ID_W-1:0] d_axi_arid,
    input  wire [  ADDR_W-1:0] d_axi_araddr,
    input  wire [         7:0] d_axi_arlen,
    input  wire [         2:0] d_axi_arsize,
    input  wire [         1:0] d_axi_arburst,
    input  wire                d_axi_arlock,
    input  wire [         3:0] d_axi_arcache,
    input  wire [         2:0] d_axi_arprot,
    input  wire                d_axi_arvalid,
    output wire                d_axi_arready,
    output wire [AXI_ID_W-1:0] d_axi_rid,
    output wire [  DATA_W-1:0] d_axi_rdata,
    output wire [         1:0] d_axi_rresp,
    output wire                d_axi_rlast,
    output wire                d_axi_rvalid,
    input  wire                d_axi_rready,

    `include "clk_en_rst_s_port.vs"
);

  localparam RAM_AXI_ID_W = 1;
  localparam RAM_AXI_LEN_W = 8;
  localparam RAM_AXI_ADDR_W = SRAM_ADDR_W;
  localparam RAM_AXI_DATA_W = DATA_W;

  wire mm_reg_enable;
  wire int_d_axi_awvalid;
  wire int_d_axi_wvalid;
  //boot register: (1) load bootloader to sram and run it: (0) run program
  wire boot_wr;
  reg  boot_nxt;
  //create CPU reset pulse
  wire cpu_rst_req;
  `include "ram_axi_wire.vs"

  assign mm_reg_enable = d_axi_awaddr[B_BIT];
  assign boot_wr = mm_reg_enable & d_axi_wvalid & (|d_axi_wstrb);
  assign cpu_rst_req = mm_reg_enable & d_axi_wvalid & (|d_axi_wstrb) & d_axi_wdata[1];

  assign int_d_axi_awvalid = (~mm_reg_enable) & d_axi_awvalid;
  assign int_d_axi_wvalid = (~mm_reg_enable) & d_axi_wvalid;

  assign ram_axi_awlock[1] = 1'b0;
  assign ram_axi_arlock[1] = 1'b0;

  iob_reg_re #(
      .DATA_W (1),
      .RST_VAL(1)
  ) bootnxt (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (boot_wr),
      .data_i(d_axi_wdata[0]),
      .data_o(boot_nxt)
  );
  iob_reg_r #(
      .DATA_W (1),
      .RST_VAL(1)
  ) bootreg (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .data_i(boot_nxt),
      .data_o(boot)
  );
  iob_pulse_gen #(
      .START   (0),
      .DURATION(100)
  ) reset_pulse (
      .clk_i  (clk_i),
      .arst_i (arst_i),
      .cke_i  (cke_i),
      .start_i(cpu_rst_req),
      .pulse_o(cpu_reset)
  );

  axi_interconnect #(
      .ID_WIDTH  (AXI_ID_W),
      .DATA_WIDTH(DATA_W),
      .ADDR_WIDTH(SRAM_ADDR_W),
      .M_ADDR_WIDTH(SRAM_ADDR_W),
      .S_COUNT   (2),
      .M_COUNT   (1)
  ) sram_axi_interconnect (
      .clk(clk_i),
      .rst(arst_i),

      .s_axi_awid({i_axi_awid, d_axi_awid}),
      .s_axi_awaddr({i_axi_awaddr[SRAM_ADDR_W-1:0], d_axi_awaddr[SRAM_ADDR_W-1:0]}),
      .s_axi_awlen({i_axi_awlen, d_axi_awlen}),
      .s_axi_awsize({i_axi_awsize, d_axi_awsize}),
      .s_axi_awburst({i_axi_awburst, d_axi_awburst}),
      .s_axi_awlock({i_axi_awlock, d_axi_awlock}),
      .s_axi_awcache({i_axi_awcache, d_axi_awcache}),
      .s_axi_awprot({i_axi_awprot, d_axi_awprot}),
      .s_axi_awqos({4'h0, 4'h0}),
      .s_axi_awvalid({i_axi_awvalid, int_d_axi_awvalid}),
      .s_axi_awready({i_axi_awready, d_axi_awready}),
      .s_axi_wdata({i_axi_wdata, d_axi_wdata}),
      .s_axi_wstrb({i_axi_wstrb, d_axi_wstrb}),
      .s_axi_wlast({i_axi_wlast, d_axi_wlast}),
      .s_axi_wvalid({i_axi_wvalid, int_d_axi_wvalid}),
      .s_axi_wready({i_axi_wready, d_axi_wready}),
      .s_axi_bid({i_axi_bid, d_axi_bid}),
      .s_axi_bresp({i_axi_bresp, d_axi_bresp}),
      .s_axi_bvalid({i_axi_bvalid, d_axi_bvalid}),
      .s_axi_bready({i_axi_bready, d_axi_bready}),
      .s_axi_arid({i_axi_arid, d_axi_arid}),
      .s_axi_araddr({i_axi_araddr[SRAM_ADDR_W-1:0], d_axi_araddr[SRAM_ADDR_W-1:0]}),
      .s_axi_arlen({i_axi_arlen, d_axi_arlen}),
      .s_axi_arsize({i_axi_arsize, d_axi_arsize}),
      .s_axi_arburst({i_axi_arburst, d_axi_arburst}),
      .s_axi_arlock({i_axi_arlock, d_axi_arlock}),
      .s_axi_arcache({i_axi_arcache, d_axi_arcache}),
      .s_axi_arprot({i_axi_arprot, d_axi_arprot}),
      .s_axi_arqos({4'h0, 4'h0}),
      .s_axi_arvalid({i_axi_arvalid, d_axi_arvalid}),
      .s_axi_arready({i_axi_arready, d_axi_arready}),
      .s_axi_rid({i_axi_rid, d_axi_rid}),
      .s_axi_rdata({i_axi_rdata, d_axi_rdata}),
      .s_axi_rresp({i_axi_rresp, d_axi_rresp}),
      .s_axi_rlast({i_axi_rlast, d_axi_rlast}),
      .s_axi_rvalid({i_axi_rvalid, d_axi_rvalid}),
      .s_axi_rready({i_axi_rready, d_axi_rready}),

      .m_axi_awid(ram_axi_awid),
      .m_axi_awaddr(ram_axi_awaddr),
      .m_axi_awlen(ram_axi_awlen),
      .m_axi_awsize(ram_axi_awsize),
      .m_axi_awburst(ram_axi_awburst),
      .m_axi_awlock(ram_axi_awlock[1]),
      .m_axi_awcache(ram_axi_awcache),
      .m_axi_awprot(ram_axi_awprot),
      .m_axi_awqos(ram_axi_awqos),
      .m_axi_awvalid(ram_axi_awvalid),
      .m_axi_awready(ram_axi_awready),
      .m_axi_wdata(ram_axi_wdata),
      .m_axi_wstrb(ram_axi_wstrb),
      .m_axi_wlast(ram_axi_wlast),
      .m_axi_wvalid(ram_axi_wvalid),
      .m_axi_wready(ram_axi_wready),
      .m_axi_bid(ram_axi_bid),
      .m_axi_bresp(ram_axi_bresp),
      .m_axi_bvalid(ram_axi_bvalid),
      .m_axi_bready(ram_axi_bready),
      .m_axi_arid(ram_axi_arid),
      .m_axi_araddr(ram_axi_araddr),
      .m_axi_arlen(ram_axi_arlen),
      .m_axi_arsize(ram_axi_arsize),
      .m_axi_arburst(ram_axi_arburst),
      .m_axi_arlock(ram_axi_arlock[1]),
      .m_axi_arcache(ram_axi_arcache),
      .m_axi_arprot(ram_axi_arprot),
      .m_axi_arqos(ram_axi_arqos),
      .m_axi_arvalid(ram_axi_arvalid),
      .m_axi_arready(ram_axi_arready),
      .m_axi_rid(ram_axi_rid),
      .m_axi_rdata(ram_axi_rdata),
      .m_axi_rresp(ram_axi_rresp),
      .m_axi_rlast(ram_axi_rlast),
      .m_axi_rvalid(ram_axi_rvalid),
      .m_axi_rready(ram_axi_rready),

      //optional signals
      .s_axi_awuser(2'b00),
      .s_axi_wuser (2'b00),
      .s_axi_aruser(2'b00),
      .m_axi_buser (1'b0),
      .m_axi_ruser (1'b0)
  );

  axi_ram #(
      .DATA_WIDTH(DATA_W),
      .ADDR_WIDTH(SRAM_ADDR_W),
      .ID_WIDTH(AXI_ID_W),
      .FILE(BOOT_HEXFILE),
      .FILE_SIZE(2**(BOOTROM_ADDR_W-2))
  ) boot_ram (
      .clk_i(clk_i),
      .rst_i(arst_i),

      .axi_awid_i(ram_axi_awid),
      .axi_awaddr_i(ram_axi_awaddr),
      .axi_awlen_i(ram_axi_awlen),
      .axi_awsize_i(ram_axi_awsize),
      .axi_awburst_i(ram_axi_awburst),
      .axi_awlock_i(ram_axi_awlock),
      .axi_awcache_i(ram_axi_awcache),
      .axi_awprot_i(ram_axi_awprot),
      .axi_awqos_i(ram_axi_awqos),
      .axi_awvalid_i(ram_axi_awvalid),
      .axi_awready_o(ram_axi_awready),
      .axi_wdata_i(ram_axi_wdata),
      .axi_wstrb_i(ram_axi_wstrb),
      .axi_wlast_i(ram_axi_wlast),
      .axi_wvalid_i(ram_axi_wvalid),
      .axi_wready_o(ram_axi_wready),
      .axi_bid_o(ram_axi_bid),
      .axi_bresp_o(ram_axi_bresp),
      .axi_bvalid_o(ram_axi_bvalid),
      .axi_bready_i(ram_axi_bready),
      .axi_arid_i(ram_axi_arid),
      .axi_araddr_i(ram_axi_araddr),
      .axi_arlen_i(ram_axi_arlen),
      .axi_arsize_i(ram_axi_arsize),
      .axi_arburst_i(ram_axi_arburst),
      .axi_arlock_i(ram_axi_arlock),
      .axi_arcache_i(ram_axi_arcache),
      .axi_arprot_i(ram_axi_arprot),
      .axi_arqos_i(ram_axi_arqos),
      .axi_arvalid_i(ram_axi_arvalid),
      .axi_arready_o(ram_axi_arready),
      .axi_rid_o(ram_axi_rid),
      .axi_rdata_o(ram_axi_rdata),
      .axi_rresp_o(ram_axi_rresp),
      .axi_rlast_o(ram_axi_rlast),
      .axi_rvalid_o(ram_axi_rvalid),
      .axi_rready_i(ram_axi_rready)
  );

endmodule
