`ifdef verilator
module top (
`else
module design (
`endif
        input clk,
        input rst,
        input [3:0] in1,
        input [3:0] in2,
        input ena,
        output [3:0] out,
        output rdy);
    
    gcd gcd(.clk, .rst, .in1, .in2, .ena, .out, .rdy);

    initial begin
        //$display("hello from ", `__FILE__);
        //$finish;
    end
endmodule
