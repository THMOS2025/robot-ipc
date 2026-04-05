#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
export ROBOT_IPC_BUILD_DIR="${BUILD_DIR}"
export ROBOT_IPC_EXAMPLES_DIR="${BUILD_DIR}/examples"
export ROBOT_IPC_LOG_DIR="/tmp/robot_ipc_ci_logs"
export LD_LIBRARY_PATH="${PWD}/${BUILD_DIR}:${LD_LIBRARY_PATH:-}"

mkdir -p "${ROBOT_IPC_LOG_DIR}"

tests=(
  "scripts/ci/tests/host_variable_basic.sh"
  "scripts/ci/tests/host_variable_cross_language.sh"
  "scripts/ci/tests/host_variable_integrity.sh"
  "scripts/ci/tests/host_variable_latency.sh"
  "scripts/ci/tests/host_function_basic.sh"
  "scripts/ci/tests/host_function_integrity.sh"
  "scripts/ci/tests/host_function_latency.sh"
)

echo "Running robot_ipc example smoke tests"
for test_script in "${tests[@]}"; do
  "${test_script}"
done
echo "All smoke tests passed"
