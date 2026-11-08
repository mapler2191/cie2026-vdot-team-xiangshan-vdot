"""Produce deterministic patches against downloaded, pinned source archives.

Writes patched copies under upstream/. Does not publish or claim elaboration.
"""
import difflib
import hashlib
import json
from pathlib import Path
import zipfile

ROOT=Path(__file__).resolve().parents[1]
CHISEL='''package xiangshan.backend.fu.wrapper

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
'''
NEMU='''
// CIE convention: eight low int8 pairs, int32 result, zero upper 96 bits.
// Read all sources before writing, so vd may alias either source (or v0).
def_EHelper(vdot) {
  require_vector(true);
  if (vtype->vsew != 0 || vtype->vlmul != 0 || vl->val != 8 ||
      vstart->val != 0 || s->vm != 1) {
    longjmp_exception(EX_II);
  }
  int32_t sum = 0;
  for (int i = 0; i < 8; i++) {
    int32_t a = vreg_b(id_src->reg, i);
    int32_t b = vreg_b(id_src2->reg, i);
    if (a >= 128) a -= 256;
    if (b >= 128) b -= 256;
    sum += a * b;
  }
  set_vreg(id_dest->reg, 0, (uint32_t)sum, 3, 0, 0);
  set_vreg(id_dest->reg, 1, 0, 3, 0, 0);
  vstart->val = 0;
  vp_set_dirty();
}

'''

def main():
    manifests={}
    for repo in ('xiangshan','nemu'):
        with zipfile.ZipFile(ROOT/'research'/f'{repo}.zip') as z:
            original={ '/'.join(Path(f.filename).parts[1:]):z.read(f).decode('utf-8')
                for f in z.infolist() if not f.is_dir() and f.filename.endswith(('.scala','.c','.h')) }
        changed={}
        def replace(path, old, new):
            s=changed.get(path,original[path])
            if s.count(old)!=1:raise ValueError(f'Anchor not unique: {path} {old[:60]}')
            changed[path]=s.replace(old,new)
        if repo=='xiangshan':
            base='src/main/scala/xiangshan/backend/'
            changed[base+'fu/wrapper/Vdot8Datapath.scala']=CHISEL
            replace(base+'decode/VecDecoder.scala',
                '    VMUL_VV      ->',
                '    xiangshan.backend.fu.wrapper.VdotSpec.instruction -> OPMVV(T, FuType.vimac, xiangshan.backend.fu.wrapper.VdotSpec.fuOp, F, T, F, UopSplitType.VEC_VVV),\n    VMUL_VV      ->')
            replace(base+'decode/isa/bitfield/RiscvInst.scala',
                'this.OPCODE5Bit === xiangshan.backend.decode.isa.bitfield.OPCODE5Bit.OP_V\n',
                'this.OPCODE5Bit === xiangshan.backend.decode.isa.bitfield.OPCODE5Bit.OP_V ||\n      xiangshan.backend.fu.wrapper.VdotSpec.instruction === this.ALL\n')
            replace(base+'decode/DecodeUnit.scala',
                '    isAmocasQIllegal\n',
                '''    isAmocasQIllegal ||
    (xiangshan.backend.fu.wrapper.VdotSpec.instruction === inst.ALL &&
      (io.enq.vtype.illegal || io.enq.vtype.vsew =/= 0.U ||
       io.enq.vtype.vlmul =/= 0.U || io.enq.vstart =/= 0.U))
''')
            path=base+'fu/wrapper/VIMacU.scala'
            replace(path,'  // params alias','''  // CIE vdot uses the same issue port, ROB tags and vector writeback.
  private val isDot = VdotSpec.isDot(fuOpType)
  private val outIsDot = VdotSpec.isDot(outCtrl.fuOpType)
  private val dot = Module(new Vdot8Datapath)
  require(cfg.destDataBits == 128 && latency == 2)
  dot.io.vs1 := vs1
  dot.io.vs2 := vs2
  dot.io.enable1 := regEnable(1) && isDot
  dot.io.enable2 := regEnable(2)

  // params alias''')
            replace(path,'mod.io.fire        := io.in.valid','mod.io.fire        := io.in.valid && !isDot')
            replace(path,'  io.out.bits.res.data := mgu.io.out.vd',
                '  io.out.bits.res.data := Mux(outIsDot, dot.io.vd, mgu.io.out.vd)')
            replace(path,'  io.out.bits.res.vxsat.get := (outVxsat & mgu.io.out.active).orR',
                '  io.out.bits.res.vxsat.get := !outIsDot && (outVxsat & mgu.io.out.active).orR')
            replace(path,'  io.out.bits.ctrl.exceptionVec.get(ExceptionNO.illegalInstr) := mgu.io.out.illegal',
                '''  val dotIllegal = outVecCtrl.vill || outVecCtrl.vsew =/= 0.U ||
    outVecCtrl.vlmul =/= 0.U || outVl =/= 8.U || outVstart =/= 0.U || !outVm
  io.out.bits.ctrl.exceptionVec.get(ExceptionNO.illegalInstr) := Mux(outIsDot, dotIllegal, mgu.io.out.illegal)''')
        else:
            replace('src/isa/riscv64/include/isa-all-instr.h','  f(vadd) f(vsub)',
                '  f(vdot) f(vadd) f(vsub)')
            replace('src/isa/riscv64/instr/decode.c',
                '  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 10101 ??", OP_V  , OP_V);',
                '  def_INSTR_IDTAB("0000001 ????? ????? 010 ????? 00010 11", OP_V, vdot);\n  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 10101 ??", OP_V  , OP_V);')
            replace('src/isa/riscv64/instr/rvv/vcompute.h','def_EHelper(vadd) {',NEMU+'def_EHelper(vadd) {')
        patch=[]
        for path,new in changed.items():
            target=ROOT/'upstream'/repo/path;target.parent.mkdir(parents=True,exist_ok=True)
            target.write_text(new,encoding='utf-8',newline='\n')
            old=original.get(path,'')
            patch.extend(difflib.unified_diff(old.splitlines(True),new.splitlines(True),
                fromfile='a/'+path if path in original else '/dev/null',tofile='b/'+path))
        out=ROOT/'patches'/f'{repo}-vdot.patch';out.parent.mkdir(exist_ok=True)
        out.write_text(''.join(patch),encoding='utf-8',newline='\n')
        manifests[repo]={p:hashlib.sha256(s.encode()).hexdigest() for p,s in changed.items()}
    (ROOT/'patches/manifest.json').write_text(json.dumps(manifests,indent=2),encoding='utf-8')
    print('Generated pinned-source integration patches; full elaboration/difftest still required.')

if __name__=='__main__':main()
