`timescale 1ns / 1ps

module PriorityEncoder #(
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
    input [DATA_W-1:0] in2,

    (* versat_latency = 1 *) output reg [DATA_W-1:0] out0
    );

  always @(posedge rst,posedge clk) begin
     if(rst) begin
        out0 <= 0;
     end else begin
        if(|in0) begin
           out0 <= 0;
        end else if(|in1) begin
           out0 <= 1;
        end else if(|in2) begin
           out0 <= 2;
        end else begin
           out0 <= 3;
        end
     end
  end

endmodule

