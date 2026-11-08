`timescale 1ns/1ps
// CIE vdot prototype: low eight signed int8 pairs, int32 in vd[31:0].
// Two elastic stages. Reset/flush cancel all outstanding transactions.
module vdot8 (
  input wire clk, input wire rst_n, input wire flush,
  input wire in_valid, output wire in_ready,
  input wire [127:0] vs1, input wire [127:0] vs2,
  output wire out_valid, input wire out_ready,
  output wire [127:0] vd
);
  reg valid1, valid2;
  reg signed [15:0] products [0:7];
  wire signed [16:0] pairs [0:3];
  wire signed [17:0] halves [0:1];
  wire signed [18:0] total;
  reg signed [31:0] result;
  wire ready2 = !valid2 || out_ready;
  wire ready1 = !valid1 || ready2;
  assign in_ready = ready1 && rst_n && !flush;
  assign out_valid = valid2 && rst_n && !flush;
  assign vd = {96'b0, result};
  genvar g;
  generate
    for (g=0; g<4; g=g+1) begin: pair_sum
      assign pairs[g] = {products[2*g][15],products[2*g]} +
                        {products[2*g+1][15],products[2*g+1]};
    end
    for (g=0; g<2; g=g+1) begin: half_sum
      assign halves[g] = {pairs[2*g][16],pairs[2*g]} +
                         {pairs[2*g+1][16],pairs[2*g+1]};
    end
  endgenerate
  assign total = {halves[0][17],halves[0]} + {halves[1][17],halves[1]};
  integer i;
  always @(posedge clk) begin
    if (!rst_n || flush) begin
      valid1 <= 0;
      valid2 <= 0;
      result <= 0;
      for (i=0;i<8;i=i+1) products[i] <= 0;
    end else begin
      if (ready2) begin
        valid2 <= valid1;
        if (valid1) result <= {{13{total[18]}},total};
      end
      if (ready1) begin
        valid1 <= in_valid;
        if (in_valid)
          for (i=0;i<8;i=i+1)
            products[i] <= $signed(vs1[8*i +: 8]) * $signed(vs2[8*i +: 8]);
      end
    end
  end
endmodule
