`timescale 1ns / 1ps

module FloatGreater #(
                 parameter DATA_W = 32,
                 parameter EXP_W = 8
                 )
  (
    //control
    input               clk,
    input               rst,
    
    input               running,
    input               run,

    //input / output data
    input [DATA_W-1:0] in0,
    input [DATA_W-1:0] in1,

    (* versat_latency = 1 *) output reg [DATA_W-1:0] out0
    );

   reg         greater;

   always @* 
   begin
   greater = 1'b0;

   if(in0[DATA_W-1] & in1[DATA_W-1]) begin // If both negative
      greater = (in0[DATA_W-2:0] < in1[DATA_W-2:0]);
   end else if(in0[DATA_W-1] == in1[DATA_W-1]) begin // If both positive
      greater = (in0[DATA_W-2:0] > in1[DATA_W-2:0]);
   end else begin // One negative and the other positive
         greater = ~in0[DATA_W-1];
      end
   end

   wire               in0_nan = &in0[DATA_W-2 -: EXP_W] & |in0[DATA_W-EXP_W-2:0];
   wire               in1_nan = &in1[DATA_W-2 -: EXP_W] & |in1[DATA_W-EXP_W-2:0];

   wire               res_int = (in0_nan | in1_nan)? 0: greater;

   always @(posedge clk, posedge rst) begin
      if (rst) begin
         out0 <= 0;
      end else begin
         out0 <= {32{res_int}};
      end
   end

endmodule

