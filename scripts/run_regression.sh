#!/usr/bin/env bash
set -euo pipefail
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "${NOOP_HOME:?set NOOP_HOME first}"
for t in vdot vadd exception-check hello; do
  echo "=== $t ==="
  ./build/emu -i "$PROJECT_HOME/software/build/$t.bin" \
    --diff "$NEMU_HOME/build/riscv64-nemu-interpreter-so" \
    -F "$ARTIFACT_HOME/boot-minimal.bin"
done
