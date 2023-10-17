`timescale 1 ns / 1 ps

module boot_ctr #(
    parameter ADDR_W = 0,
    parameter DATA_W = 0,
    parameter STRB_W = (DATA_W / 8)
) (
    output wire boot,
    output wire cpu_reset,

    input  [     1-1:0] iob_avalid_i,
    input  [ADDR_W-1:0] iob_addr_i,
    input  [DATA_W-1:0] iob_wdata_i,
    input  [STRB_W-1:0] iob_wstrb_i,
    output [     1-1:0] iob_rvalid_o,
    output [DATA_W-1:0] iob_rdata_o,
    output [     1-1:0] iob_ready_o,

    `include "clk_en_rst_s_port.vs"
);

  //boot register: (1) load bootloader to sram and run it: (0) run program
  wire boot_wr;
  reg  boot_nxt;
  //create CPU reset pulse
  wire cpu_rst_req;
  wire rst_sync;

  assign boot_wr = iob_avalid_i & (|iob_wstrb_i);
  assign cpu_rst_req = (iob_avalid_i & (|iob_wstrb_i) & iob_wdata_i[1]) | rst_sync;

  iob_reg_re #(
      .DATA_W (1),
      .RST_VAL(1)
  ) bootnxt (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (boot_wr),
      .data_i(iob_wdata_i[0]),
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
  iob_reg #(
      .DATA_W (1),
      .RST_VAL(1)
  ) sync_reset (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .data_i(1'b0),
      .data_o(rst_sync)
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

endmodule
