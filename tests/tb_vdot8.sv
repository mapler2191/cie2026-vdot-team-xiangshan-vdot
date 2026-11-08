`timescale 1ns/1ps
module tb_vdot8;
  reg clk=0; always #5 clk=~clk;
  reg rst_n=0,flush=0,in_valid=0,out_ready=0;
  reg [127:0] vs1=0,vs2=0;
  wire in_ready,out_valid; wire [127:0] vd;
  vdot8 dut(.*);
  reg [31:0] expected[0:100000];
  reg [31:0] current_expected;
  reg [127:0] hold_data;
  reg was_stalled=0;
  integer head=0,tail=0,accepted=0,checked=0,cancelled=0,cycles=0;
  integer fd,rc,phase=0,seed=20261023;
  reg [31:0] rnd=32'h12345678;
  reg eof=0;
  always @(posedge clk) begin
    cycles=cycles+1;
    if (!rst_n || flush) begin
      cancelled=cancelled+tail-head; head=tail;was_stalled=0;
    end else begin
      if (was_stalled && (!out_valid || vd !== hold_data))
        $fatal(1,"Output changed under backpressure");
      if (out_valid && out_ready) begin
        if (head==tail) $fatal(1,"Unexpected output");
        if (vd !== {96'b0,expected[head]})
          $fatal(1,"Mismatch index=%0d got=%032h expected=%08h",head,vd,expected[head]);
        head=head+1;checked=checked+1;
      end
      if (in_valid && in_ready) begin
        expected[tail]=current_expected;tail=tail+1;accepted=accepted+1;
        in_valid<=0;
      end
      was_stalled=out_valid&&!out_ready;hold_data=vd;
    end
    if (cycles>300000) $fatal(1,"Watchdog timeout");
  end
  initial begin
    $dumpfile("results/vdot8.vcd");$dumpvars(0,tb_vdot8);
    fd=$fopen("results/vectors.hex","r");
    if (!fd) $fatal(1,"Cannot open vectors");
    repeat(3) @(negedge clk);
    rst_n=1;out_ready=1;
    // Run all vectors, mixing full throughput, bubbles and backpressure.
    while (!eof || in_valid || head!=tail) begin
      @(negedge clk);
      rnd = rnd ^ (rnd << 13);rnd = rnd ^ (rnd >> 17);rnd = rnd ^ (rnd << 5);
      out_ready=(cycles<100 || rnd[1:0]!=0);
      if (!in_valid && !eof && (cycles<100 || rnd[3:2]!=0)) begin
        rc=$fscanf(fd,"%h %h %h\n",vs1,vs2,current_expected);
        if (rc==3) in_valid=1; else eof=1;
      end
    end
    $fclose(fd);
    if (checked!=69656 || accepted!=checked) $fatal(1,"Vector accounting error %0d",checked);
    // Fill both stages, then flush. No pre-flush result may survive.
    @(negedge clk);out_ready=0;vs1={16{8'h80}};vs2=vs1;current_expected=131072;in_valid=1;
    @(negedge clk);in_valid=1;
    @(negedge clk);in_valid=0;
    @(negedge clk);flush=1;
    @(negedge clk);flush=0;out_ready=1;
    repeat(4) @(negedge clk);
    if (out_valid || head!=tail) $fatal(1,"Flush failed");
    // Reset while requests are outstanding, then recover.
    out_ready=0;in_valid=1;
    @(negedge clk);in_valid=1;
    @(negedge clk);in_valid=0;rst_n=0;
    @(negedge clk);rst_n=1;out_ready=1;
    repeat(4) @(negedge clk);
    in_valid=1;
    @(negedge clk);in_valid=0;
    repeat(4) @(negedge clk);
    if (head!=tail || checked!=69657 || cancelled!=4)
      $fatal(1,"Recovery/accounting failed checked=%0d cancelled=%0d",checked,cancelled);
    $display("PASS vectors=69656 checked=%0d cancelled=%0d cycles=%0d",checked,cancelled,cycles);
    $finish;
  end
endmodule
