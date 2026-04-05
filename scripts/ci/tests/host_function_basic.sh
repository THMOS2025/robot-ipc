#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/_lib.sh"

receiver_c="${ROBOT_IPC_EXAMPLES_DIR}/host_function/host_function_receiver_c"
receiver_cpp="${ROBOT_IPC_EXAMPLES_DIR}/host_function/host_function_receiver_cpp"
caller_c="${ROBOT_IPC_EXAMPLES_DIR}/host_function/host_function_caller_c"

if [[ ! -x "${receiver_c}" || ! -x "${receiver_cpp}" || ! -x "${caller_c}" ]]; then
  skip_test "host_function basic examples are not built"
fi

print_test "host_function C caller -> C receiver"
cleanup_ipc_artifacts
"${receiver_c}" >"${ROBOT_IPC_LOG_DIR}/hf_basic_receiver_c.log" 2>&1 &
receiver_pid=$!
sleep 0.2

timeout 5s "${caller_c}" >"${ROBOT_IPC_LOG_DIR}/hf_basic_caller_c.log" 2>&1
wait "${receiver_pid}"
assert_log_has "Return:" "${ROBOT_IPC_LOG_DIR}/hf_basic_caller_c.log"

print_test "host_function C caller -> C++ receiver"
cleanup_ipc_artifacts
"${receiver_cpp}" >"${ROBOT_IPC_LOG_DIR}/hf_basic_receiver_cpp.log" 2>&1 &
receiver_pid=$!
sleep 0.2

timeout 5s "${caller_c}" >"${ROBOT_IPC_LOG_DIR}/hf_basic_caller_cross.log" 2>&1
wait "${receiver_pid}"
assert_log_has "Return:" "${ROBOT_IPC_LOG_DIR}/hf_basic_caller_cross.log"
