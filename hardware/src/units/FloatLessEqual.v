`timescale 1ns / 1ps

module FloatLessEqual #(
                        parameter DATA_W = 32,
                        parameter EXP_W = 8
                     )
   (
    // Control
    input               clk,
    input               rst,

    input               running,
    input               run,

    // Input / Output data
    input [DATA_W-1:0] in0,
    input [DATA_W-1:0] in1,

    (* versat_latency = 1 *) output reg [DATA_W-1:0] out0
   );

    reg         less_equal;

    always @* 
    begin
    less_equal = 1'b0;

    if (in0[DATA_W-1] & in1[DATA_W-1]) begin // If both negative
      less_equal = (in0[DATA_W-2:0] >= in1[DATA_W-2:0]);
    end else if (in0[DATA_W-1] == in1[DATA_W-1]) begin // If both positive
      less_equal = (in0[DATA_W-2:0] <= in1[DATA_W-2:0]);
    end else begin // One negative and the other positive
      less_equal = in0[DATA_W-1];
    end
    end

    wire               in0_nan = &in0[DATA_W-2 -: EXP_W] & |in0[DATA_W-EXP_W-2:0];
    wire               in1_nan = &in1[DATA_W-2 -: EXP_W] & |in1[DATA_W-EXP_W-2:0];

    wire               res_int = (in0_nan | in1_nan) ? 0 : less_equal;

    always @(posedge clk, posedge rst) begin
        if (rst) begin
            out0 <= 0;
        end else begin
            out0 <= {32{res_int}};
        end
    end

endmodule
