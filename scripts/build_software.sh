#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../software"
make all hello vadd exception-check perf perf-reverse vdot-wave
ls -l build/*.bin
