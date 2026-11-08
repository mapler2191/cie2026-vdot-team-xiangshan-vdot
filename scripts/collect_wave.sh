#!/usr/bin/env bash
set -euo pipefail
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$ARTIFACT_HOME/waves"
cd "${NOOP_HOME:?set NOOP_HOME first}"

# vadd
"$REPO_ROOT/scripts/build_software.sh"
make -C "$PROJECT_HOME/software" vadd-wave
./build/emu-fst -i "$PROJECT_HOME/software/build/vadd-wave.bin" \
  --diff "$NEMU_HOME/build/riscv64-nemu-interpreter-so" \
  -F "$ARTIFACT_HOME/boot-minimal.bin" --dump-wave
cp -v ./*.fst "$ARTIFACT_HOME/waves/" 2>/dev/null || true

# vdot
make -C "$PROJECT_HOME/software" vdot-wave
./build/emu-fst -i "$PROJECT_HOME/software/build/vdot-wave.bin" \
  --diff "$NEMU_HOME/build/riscv64-nemu-interpreter-so" \
  -F "$ARTIFACT_HOME/boot-minimal.bin" --dump-wave
cp -v ./*.fst "$ARTIFACT_HOME/waves/" 2>/dev/null || true
