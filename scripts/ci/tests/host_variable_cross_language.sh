#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/_lib.sh"

writer_c="${ROBOT_IPC_EXAMPLES_DIR}/host_variable/writer_c"
reader_c="${ROBOT_IPC_EXAMPLES_DIR}/host_variable/reader_c"
writer_cpp="${ROBOT_IPC_EXAMPLES_DIR}/host_variable/writer_cpp"
reader_cpp="${ROBOT_IPC_EXAMPLES_DIR}/host_variable/reader_cpp"

if [[ ! -x "${writer_c}" || ! -x "${reader_c}" || ! -x "${writer_cpp}" || ! -x "${reader_cpp}" ]]; then
  skip_test "host_variable cross-language examples are not built"
fi

print_test "host_variable C writer -> C++ reader"
cleanup_ipc_artifacts
"${writer_c}" >"${ROBOT_IPC_LOG_DIR}/hv_cross_writer_c.log" 2>&1

set +e
timeout 3s stdbuf -oL -eL "${reader_cpp}" >"${ROBOT_IPC_LOG_DIR}/hv_cross_reader_cpp.log" 2>&1
rc=$?
set -e

if ! assert_timeout_or_zero "${rc}"; then
  echo "cross-language reader_cpp exited with ${rc}"
  cat "${ROBOT_IPC_LOG_DIR}/hv_cross_reader_cpp.log"
  exit 1
fi
assert_log_has "data = 100" "${ROBOT_IPC_LOG_DIR}/hv_cross_reader_cpp.log"

print_test "host_variable C++ writer -> C reader"
cleanup_ipc_artifacts
"${writer_cpp}" >"${ROBOT_IPC_LOG_DIR}/hv_cross_writer_cpp.log" 2>&1

set +e
timeout 3s stdbuf -oL -eL "${reader_c}" >"${ROBOT_IPC_LOG_DIR}/hv_cross_reader_c.log" 2>&1
rc=$?
set -e

if ! assert_timeout_or_zero "${rc}"; then
  echo "cross-language reader_c exited with ${rc}"
  cat "${ROBOT_IPC_LOG_DIR}/hv_cross_reader_c.log"
  exit 1
fi
assert_log_has "data = 100" "${ROBOT_IPC_LOG_DIR}/hv_cross_reader_c.log"
