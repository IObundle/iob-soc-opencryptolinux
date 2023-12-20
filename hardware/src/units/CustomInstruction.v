`timescale 1ns / 1ps

module CustomInstruction #(
         parameter DELAY_W = 32,
         parameter DATA_W = 32
              )
    (
    //control
    input               clk,
    input               rst,
    
    input               running,
    input               run,

    //input
    input [DATA_W-1:0]  in0,
    input [DATA_W-1:0]  in1,
    input [DATA_W-1:0]  in2,

    (* versat_latency = 0 *) output [DATA_W-1:0] out0 // Need to tell versat how many cycles the unit takes to produce data
);

assign out0 = in0 + in1 + in2;

endmodule
