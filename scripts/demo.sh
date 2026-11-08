#!/usr/bin/env bash
# One-command demo for recording. Requires a built VM environment.
set -euo pipefail
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
source "$REPO_ROOT/scripts/setup_env.sh"

echo "=== versions ==="
git -C "$NOOP_HOME" log -1 --oneline
git -C "$NEMU_HOME" log -1 --oneline
sha256sum "$NOOP_HOME/build/emu" "$NEMU_HOME/build/riscv64-nemu-interpreter-so" "$ARTIFACT_HOME/boot-minimal.bin"

echo "=== hello ==="
"$REPO_ROOT/scripts/run_regression.sh" 2>&1 | tee /tmp/vdot-demo.log

echo "=== perf ==="
cd "$NOOP_HOME"
./build/emu -i "$PROJECT_HOME/software/build/perf.bin" \
  --diff "$NEMU_HOME/build/riscv64-nemu-interpreter-so" \
  -F "$ARTIFACT_HOME/boot-minimal.bin" 2>&1 | tee -a /tmp/vdot-demo.log
./build/emu -i "$PROJECT_HOME/software/build/perf-reverse.bin" \
  --diff "$NEMU_HOME/build/riscv64-nemu-interpreter-so" \
  -F "$ARTIFACT_HOME/boot-minimal.bin" 2>&1 | tee -a /tmp/vdot-demo.log

echo "=== done; log at /tmp/vdot-demo.log ==="
