#!/usr/bin/env bash
set -euo pipefail
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "${NOOP_HOME:?set NOOP_HOME first}"
if [ -f "$REPO_ROOT/rtl/xiangshan/xiangshan-vdot-complete.patch" ]; then
  git apply "$REPO_ROOT/rtl/xiangshan/xiangshan-vdot-complete.patch"
fi
make emu CONFIG=KunminghuV2MinimalConfig JVM_XMX=12G EMU_THREADS=2 -j2
ls -l build/emu
sha256sum build/emu
