`timescale 1ns / 1ps

module iob2axi #(
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of input (slave/master) AXI/IOb interface data bus in bits
    parameter DATA_WIDTH = 32,
    // Width of input (slave/master) AXI/IOb interface wstrb (width of data bus in words)
    parameter STRB_WIDTH = (DATA_WIDTH / 8),
    // Width of AXI ID signal
    parameter AXI_ID_WIDTH = 8,
    // Length of AXI read burst (Burst_Length = AxLEN[7:0] + 1)
    parameter AXI_RLEN = 8'h00,
    // Length of AXI write burst (Burst_Length = AxLEN[7:0] + 1)
    parameter AXI_WLEN = 8'h00
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
    output reg  [  DATA_WIDTH-1:0] iob_rdata_o,
    output reg                     iob_rvalid_o,
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

  localparam integer BurstRLEN = AXI_RLEN + 1;
  localparam integer BurstWLEN = AXI_WLEN + 1;
  localparam integer IDLE = 0, WriteFIFO = 1, ReadFIFO = 2;

  wire                             iob_we;
  // Write bus
  wire                             w_fifo_full;
  wire                             w_fifo_wr;
  wire [DATA_WIDTH+STRB_WIDTH-1:0] w_fifo_wdata;
  wire                             w_fifo_empty;
  wire                             w_fifo_rd;
  wire [DATA_WIDTH+STRB_WIDTH-1:0] w_fifo_rdata;
  wire [               AXI_WLEN:0] w_fifo_level;
  wire                             w_ext_mem_clk;
  wire                             w_ext_mem_w_en;
  wire [             AXI_WLEN-1:0] w_ext_mem_w_addr;
  wire [DATA_WIDTH+STRB_WIDTH-1:0] w_ext_mem_w_data;
  wire                             w_ext_mem_r_en;
  wire [             AXI_WLEN-1:0] w_ext_mem_r_addr;
  wire [DATA_WIDTH+STRB_WIDTH-1:0] w_ext_mem_r_data;
  wire                             iob_wvalid;
  wire                             iob_waddr_q;
  wire                             iob_wready;
  wire                             w_state_q;
  reg                              w_state_next;
  // Read bus
  wire                             r_fifo_full;
  wire                             r_fifo_wr;
  wire [           DATA_WIDTH-1:0] r_fifo_wdata;
  wire                             r_fifo_empty;
  wire                             r_fifo_rd;
  wire [           DATA_WIDTH-1:0] r_fifo_rdata;
  wire [               AXI_RLEN:0] r_fifo_level;
  wire                             r_ext_mem_clk;
  wire                             r_ext_mem_w_en;
  wire [             AXI_RLEN-1:0] r_ext_mem_w_addr;
  wire [           DATA_WIDTH-1:0] r_ext_mem_w_data;
  wire                             r_ext_mem_r_en;
  wire [             AXI_RLEN-1:0] r_ext_mem_r_addr;
  wire [           DATA_WIDTH-1:0] r_ext_mem_r_data;
  wire                             iob_arvalid;
  wire                             iob_arready;
  wire                             iob_raddr_q;
  reg                              iob_rvalid_next;
  wire                             r_state_q;
  reg                              r_state_next;

  assign iob_we        = |iob_wstrb_i;
  assign iob_rready_o  = iob_we ? iob_wready : iob_arready;
  // Write bus
  // // IOb-bus
  assign iob_wvalid    = iob_we & iob_avalid_i;
  assign iob_wready    = axi_awready_i;
  assign iob_wready    = axi_wready_i;
  // // AXI
  assign axi_awid_o    = {AXI_ID_WIDTH{1'b0}};
  assign axi_awaddr_o  = iob_wvalid ? iob_addr_i : iob_waddr_q;
  assign axi_awlen_o   = AXI_WLEN;
  assign axi_awsize_o  = 3'b010;
  assign axi_awburst_o = 2'b01;
  assign axi_awlock_o  = 1'b0;
  assign axi_awcache_o = 4'hf;
  assign axi_awprot_o  = 3'b010;
  assign axi_awvalid_o = iob_wvalid;
  assign axi_wdata_o   = iob_wdata_i;
  assign axi_wstrb_o   = iob_wstrb_i;
  assign axi_wlast_o   = 1'b1;
  assign axi_wvalid_o  = iob_wvalid;
  assign axi_bid_i     = {AXI_ID_WIDTH{1'b0}};
  assign axi_bresp_i   = 2'b00;
  assign axi_bvalid_i  = 1'b0;
  assign axi_bready_o  = 1'b1;
  // Read bus
  // // IOb-bus
  assign iob_arvalid   = ~iob_we & iob_avalid_i;
  // // AXI
  assign axi_arid_o    = {AXI_ID_WIDTH{1'b0}};
  assign axi_araddr_o  = iob_arvalid ? iob_addr_i : iob_raddr_q;
  assign axi_arlen_o   = AXI_RLEN;
  assign axi_arsize_o  = 3'b010;
  assign axi_arburst_o = 2'b01;
  assign axi_arlock_o  = 1'b0;
  assign axi_arcache_o = 4'hf;
  assign axi_arprot_o  = 3'b010;
  assign axi_rready_o  = 1'b1;


  // Write bus FSM
  always @(*) begin
    axi_awvalid_o = 1'b0;
    axi_wvalid_o  = 1'b0;
    case (w_state_q)
      default: begin
        iob_wready = 1'b1;
        if (iob_wvalid) begin
          r_state_next = WriteFIFO;
          w_fifo_wr = 1'b1;
          w_fifo_wdata = {iob_wdata_i, iob_wstrb_i};
        end else begin
          r_state_next = IDLE;
        end
      end
      WriteFIFO: begin
        w_fifo_wr = 1'b0;
        if (w_fifo_full) begin
          w_state_next = IDLE;
          iob_wready   = 1'b0;
        end else begin
          w_state_next = WriteFIFO;
          if (iob_wvalid) begin
            w_fifo_wr = 1'b1;
            w_fifo_wdata = {iob_wdata_i, iob_wstrb_i};
          end
        end
      end
      ReadFIFO: begin
        if (w_fifo_empty) begin
          w_state_next = IDLE;
          iob_wready   = 1'b1;
        end else begin
          w_state_next = ReadFIFO;
        end
      end
    endcase
  end

  // Read bus FSM
  always @(*) begin
    iob_rvalid_next = 1'b0;
    iob_arready = 1'b0;
    r_fifo_wr = 1'b0;
    case (r_state_q)
      default: begin
        iob_arready = axi_arready_i;
        if (iob_arvalid & axi_arready_i) begin
          r_state_next  = WriteFIFO;
          axi_arvalid_o = 1'b1;
        end else begin
          r_state_next = IDLE;
        end
      end
      WriteFIFO: begin
        if (r_fifo_full) begin
          r_state_next = ReadFIFO;
          r_fifo_rd = 1'b1;
          iob_rvalid_next = 1'b1;
        end else begin
          r_state_next = WriteFIFO;
          if(axi_rvalid_i) begin
            r_fifo_wr = 1'b1;
            r_fifo_wdata = axi_rdata_i;
          end
        end
      end
      ReadFIFO: begin
        iob_arready = 1'b1;
        if (r_fifo_empty) begin
          r_state_next = IDLE;
          r_fifo_rd = 1'b0;
          iob_rvalid_o = 1'b0;
          iob_rdata_o = {DATA_WIDTH{1'b0}};
        end else begin
          r_state_next = ReadFIFO;
          iob_rdata_o  = r_fifo_rdata;
          if (iob_arvalid) begin
            r_fifo_rd = 1'b1;
            iob_rvalid_next = 1'b1;
          end
        end
      end
    endcase
  end


  //
  // Write bus FIFO
  //
  iob_fifo_sync #(
      .W_DATA_W(DATA_WIDTH + STRB_WIDTH),
      .R_DATA_W(DATA_WIDTH + STRB_WIDTH),
      .ADDR_W  (BurstWLEN)
  ) write_bus_iob_fifo_sync (
      .clk_i(clk_i),
      .rst_i(rst_i),

      .w_en_i  (w_fifo_wr),
      .w_data_i(w_fifo_wdata),
      .w_full_o(w_fifo_full),

      .r_en_i   (w_fifo_rd),
      .r_data_o (w_fifo_rdata),
      .r_empty_o(w_fifo_empty),

      .ext_mem_clk_o(w_ext_mem_clk),
      .ext_mem_w_en_o(w_ext_mem_w_en),
      .ext_mem_w_addr_o(w_ext_mem_w_addr),
      .ext_mem_w_data_o(w_ext_mem_w_data),

      .ext_mem_r_en_o  (w_ext_mem_r_en),
      .ext_mem_r_addr_o(w_ext_mem_r_addr),
      .ext_mem_r_data_i(w_ext_mem_r_data),

      .level_o(w_fifo_level)
  );

  iob_ram_2p #(
      .DATA_W(DATA_WIDTH + STRB_WIDTH),
      .ADDR_W(BurstWLEN)
  ) write_bus_ram (
      .clk_i(w_ext_mem_clk),

      .w_en_i  (w_ext_mem_w_en),
      .w_addr_i(w_ext_mem_w_addr),
      .w_data_i(w_ext_mem_w_data),

      .r_en_i  (w_ext_mem_r_en),
      .r_addr_i(w_ext_mem_r_addr),
      .r_data_o(w_ext_mem_r_data)
  );

  iob_reg_re #(
      .DATA_W (ADDR_WIDTH),
      .RST_VAL(0)
  ) iob_reg_iob_waddr (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (iob_wvalid),
      .data_i(iob_addr_i),
      .data_o(iob_waddr_q)
  );

  iob_reg_re #(
      .DATA_W (1),
      .RST_VAL(0)
  ) iob_reg_w_state (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (1'b1),
      .data_i(w_state_next),
      .data_o(w_state_q)
  );

  //
  // Read bus FIFO
  //
  iob_fifo_sync #(
      .W_DATA_W(DATA_WIDTH),
      .R_DATA_W(DATA_WIDTH),
      .ADDR_W  (BurstRLEN)
  ) read_bus_iob_fifo_sync (
      .clk_i(clk_i),
      .rst_i(rst_i),

      .w_en_i  (r_fifo_wr),
      .w_data_i(r_fifo_wdata),
      .w_full_o(r_fifo_full),

      .r_en_i   (r_fifo_rd),
      .r_data_o (r_fifo_rdata),
      .r_empty_o(r_fifo_empty),

      .ext_mem_clk_o(r_ext_mem_clk),
      .ext_mem_w_en_o(r_ext_mem_w_en),
      .ext_mem_w_addr_o(r_ext_mem_w_addr),
      .ext_mem_w_data_o(r_ext_mem_w_data),

      .ext_mem_r_en_o  (r_ext_mem_r_en),
      .ext_mem_r_addr_o(r_ext_mem_r_addr),
      .ext_mem_r_data_i(r_ext_mem_r_data),

      .level_o(r_fifo_level)
  );

  iob_ram_2p #(
      .DATA_W(DATA_WIDTH),
      .ADDR_W(BurstRLEN)
  ) read_bus_ram (
      .clk_i(r_ext_mem_clk),

      .w_en_i  (r_ext_mem_w_en),
      .w_addr_i(r_ext_mem_w_addr),
      .w_data_i(r_ext_mem_w_data),

      .r_en_i  (r_ext_mem_r_en),
      .r_addr_i(r_ext_mem_r_addr),
      .r_data_o(r_ext_mem_r_data)
  );

  iob_reg_re #(
      .DATA_W (ADDR_WIDTH),
      .RST_VAL(0)
  ) iob_reg_iob_raddr (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (iob_arvalid),
      .data_i(iob_addr_i),
      .data_o(iob_raddr_q)
  );

  iob_reg_re #(
      .DATA_W (1),
      .RST_VAL(0)
  ) iob_reg_r_state (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (1'b1),
      .data_i(r_state_next),
      .data_o(r_state_q)
  );

  iob_reg_re #(
      .DATA_W (1),
      .RST_VAL(0)
  ) iob_reg_iob_rvalid (
      .clk_i (clk_i),
      .arst_i(arst_i),
      .cke_i (cke_i),
      .rst_i (1'b0),
      .en_i  (1'b1),
      .data_i(iob_rvalid_next),
      .data_o(iob_rvalid_o)
  );

endmodule
