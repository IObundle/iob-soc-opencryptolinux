`timescale 1ns / 1ps

`include "bsp.vh"
`include "iob_utils.vh"
`include "iob_soc_opencryptolinux_conf.vh"
`include "iob_uart_conf.vh"
`include "iob_uart_swreg_def.vh"

//Peripherals _swreg_def.vh file includes.
`include "iob_soc_opencryptolinux_periphs_swreg_def.vs"


module iob_soc_opencryptolinux_tb;

   parameter realtime clk_per = 1s / `FREQ;

   localparam ADDR_W = `IOB_SOC_OPENCRYPTOLINUX_ADDR_W;
   localparam DATA_W = `IOB_SOC_OPENCRYPTOLINUX_DATA_W;
   localparam integer DIV = `FREQ / (16 * `BAUD);

   //clock
   reg clk = 1;
   always #(clk_per / 2) clk = ~clk;

   //reset
   reg       reset = 0;

   //received by getchar
   reg       rxread_reg;
   reg       txread_reg;
   reg [7:0] read_aux;
   reg [7:0] cpu_char;
   integer soc2cnsl_fd = 0, cnsl2soc_fd = 0;


   //IOb-SoC uart
   reg                                        iob_avalid_i;
   reg  [         `IOB_UART_SWREG_ADDR_W-1:0] iob_addr_i;
   reg  [`IOB_SOC_OPENCRYPTOLINUX_DATA_W-1:0] iob_wdata_i;
   reg  [                                3:0] iob_wstrb_i;
   wire [`IOB_SOC_OPENCRYPTOLINUX_DATA_W-1:0] iob_rdata_o;
   wire                                       iob_ready_o;
   wire                                       iob_rvalid_o;

   //iterator
   integer i = 0, n = 0;
   integer error, n_byte = 0;

   //got enquiry (connect request)
   reg  gotENQ;

   //cpu trap signal
   wire trap;

   initial begin
      //init cpu bus signals
      iob_avalid_i = 0;
      iob_wstrb_i  = 0;

      //reset system
      `IOB_PULSE(reset, 100, 1_000, 100);
      @(posedge clk) #1;

      // configure uart
      cpu_inituart();


      gotENQ      = 0;
      cpu_char    = 0;
      rxread_reg  = 0;
      txread_reg  = 0;


      soc2cnsl_fd = $fopen("soc2cnsl", "r+");
      while (!soc2cnsl_fd) begin
         $display("Could not open \"soc2cnsl\"");
         soc2cnsl_fd = $fopen("soc2cnsl", "r+");
      end
      $fclose(soc2cnsl_fd);

      while (1) begin
         while (!rxread_reg && !txread_reg) begin
            iob_read(5, read_aux, 1);
            rxread_reg = |(read_aux | 8'h01);
            txread_reg = |(read_aux | 8'h20);
         end
         if (rxread_reg) begin
            soc2cnsl_fd = $fopen("soc2cnsl", "r");
            n           = $fgets(cpu_char, soc2cnsl_fd);
            if (n == 0) begin
               $fclose(soc2cnsl_fd);
               iob_read(0, cpu_char, 1);
               soc2cnsl_fd = $fopen("soc2cnsl", "w");
               $fwriteh(soc2cnsl_fd, "%c", cpu_char);
               rxread_reg = 0;
            end
            $fclose(soc2cnsl_fd);
         end
         if (txread_reg) begin
            cnsl2soc_fd = $fopen("cnsl2soc", "r");
            if (!cnsl2soc_fd) begin
               $finish();
            end
            n = $fscanf(cnsl2soc_fd, "%c", cpu_char);
            if (n > 0) begin
               iob_write(0, cpu_char, 1);
               $fclose(cnsl2soc_fd);
               cnsl2soc_fd = $fopen("./cnsl2soc", "w");
            end
            $fclose(cnsl2soc_fd);
            txread_reg = 0;
         end
      end
   end

   iob_soc_opencryptolinux_sim_wrapper iob_soc_opencryptolinux_sim_wrapper (
      .clk_i (clk),
      .rst_i (reset),
      .trap_o(trap),

      .uart_avalid(iob_avalid_i),
      .uart_addr  (iob_addr_i),
      .uart_wdata (iob_wdata_i),
      .uart_wstrb (iob_wstrb_i),
      .uart_rdata (iob_rdata_o),
      .uart_ready (iob_ready_o),
      .uart_rvalid(iob_rvalid_o)
   );

   task automatic cpu_inituart;
      begin
         reg [7:0] lcr = 0;
         iob_read(3, lcr, 1);
         lcr = (lcr | 128);
         iob_write(3, lcr, 1);
         // Set the Divisor Latches, MSB first, LSB next.
         iob_write(1, DIV[15:8], 1);
         iob_write(0, DIV[7:0], 1);
         // Set bit 7 of LCR to ‘0’ to disable access to Divisor Latches.
         // At this time the transmission engine starts working and data can be sent and received.
         lcr = (lcr & 127);
         iob_write(3, lcr, 1);
         // Set the FIFO trigger level. Generally, higher trigger level values produce less
         // interrupt to the system, so setting it to 14 bytes is recommended if the system
         // responds fast enough.
         iob_write(2, 192, 1);
         // Enable desired interrupts by setting appropriate bits in the Interrupt Enable
         // register.
         iob_write(1, 3, 1);
      end
   endtask

   `include "iob_tasks.vs"

   //finish simulation on trap
   always @(posedge trap) begin
      #10 $display("Found CPU trap condition");
      $finish();
   end

endmodule
