`timescale 1ns / 1ps

module FloatNot #(
                    parameter DATA_W = 32
                )
(

   input                   clk,
   input                   rst,

   input                   running,
   input                   run,

   input [31:0]            in0,

   (* versat_latency = 1 *) output reg [31:0] out0
     );


always @(posedge clk,posedge rst)
begin
     if(rst) begin
          out0 <= 0;
     end else begin
          out0 <= {~in0[DATA_W-1], in0[DATA_W-2:0]}; // Negate the first bit and keep the rest unchanged
     end
end


endmodule
