`timescale 1ns / 1ps

module iob2axi #(
    // Width of address bus in bits
    parameter ADDR_WIDTH   = 32,
    // Width of input (slave/master) AXI/IOb interface data bus in bits
    parameter DATA_WIDTH   = 32,
    // Width of input (slave/master) AXI/IOb interface wstrb (width of data bus in words)
    parameter STRB_WIDTH   = (DATA_WIDTH / 8),
    // Width of AXI ID signal
    parameter AXI_ID_WIDTH = 8,
    // Length of AXI read burst
    parameter AXI_RLEN_W   = 8'h00,
    // Length of AXI write burst
    parameter AXI_WLEN_W   = 8'h00
) (
    input  wire                    clk_i,          //System clock input
    input  wire                    cke_i,          //System clock enable
    input  wire                    arst_i,         //System reset, asynchronous and active high
    /*
     * IOb-bus slave interface
     */
    input  wire                    iob_avalid_i,
    input  wire [  ADDR_WIDTH-1:0] iob_addr_i,
    input  wire [  DATA_WIDTH-1:0] iob_wdata_i,
    input  wire [  STRB_WIDTH-1:0] iob_wstrb_i,
    output wire [  DATA_WIDTH-1:0] iob_rdata_o,
    output wire                    iob_rvalid_o,
    output wire                    iob_ready_o,
    /*
     * AXI master interface
     */
    output wire [AXI_ID_WIDTH-1:0] axi_awid_o,
    output wire [  ADDR_WIDTH-1:0] axi_awaddr_o,
    output wire [             7:0] axi_awlen_o,
    output wire [             2:0] axi_awsize_o,
    output wire [             1:0] axi_awburst_o,
    output wire                    axi_awlock_o,
    output wire [             3:0] axi_awcache_o,
    output wire [             2:0] axi_awprot_o,
    output wire                    axi_awvalid_o,
    input  wire                    axi_awready_i,
    output wire [  DATA_WIDTH-1:0] axi_wdata_o,
    output wire [  STRB_WIDTH-1:0] axi_wstrb_o,
    output wire                    axi_wlast_o,
    output wire                    axi_wvalid_o,
    input  wire                    axi_wready_i,
    input  wire [AXI_ID_WIDTH-1:0] axi_bid_i,
    input  wire [             1:0] axi_bresp_i,
    input  wire                    axi_bvalid_i,
    output wire                    axi_bready_o,
    output wire [AXI_ID_WIDTH-1:0] axi_arid_o,
    output wire [  ADDR_WIDTH-1:0] axi_araddr_o,
    output wire [             7:0] axi_arlen_o,
    output wire [             2:0] axi_arsize_o,
    output wire [             1:0] axi_arburst_o,
    output wire                    axi_arlock_o,
    output wire [             3:0] axi_arcache_o,
    output wire [             2:0] axi_arprot_o,
    output wire                    axi_arvalid_o,
    input  wire                    axi_arready_i,
    input  wire [AXI_ID_WIDTH-1:0] axi_rid_i,
    input  wire [  DATA_WIDTH-1:0] axi_rdata_i,
    input  wire [             1:0] axi_rresp_i,
    input  wire                    axi_rlast_i,
    input  wire                    axi_rvalid_i,
    output wire                    axi_rready_o
);

  localparam AXI_RLEN = 2 ** AXI_RLEN_W;
  localparam AXI_WLEN = 2 ** AXI_WLEN_W;

  //
  // Input/Write bus FIFO
  //
  wire in_fifo_full;
  wire in_fifo_wr;
  wire [DATA_WIDTH-1:0] in_fifo_wdata;

  wire in_fifo_empty;
  wire in_fifo_rd;
  wire [DATA_WIDTH-1:0] in_fifo_rdata;

  wire [AXI_WLEN:0] in_fifo_level;
  wire in_ext_mem_clk;
  wire in_ext_mem_w_en;
  wire [AXI_RLEN-1:0] in_ext_mem_w_addr;
  wire [DATA_WIDTH-1:0] in_ext_mem_w_data;
  wire in_ext_mem_r_en;
  wire [AXI_RLEN-1:0] in_ext_mem_r_addr;
  wire [DATA_WIDTH-1:0] in_ext_mem_r_data;

  //
  // Output/Read bus FIFO
  //
  wire out_fifo_full;
  wire out_fifo_wr;
  wire [DATA_WIDTH-1:0] out_fifo_wdata;

  wire out_fifo_empty;
  wire out_fifo_rd;
  wire [DATA_WIDTH-1:0] out_fifo_rdata;

  wire [AXI_RLEN:0] out_fifo_level;

  wire out_ext_mem_clk;
  wire out_ext_mem_w_en;
  wire [AXI_RLEN-1:0] out_ext_mem_w_addr;
  wire [DATA_WIDTH-1:0] out_ext_mem_w_data;
  wire out_ext_mem_r_en;
  wire [AXI_RLEN-1:0] out_ext_mem_r_addr;
  wire [DATA_WIDTH-1:0] out_ext_mem_r_data;

  assign in_fifo_wr = s_valid_i & |s_wstrb_i & ~in_fifo_full;
  assign in_fifo_wdata = {s_wdata_i, s_wstrb_i};
  assign in_fifo_rd = wr_valid;
  //
  // Output/Read bus FIFO
  //
  assign out_fifo_wr = rd_valid & |rd_wstrb & ~out_fifo_full;
  assign out_fifo_wdata = rd_wdata;
  assign out_fifo_rd = s_valid_i & ~|s_wstrb_i & ~out_fifo_empty;

  iob_fifo_sync #(
      .W_DATA_W(DATA_WIDTH),
      .R_DATA_W(DATA_WIDTH),
      .ADDR_W  (AXI_WLEN)
  ) write_bus_iob_fifo_sync (
      .clk_i(clk_i),
      .rst_i(rst_i),

      .w_en_i  (in_fifo_wr),
      .w_data_i(in_fifo_wdata),
      .w_full_o(in_fifo_full),

      .r_en_i   (in_fifo_rd),
      .r_data_o (in_fifo_rdata),
      .r_empty_o(in_fifo_empty),

      .ext_mem_clk_o(in_ext_mem_clk),
      .ext_mem_w_en_o(in_ext_mem_w_en),
      .ext_mem_w_addr_o(in_ext_mem_w_addr),
      .ext_mem_w_data_o(in_ext_mem_w_data),

      .ext_mem_r_en_o  (in_ext_mem_r_en),
      .ext_mem_r_addr_o(in_ext_mem_r_addr),
      .ext_mem_r_data_i(in_ext_mem_r_data),

      .level_o(in_fifo_level)
  );

  iob_ram_2p #(
      .DATA_W(DATA_WIDTH),
      .ADDR_W(AXI_WLEN)
  ) write_bus_ram (
      .clk_i(in_ext_mem_clk),

      .w_en_i  (in_ext_mem_w_en),
      .w_addr_i(in_ext_mem_w_addr),
      .w_data_i(in_ext_mem_w_data),

      .r_en_i  (in_ext_mem_r_en),
      .r_addr_i(in_ext_mem_r_addr),
      .r_data_o(in_ext_mem_r_data)
  );

  iob_fifo_sync #(
      .W_DATA_W(DATA_WIDTH),
      .R_DATA_W(DATA_WIDTH),
      .ADDR_W  (AXI_RLEN)
  ) read_bus_iob_fifo_sync (
      .clk_i(clk_i),
      .rst_i(rst_i),

      .w_en_i  (out_fifo_wr),
      .w_data_i(out_fifo_wdata),
      .w_full_o(out_fifo_full),

      .r_en_i   (out_fifo_rd),
      .r_data_o (out_fifo_rdata),
      .r_empty_o(out_fifo_empty),

      .ext_mem_clk_o(out_ext_mem_clk),
      .ext_mem_w_en_o(out_ext_mem_w_en),
      .ext_mem_w_addr_o(out_ext_mem_w_addr),
      .ext_mem_w_data_o(out_ext_mem_w_data),

      .ext_mem_r_en_o  (out_ext_mem_r_en),
      .ext_mem_r_addr_o(out_ext_mem_r_addr),
      .ext_mem_r_data_i(out_ext_mem_r_data),

      .level_o(out_fifo_level)
  );

  iob_ram_2p #(
      .DATA_W(DATA_WIDTH),
      .ADDR_W(AXI_RLEN)
  ) read_bus_ram (
      .clk_i(out_ext_mem_clk),

      .w_en_i  (out_ext_mem_w_en),
      .w_addr_i(out_ext_mem_w_addr),
      .w_data_i(out_ext_mem_w_data),

      .r_en_i  (out_ext_mem_r_en),
      .r_addr_i(out_ext_mem_r_addr),
      .r_data_o(out_ext_mem_r_data)
  );

endmodule
