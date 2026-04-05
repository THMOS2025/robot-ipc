#!/usr/bin/env bash

set -euo pipefail

if [[ -z "${ROBOT_IPC_EXAMPLES_DIR:-}" || -z "${ROBOT_IPC_LOG_DIR:-}" ]]; then
  echo "ROBOT_IPC_EXAMPLES_DIR and ROBOT_IPC_LOG_DIR must be set"
  exit 1
fi

print_test() {
  echo "[test] $1"
}

skip_test() {
  echo "[skip] $1"
  exit 0
}

cleanup_ipc_artifacts() {
  rm -f \
    /dev/shm/host_variable \
    /dev/shm/host_variable_struct \
    /dev/shm/test \
    /dev/shm/latency_test || true

  rm -f \
    /tmp/robot_ipc/host_function_req \
    /tmp/robot_ipc/host_function_res \
    /tmp/robot_ipc/stress_test_req \
    /tmp/robot_ipc/stress_test_res || true
}

assert_timeout_or_zero() {
  local rc="$1"
  if [[ "${rc}" -ne 0 && "${rc}" -ne 124 ]]; then
    return 1
  fi
}

assert_log_has() {
  local pattern="$1"
  local log_file="$2"
  if ! grep -q "${pattern}" "${log_file}"; then
    echo "expected pattern '${pattern}' not found in ${log_file}"
    cat "${log_file}"
    return 1
  fi
}

assert_log_not_has() {
  local pattern="$1"
  local log_file="$2"
  if grep -q "${pattern}" "${log_file}"; then
    echo "unexpected pattern '${pattern}' found in ${log_file}"
    cat "${log_file}"
    return 1
  fi
}
