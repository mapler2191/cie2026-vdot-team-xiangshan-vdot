#!/usr/bin/env bash
# Source this file: source scripts/setup_env.sh
set -euo pipefail

export JAVA_HOME=${JAVA_HOME:-/opt/graalvm-jdk-21}
export PATH="$JAVA_HOME/bin:/usr/local/llvm/bin:$PATH"
export NOOP_HOME=${NOOP_HOME:-$HOME/risv/xs-env/XiangShan}
export NEMU_HOME=${NEMU_HOME:-$HOME/risv/xs-env/NEMU}
export PROJECT_HOME=${PROJECT_HOME:-$HOME/risv/project}
export ARTIFACT_HOME=${ARTIFACT_HOME:-$HOME/risv/artifacts}

echo "JAVA_HOME=$JAVA_HOME"
echo "NOOP_HOME=$NOOP_HOME"
echo "NEMU_HOME=$NEMU_HOME"
echo "PROJECT_HOME=$PROJECT_HOME"
