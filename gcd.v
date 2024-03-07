module gcd (
    input clk,
    input rst,
    input [3:0] in1,
    input [3:0] in2,
    input ena,
    output reg [3:0] out,
    output reg rdy);

    // States
    typedef enum {
        idle,
        read_inputs,
        check_equality,
        check_inequality,
        write_outputs,
        assign_r_in1,
        assign_r_in2
    } State;
    /*
    localparam idle = 3'b000;
    localparam read_inputs = 3'b001;
    localparam check_equality = 3'b010;
    localparam check_inequality = 3'b011;
    localparam write_outputs = 3'b100;
    localparam assign_r_in1 = 3'b101;
    localparam assign_r_in2 = 3'b110;
    */

    reg [3:0] r_in1;
    reg [3:0] r_in1_next;
    reg [3:0] r_in2;
    reg [3:0] r_in2_next;
    State state;
    State state_next;
    //reg [2:0] state;
    //reg [2:0] state_next;

    wire [3:0] in1_minus_in2 = r_in1 - r_in2;
    wire [3:0] in2_minus_in1 = r_in2 - r_in1;
    wire in1_equal_to_in2 = r_in1 == r_in2;
    wire in1_greater_than_in2 = r_in2 < r_in1;
    wire in2_greater_than_in1 = r_in1 < r_in2;

    // state register
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            // $display("async reset ", `__FILE__);
            // asynchronous reset
            state <= idle;
            r_in1 <= 0;
            r_in2 <= 0;
        end else begin
            state <= state_next;
            r_in1 <= r_in1_next;
            r_in2 <= r_in2_next;
        end
    end

    // next state logic
    always @(state or ena or in1 or in2 or in1_minus_in2 or in2_minus_in1 or in1_equal_to_in2 or in1_greater_than_in2 or in2_greater_than_in1) begin
        // default values
        r_in1_next <= r_in1;
        r_in2_next <= r_in2;
        case (state)
            idle: begin
                if (ena == 1) begin
                    state_next <= read_inputs;
                end else begin
                    state_next <= idle;
                end
            end
            read_inputs: begin
                r_in1_next <= in1;
                r_in2_next <= in2;
                state_next <= check_equality;
            end
            check_equality: begin
                if (in1_equal_to_in2) begin
                    state_next <= write_outputs;
                end else begin
                    state_next <= check_inequality;
                end
            end
            check_inequality: begin
                if (in1_greater_than_in2) begin
                    state_next <= assign_r_in1;
                end else begin
                    state_next <= assign_r_in2;
                end
            end
            write_outputs: begin
                state_next <= idle;
            end
            assign_r_in1: begin
                r_in1_next <= in1_minus_in2;
                state_next <= check_equality;
            end
            assign_r_in2: begin
                r_in2_next <= in2_minus_in1;
                state_next <= check_equality;
            end
            default: ;
        endcase
    end

    // moore output logic
    always @(state, r_in1) begin
        // default values
        out <= out;
        rdy <= 0;
        case (state)
            idle: ;
            read_inputs: ;
            check_equality: ;
            check_inequality: ;
            write_outputs: begin
                out <= r_in1;
                rdy <= 1;
            end
            assign_r_in1: ;
            assign_r_in2: ;
            default: ;
        endcase
    end

endmodule
