`timescale 1 ns / 1 ps

`include "iob_utils.vh"

module ext_mem #(
    parameter ADDR_W      = 0,
    parameter DATA_W      = 0,
    parameter MEM_ADDR_W  = 0,
    parameter AXI_ID_W    = 0
) (
    // Instruction bus
    input  [ `REQ_W-1:0] i_req,
    output [`RESP_W-1:0] i_resp,

    // Data bus
    input  [ `REQ_W-1:0] d_req,
    output [`RESP_W-1:0] d_resp,

    // AXI interface
    `include "axi_m_port.vs"
    `include "clk_en_rst_s_port.vs"
);

  // External memory interface signals
  wire [ `REQ_W-1:0] ext_mem_req;
  wire [`RESP_W-1:0] ext_mem_resp;

  // Merge cache back-ends
  iob_merge #(
      .ADDR_W   (ADDR_W),
      .N_MASTERS(2)
  ) merge_i_d_buses_into_l2 (
      .clk_i   (clk_i),
      .arst_i  (arst_i),
      // masters
      .m_req_i ({i_req, d_req}),
      .m_resp_o({i_resp, d_resp}),
      // slave
      .s_req_o (ext_mem_req),
      .s_resp_i(ext_mem_resp)
  );

  // AXI interface
  iob2axi #(
      .ADDR_WIDTH(MEM_ADDR_W),
      .DATA_WIDTH(DATA_W),
      .AXI_ID_WIDTH(AXI_ID_W),
      .AXI_RLEN(8'h0f),
      .AXI_WLEN(8'h00)
  ) ext_mem_iob2axi (
      .iob_avalid_i(ext_mem_req[`AVALID(0)]),
      .iob_addr_i  (ext_mem_req[`ADDRESS(0, MEM_ADDR_W)]),
      .iob_wdata_i (ext_mem_req[`WDATA(0)]),
      .iob_wstrb_i (ext_mem_req[`WSTRB(0)]),
      .iob_rdata_o (ext_mem_resp[`RVALID(0)]),
      .iob_rvalid_o(ext_mem_resp[`RDATA(0)]),
      .iob_rready_o(ext_mem_resp[`READY(0)]),
      `include "axi_m_m_portmap.vs"
      .clk_i       (clk_i),
      .cke_i       (cke_i),
      .arst_i      (arst_i)
  );

endmodule
