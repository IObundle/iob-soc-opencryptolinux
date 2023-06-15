`timescale 1ns / 1ps

`include "bsp.vh"
`include "iob_soc_opencryptolinux_conf.vh"
`include "iob_lib.vh"
`include "iob_uart_conf.vh"
`include "iob_uart_swreg_def.vh"

//Peripherals _swreg_def.vh file includes.
`include "iob_soc_opencryptolinux_periphs_swreg_def.vs"

`ifndef IOB_UART16550_SWREG_ADDR_W
`define IOB_UART16550_SWREG_ADDR_W 16
`endif
`ifndef IOB_SOC_OPENCRYPTOLINUX_DATA_W
`define IOB_SOC_OPENCRYPTOLINUX_DATA_W 32
`endif

module iob_soc_opencryptolinux_sim_wrapper (
   output                                       trap_o,
   //tester uart
   input                                        uart_avalid,
   input  [    `IOB_UART_SWREG_ADDR_W-1:0] uart_addr,
   input  [`IOB_SOC_OPENCRYPTOLINUX_DATA_W-1:0] uart_wdata,
   input  [                                3:0] uart_wstrb,
   output [`IOB_SOC_OPENCRYPTOLINUX_DATA_W-1:0] uart_rdata,
   output                                       uart_ready,
   output                                       uart_rvalid,
   input  [                              1-1:0] clk_i,        //V2TEX_IO System clock input.
   input  [                              1-1:0] rst_i         //V2TEX_IO System reset, asynchronous and active high.
);

   localparam AXI_ID_W = 4;
   localparam AXI_LEN_W = 8;
   localparam AXI_ADDR_W = `DDR_ADDR_W;
   localparam AXI_DATA_W = `DDR_DATA_W;

   `include "iob_soc_opencryptolinux_sim_pwires.vs"


   /////////////////////////////////////////////
   // TEST PROCEDURE
   //
   initial begin
`ifdef VCD
      $dumpfile("uut.vcd");
      $dumpvars();
`endif
   end

   //
   // INSTANTIATE COMPONENTS
   //

   //DDR AXI interface signals (2 for the two systems + 1 for memory)
`ifdef IOB_SOC_OPENCRYPTOLINUX_USE_EXTMEM
   `include "iob_bus_3_axi_wire.vs"
`endif

   //'Or' between trap signals of IOb-SoC and SUT
   wire [1:0] trap_signals;
   assign trap_o = trap_signals[0] || trap_signals[1];

   //
   // IOb-SoC-OpenCryptoLinux (may also include Unit Under Test)
   //
   iob_soc_opencryptolinux #(
      .AXI_ID_W  (AXI_ID_W),
      .AXI_LEN_W (AXI_LEN_W),
      .AXI_ADDR_W(AXI_ADDR_W),
      .AXI_DATA_W(AXI_DATA_W)
   ) iob_soc_opencryptolinux0 (
      `include "iob_soc_opencryptolinux_pportmaps.vs"
`ifdef IOB_SOC_OPENCRYPTOLINUX_USE_EXTMEM
      `include "iob_bus_0_2_axi_m_portmap.vs"
`endif
      .clk_i (clk_i),
      .arst_i(rst_i),
      .trap_o(trap_signals)
   );

`ifdef IOB_SOC_OPENCRYPTOLINUX_USE_EXTMEM
   //instantiate axi interconnect
   //This connects iob_soc+SUT to the same memory
   axi_interconnect #(
      .ID_WIDTH    (AXI_ID_W),
      .DATA_WIDTH  (AXI_DATA_W),
      .ADDR_WIDTH  (AXI_ADDR_W),
      .M_ADDR_WIDTH(AXI_ADDR_W),
      .S_COUNT     (2),
      .M_COUNT     (1)
   ) system_axi_interconnect (
      .clk(clk_i),
      .rst(rst_i),

      //`include "iob_bus_0_2_s_axi_portmap.vs"
      // Need to use manually defined connections because awlock and arlock of interconnect is only on bit for each slave
      .s_axi_awid   (axi_awid[0*AXI_ID_W+:2*AXI_ID_W]),               //Address write channel ID.
      .s_axi_awaddr (axi_awaddr[0*AXI_ADDR_W+:2*AXI_ADDR_W]),         //Address write channel address.
      .s_axi_awlen  (axi_awlen[0*AXI_LEN_W+:2*AXI_LEN_W]),            //Address write channel burst length.
      .s_axi_awsize (axi_awsize[0*3+:2*3]),                           //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .s_axi_awburst(axi_awburst[0*2+:2*2]),                          //Address write channel burst type.
      .s_axi_awlock ({axi_awlock[2], axi_awlock[0]}),                 //Address write channel lock type.
      .s_axi_awcache(axi_awcache[0*4+:2*4]),                          //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .s_axi_awprot (axi_awprot[0*3+:2*3]),                           //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .s_axi_awqos  (axi_awqos[0*4+:2*4]),                            //Address write channel quality of service.
      .s_axi_awvalid(axi_awvalid[0*1+:2*1]),                          //Address write channel valid.
      .s_axi_awready(axi_awready[0*1+:2*1]),                          //Address write channel ready.
      .s_axi_wdata  (axi_wdata[0*AXI_DATA_W+:2*AXI_DATA_W]),          //Write channel data.
      .s_axi_wstrb  (axi_wstrb[0*(AXI_DATA_W/8)+:2*(AXI_DATA_W/8)]),  //Write channel write strobe.
      .s_axi_wlast  (axi_wlast[0*1+:2*1]),                            //Write channel last word flag.
      .s_axi_wvalid (axi_wvalid[0*1+:2*1]),                           //Write channel valid.
      .s_axi_wready (axi_wready[0*1+:2*1]),                           //Write channel ready.
      .s_axi_bid    (axi_bid[0*AXI_ID_W+:2*AXI_ID_W]),                //Write response channel ID.
      .s_axi_bresp  (axi_bresp[0*2+:2*2]),                            //Write response channel response.
      .s_axi_bvalid (axi_bvalid[0*1+:2*1]),                           //Write response channel valid.
      .s_axi_bready (axi_bready[0*1+:2*1]),                           //Write response channel ready.
      .s_axi_arid   (axi_arid[0*AXI_ID_W+:2*AXI_ID_W]),               //Address read channel ID.
      .s_axi_araddr (axi_araddr[0*AXI_ADDR_W+:2*AXI_ADDR_W]),         //Address read channel address.
      .s_axi_arlen  (axi_arlen[0*AXI_LEN_W+:2*AXI_LEN_W]),            //Address read channel burst length.
      .s_axi_arsize (axi_arsize[0*3+:2*3]),                           //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .s_axi_arburst(axi_arburst[0*2+:2*2]),                          //Address read channel burst type.
      .s_axi_arlock ({axi_arlock[2], axi_arlock[0]}),                 //Address read channel lock type.
      .s_axi_arcache(axi_arcache[0*4+:2*4]),                          //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .s_axi_arprot (axi_arprot[0*3+:2*3]),                           //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .s_axi_arqos  (axi_arqos[0*4+:2*4]),                            //Address read channel quality of service.
      .s_axi_arvalid(axi_arvalid[0*1+:2*1]),                          //Address read channel valid.
      .s_axi_arready(axi_arready[0*1+:2*1]),                          //Address read channel ready.
      .s_axi_rid    (axi_rid[0*AXI_ID_W+:2*AXI_ID_W]),                //Read channel ID.
      .s_axi_rdata  (axi_rdata[0*AXI_DATA_W+:2*AXI_DATA_W]),          //Read channel data.
      .s_axi_rresp  (axi_rresp[0*2+:2*2]),                            //Read channel response.
      .s_axi_rlast  (axi_rlast[0*1+:2*1]),                            //Read channel last word.
      .s_axi_rvalid (axi_rvalid[0*1+:2*1]),                           //Read channel valid.
      .s_axi_rready (axi_rready[0*1+:2*1]),                           //Read channel ready.

      //`include "iob_bus_2_3_m_axi_portmap.vs"
      // Need to use manually defined connections because awlock and arlock of interconnect is only on bit
      .m_axi_awid   (axi_awid[2*AXI_ID_W+:1*AXI_ID_W]),               //Address write channel ID.
      .m_axi_awaddr (axi_awaddr[2*AXI_ADDR_W+:1*AXI_ADDR_W]),         //Address write channel address.
      .m_axi_awlen  (axi_awlen[2*AXI_LEN_W+:1*AXI_LEN_W]),            //Address write channel burst length.
      .m_axi_awsize (axi_awsize[2*3+:1*3]),                           //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .m_axi_awburst(axi_awburst[2*2+:1*2]),                          //Address write channel burst type.
      .m_axi_awlock (axi_awlock[2*2+:1]),                             //Address write channel lock type.
      .m_axi_awcache(axi_awcache[2*4+:1*4]),                          //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .m_axi_awprot (axi_awprot[2*3+:1*3]),                           //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .m_axi_awqos  (axi_awqos[2*4+:1*4]),                            //Address write channel quality of service.
      .m_axi_awvalid(axi_awvalid[2*1+:1*1]),                          //Address write channel valid.
      .m_axi_awready(axi_awready[2*1+:1*1]),                          //Address write channel ready.
      .m_axi_wdata  (axi_wdata[2*AXI_DATA_W+:1*AXI_DATA_W]),          //Write channel data.
      .m_axi_wstrb  (axi_wstrb[2*(AXI_DATA_W/8)+:1*(AXI_DATA_W/8)]),  //Write channel write strobe.
      .m_axi_wlast  (axi_wlast[2*1+:1*1]),                            //Write channel last word flag.
      .m_axi_wvalid (axi_wvalid[2*1+:1*1]),                           //Write channel valid.
      .m_axi_wready (axi_wready[2*1+:1*1]),                           //Write channel ready.
      .m_axi_bid    (axi_bid[2*AXI_ID_W+:1*AXI_ID_W]),                //Write response channel ID.
      .m_axi_bresp  (axi_bresp[2*2+:1*2]),                            //Write response channel response.
      .m_axi_bvalid (axi_bvalid[2*1+:1*1]),                           //Write response channel valid.
      .m_axi_bready (axi_bready[2*1+:1*1]),                           //Write response channel ready.
      .m_axi_arid   (axi_arid[2*AXI_ID_W+:1*AXI_ID_W]),               //Address read channel ID.
      .m_axi_araddr (axi_araddr[2*AXI_ADDR_W+:1*AXI_ADDR_W]),         //Address read channel address.
      .m_axi_arlen  (axi_arlen[2*AXI_LEN_W+:1*AXI_LEN_W]),            //Address read channel burst length.
      .m_axi_arsize (axi_arsize[2*3+:1*3]),                           //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .m_axi_arburst(axi_arburst[2*2+:1*2]),                          //Address read channel burst type.
      .m_axi_arlock (axi_arlock[2*2+:1]),                             //Address read channel lock type.
      .m_axi_arcache(axi_arcache[2*4+:1*4]),                          //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .m_axi_arprot (axi_arprot[2*3+:1*3]),                           //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .m_axi_arqos  (axi_arqos[2*4+:1*4]),                            //Address read channel quality of service.
      .m_axi_arvalid(axi_arvalid[2*1+:1*1]),                          //Address read channel valid.
      .m_axi_arready(axi_arready[2*1+:1*1]),                          //Address read channel ready.
      .m_axi_rid    (axi_rid[2*AXI_ID_W+:1*AXI_ID_W]),                //Read channel ID.
      .m_axi_rdata  (axi_rdata[2*AXI_DATA_W+:1*AXI_DATA_W]),          //Read channel data.
      .m_axi_rresp  (axi_rresp[2*2+:1*2]),                            //Read channel response.
      .m_axi_rlast  (axi_rlast[2*1+:1*1]),                            //Read channel last word.
      .m_axi_rvalid (axi_rvalid[2*1+:1*1]),                           //Read channel valid.
      .m_axi_rready (axi_rready[2*1+:1*1]),                           //Read channel ready.

      //optional signals
      .s_axi_awuser(2'b00),
      .s_axi_wuser (2'b00),
      .s_axi_aruser(2'b00),
      .m_axi_buser (1'b0),
      .m_axi_ruser (1'b0)
   );


   //instantiate the axi memory
   //IOb-SoC-OpenCryptoLinux and SUT access the same memory.
   axi_ram #(
`ifdef IOB_SOC_INIT_MEM
      .FILE      ("init_ddr_contents.hex"),  //This file contains firmware for both systems
      .FILE_SIZE (2 ** (AXI_ADDR_W - 2)),
`endif
      .ID_WIDTH  (AXI_ID_W),
      .DATA_WIDTH(AXI_DATA_W),
      .ADDR_WIDTH(AXI_ADDR_W)
   ) ddr_model_mem (
      `include "iob_bus_2_3_axi_s_portmap.vs"

      .clk_i(clk_i),
      .rst_i(rst_i)
   );
`endif

   //finish simulation on trap
   /* //Sut
always @(posedge trap[0]) begin
      #10 $display("Found SUT CPU trap condition");
      $finish;
   end
//IOb-SoC
always @(posedge trap[1]) begin
      #10 $display("Found iob_soc CPU trap condition");
      $finish;
   end */

   //sram monitor - use for debugging programs
   /*
    wire [`IOB_SOC_OPENCRYPTOLINUX_SRAM_ADDR_W-1:0] sram_daddr = uut.int_mem0.int_sram.d_addr;
    wire sram_dwstrb = |uut.int_mem0.int_sram.d_wstrb & uut.int_mem0.int_sram.d_valid;
    wire sram_drdstrb = !uut.int_mem0.int_sram.d_wstrb & uut.int_mem0.int_sram.d_valid;
    wire [`IOB_SOC_OPENCRYPTOLINUX_DATA_W-1:0] SRam_dwdata = uut.int_mem0.int_sram.d_wdata;


    wire sram_iwstrb = |uut.int_mem0.int_sram.i_wstrb & uut.int_mem0.int_sram.i_valid;
    wire sram_irdstrb = !uut.int_mem0.int_sram.i_wstrb & uut.int_mem0.int_sram.i_valid;
    wire [`IOB_SOC_OPENCRYPTOLINUX_SRAM_ADDR_W-1:0] sram_iaddr = uut.int_mem0.int_sram.i_addr;
    wire [`IOB_SOC_OPENCRYPTOLINUX_DATA_W-1:0] SRam_irdata = uut.int_mem0.int_sram.i_rdata;


    always @(posedge sram_dwstrb)
    if(sram_daddr == 13'h090d)  begin
    #10 $display("Found CPU memory condition at %f : %x : %x", $time, sram_daddr, sram_dwdata );
    //$finish;
      end
    */
   //Manually added testbench uart core. RS232 pins attached to the same pins
   //of the iob_soc_opencryptolinux UART0 instance to communicate with it
   // The interface of iob_soc UART0 is assumed to be the first portmapped interface (UART_*)
   wire cke_i = 1'b1;
   iob_uart uart_tb (
      .clk_i (clk_i),
      .cke_i (cke_i),
      .arst_i(rst_i),

      .iob_avalid_i(uart_avalid),
      .iob_addr_i  (uart_addr),
      .iob_wdata_i (uart_wdata),
      .iob_wstrb_i (uart_wstrb),
      .iob_rdata_o (uart_rdata),
      .iob_rvalid_o(uart_rvalid),
      .iob_ready_o (uart_ready),

      .txd(UART_rxd),
      .rxd(UART_txd),
      .rts(UART_cts),
      .cts(UART_rts)
   );

   //Ethernet
`ifdef IOB_SOC_OPENCRYPTOLINUX_USE_ETHERNET
   //ethernet clock: 4x slower than system clock
   reg [1:0] eth_cnt = 2'b0;
   reg       eth_clk;

   always @(posedge clk_i) begin
      eth_cnt <= eth_cnt + 1'b1;
      eth_clk <= eth_cnt[1];
   end

   // Ethernet Interface signals
   assign ETHERNET0_RX_CLK     = eth_clk;
   assign ETHERNET0_TX_CLK     = eth_clk;
   assign ETHERNET0_PLL_LOCKED = 1'b1;

   //add core test module in testbench
   iob_eth_tb_gen eth_tb (
      .clk  (clk_i),
      .reset(rst_i),

      // This module acts like a loopback
      .RX_CLK (ETHERNET0_TX_CLK),
      .RX_DATA(ETHERNET0_TX_DATA),
      .RX_DV  (ETHERNET0_TX_EN),

      // The wires are thus reversed
      .TX_CLK (ETHERNET0_RX_CLK),
      .TX_DATA(ETHERNET0_RX_DATA),
      .TX_EN  (ETHERNET0_RX_DV)
   );
`endif

endmodule
