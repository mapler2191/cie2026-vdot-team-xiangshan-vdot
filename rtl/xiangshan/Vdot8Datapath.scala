package xiangshan.backend.fu.wrapper

import chisel3._
import chisel3.util._

// Local competition extension, not a ratified RISC-V instruction.
object VdotSpec {
  val instruction = BitPat("b0000001??????????010?????0001011")
  val fuOp = BitPat("b100000000")
  def isDot(op: UInt): Bool = op === "h100".U
}

// Controls, ROB redirects and valid tracking stay in the existing VIMacU.
class Vdot8Datapath extends Module {
  val io = IO(new Bundle {
    val vs1 = Input(UInt(128.W))
    val vs2 = Input(UInt(128.W))
    val enable1 = Input(Bool())
    val enable2 = Input(Bool())
    val vd = Output(UInt(128.W))
  })
  val products = (0 until 8).map { i =>
    RegEnable(io.vs1(8*i+7,8*i).asSInt * io.vs2(8*i+7,8*i).asSInt, io.enable1)
  }
  val pairs = products.grouped(2).map(x => x(0) +& x(1)).toSeq
  val halves = pairs.grouped(2).map(x => x(0) +& x(1)).toSeq
  val sum = halves(0) +& halves(1)
  val result = RegEnable(sum.pad(32).asUInt, io.enable2)
  io.vd := Cat(0.U(96.W), result)
}
