#!/usr/bin/env bash
set -euo pipefail
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$REPO_ROOT"
python3 tools/run_tests.py
cat evidence/reports/unit-test-report.json
