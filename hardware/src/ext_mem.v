`timescale 1 ns / 1 ps

`include "system.vh"
`include "iob_intercon.vh"

module ext_mem
  #(
    parameter ADDR_W=`ADDR_W,
    parameter DATA_W=`DATA_W,
    parameter AXI_ID_W=0,
    parameter AXI_ADDR_W=`ADDR_W,
    parameter AXI_DATA_W=`DATA_W
    )
   (
`ifdef RUN_EXTMEM
    // Instruction bus
    input [1+`FIRM_ADDR_W-2+`WRITE_W-1:0]     i_req,
    output [`RESP_W-1:0] 		      i_resp,
`endif

    // Data bus
    input [1+1+`DCACHE_ADDR_W-2+`WRITE_W-1:0] d_req,
    output [`RESP_W-1:0] 		      d_resp,

    // AXI interface 
`include "m_axi_m_port.vh"
`include "iob_gen_if.vh"
    );

    //l2 cache interface signals
    wire [1+`DCACHE_ADDR_W+`WRITE_W-1:0]       l2cache_req;
    wire [`RESP_W-1:0]                        l2cache_resp;

    //ext_mem control signals
    wire                                      l2_wtb_empty;
    wire                                      invalidate;
    reg                                       invalidate_reg;
    wire                                      l2_valid = l2cache_req[1+`DCACHE_ADDR_W+`WRITE_W-1];
    //Necessary logic to avoid invalidating L2 while it's being accessed by a request
    always @(posedge clk, posedge rst)
      if (rst)
        invalidate_reg <= 1'b0;
      else
        if (invalidate)
          invalidate_reg <= 1'b1;
        else
          if(~l2_valid)
            invalidate_reg <= 1'b0;
          else
            invalidate_reg <= invalidate_reg;



`ifdef RUN_EXTMEM
   wire [1+`DCACHE_ADDR_W+`WRITE_W-1:0]       icache_req;
   wire [`RESP_W-1:0]                        icache_resp;
   assign i_resp = icache_resp;
   assign icache_req = {i_req[1+`FIRM_ADDR_W-2+`WRITE_W-1], {(`DCACHE_ADDR_W-`FIRM_ADDR_W){1'b0}}, i_req[`address(0, `FIRM_ADDR_W-2)], 2'b00, i_req[`WRITE_W-1:0]};
`endif

   wire [1+`DCACHE_ADDR_W+`WRITE_W-1:0]       dcache_req;
   wire [`RESP_W-1:0]                        dcache_resp;
   assign d_resp = dcache_resp;
   assign dcache_req = {d_req[2+`DCACHE_ADDR_W-2+`WRITE_W-1], d_req[`address(0,`DCACHE_ADDR_W-2)], 2'b11, d_req[`WRITE_W-1:0]};


   // Merge cache back-ends
`ifdef RUN_EXTMEM
   iob_merge
     #(
       .ADDR_W(`DCACHE_ADDR_W),
       .N_MASTERS(2)
       )
   merge_i_d_buses_into_l2
     (
      .clk(clk),
      .rst(rst),
      // masters
      .m_req  ({icache_req, dcache_req}),
      .m_resp ({icache_resp, dcache_resp}),
      // slave
      .s_req  (l2cache_req),
      .s_resp (l2cache_resp)
      );
`else
   assign l2cache_req = dcache_req;
   assign l2cache_resp = dcache_resp;
`endif


   // L2 cache instance
   iob_cache_axi #
     (
      .AXI_ID_W(AXI_ID_W),
      .FE_ADDR_W(`DCACHE_ADDR_W),
      .BE_ADDR_W(`DDR_ADDR_W),
      .BE_DATA_W(`DDR_DATA_W),
      .N_WAYS(4),        //Number of Ways
      .LINE_OFF_W(7),    //Cache Line Offset (number of lines)
      .WORD_OFF_W(3),    //Word Offset (number of words per line)
      .WTBUF_DEPTH_W(5), //FIFO's depth -- 5 minimum for BRAM implementation
      .CTRL_CACHE (0),   //Cache-Control can't be accessed
      .CTRL_CNT(0)       //Remove counters
      )
   l2cache 
     (
      // Native interface
      .valid    (l2cache_req[1+`DCACHE_ADDR_W+`WRITE_W-1]),
      .addr     (l2cache_req[`address(0, `DCACHE_ADDR_W)-2]),
      .wdata    (l2cache_req[`wdata(0)]),
      .wstrb    (l2cache_req[`wstrb(0)]),
      .rdata    (l2cache_resp[`rdata(0)]),
      .ready    (l2cache_resp[`ready(0)]),
      //Control IO
      .force_inv_in(invalidate_reg & ~l2_valid),
      .force_inv_out(),
      .wtb_empty_in(1'b1),
      .wtb_empty_out(l2_wtb_empty),
      // AXI interface
`include "m_axi_portmap.vh"
      .clk(clk),
      .reset(rst)
      );

endmodule
