#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/_lib.sh"

receiver="${ROBOT_IPC_EXAMPLES_DIR}/host_function_latency_test/receiver"
caller="${ROBOT_IPC_EXAMPLES_DIR}/host_function_latency_test/caller"

if [[ ! -x "${receiver}" || ! -x "${caller}" ]]; then
  skip_test "host_function latency examples are not built"
fi

print_test "host_function latency"
cleanup_ipc_artifacts

set +e
timeout 12s "${receiver}" >"${ROBOT_IPC_LOG_DIR}/hf_latency_receiver.log" 2>&1 &
receiver_pid=$!
sleep 0.3

timeout 12s "${caller}" >"${ROBOT_IPC_LOG_DIR}/hf_latency_caller.log" 2>&1
caller_rc=$?

wait "${receiver_pid}"
receiver_rc=$?
set -e

if ! assert_timeout_or_zero "${caller_rc}"; then
  echo "latency caller exited with ${caller_rc}"
  cat "${ROBOT_IPC_LOG_DIR}/hf_latency_caller.log"
  exit 1
fi

if ! assert_timeout_or_zero "${receiver_rc}"; then
  echo "latency receiver exited with ${receiver_rc}"
  cat "${ROBOT_IPC_LOG_DIR}/hf_latency_receiver.log"
  exit 1
fi

assert_log_has "latency =" "${ROBOT_IPC_LOG_DIR}/hf_latency_receiver.log"
