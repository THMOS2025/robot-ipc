#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/_lib.sh"

writer="${ROBOT_IPC_EXAMPLES_DIR}/host_variable_integrity_check/host_variable_integrity_test_writer"
reader="${ROBOT_IPC_EXAMPLES_DIR}/host_variable_integrity_check/host_variable_integrity_test_reader"

if [[ ! -x "${writer}" || ! -x "${reader}" ]]; then
  skip_test "host_variable integrity examples are not built"
fi

print_test "host_variable data integrity"
cleanup_ipc_artifacts

timeout 8s "${writer}" >"${ROBOT_IPC_LOG_DIR}/hv_integrity_writer.log" 2>&1 &
writer_pid=$!
sleep 0.3

set +e
timeout 5s "${reader}" >"${ROBOT_IPC_LOG_DIR}/hv_integrity_reader.log" 2>&1
reader_rc=$?
set -e

wait "${writer_pid}" || true

if ! assert_timeout_or_zero "${reader_rc}"; then
  echo "integrity reader exited with ${reader_rc}"
  cat "${ROBOT_IPC_LOG_DIR}/hv_integrity_reader.log"
  exit 1
fi

assert_log_not_has "chksum failed" "${ROBOT_IPC_LOG_DIR}/hv_integrity_reader.log"
