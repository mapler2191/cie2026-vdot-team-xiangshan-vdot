#!/usr/bin/env bash
set -euo pipefail
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "${NEMU_HOME:?set NEMU_HOME first}"
if [ -f "$REPO_ROOT/nemu/nemu-vdot-clean.patch" ]; then
  git apply "$REPO_ROOT/nemu/nemu-vdot-clean.patch"
fi
make riscv64-xs-ref_defconfig
make -j"$(nproc)"
ls -l build/riscv64-nemu-interpreter-so
sha256sum build/riscv64-nemu-interpreter-so
