#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/_lib.sh"

receiver_c="${ROBOT_IPC_EXAMPLES_DIR}/host_function/host_function_receiver_c"
receiver_cpp="${ROBOT_IPC_EXAMPLES_DIR}/host_function/host_function_receiver_cpp"
caller_c="${ROBOT_IPC_EXAMPLES_DIR}/host_function/host_function_caller_c"

if [[ ! -x "${receiver_c}" || ! -x "${receiver_cpp}" || ! -x "${caller_c}" ]]; then
  skip_test "host_function integrity examples are not built"
fi

run_integrity_case() {
  local receiver="$1"
  local suffix="$2"

  cleanup_ipc_artifacts

  "${receiver}" >"${ROBOT_IPC_LOG_DIR}/hf_integrity_receiver_${suffix}.log" 2>&1 &
  local receiver_pid=$!
  sleep 0.2

  timeout 5s "${caller_c}" >"${ROBOT_IPC_LOG_DIR}/hf_integrity_caller_${suffix}.log" 2>&1
  wait "${receiver_pid}"

  local args ret
  args="$(awk '/args = /{print $NF; exit}' "${ROBOT_IPC_LOG_DIR}/hf_integrity_caller_${suffix}.log")"
  ret="$(awk '/Return:/{print $2; exit}' "${ROBOT_IPC_LOG_DIR}/hf_integrity_caller_${suffix}.log")"

  if [[ ! "${args}" =~ ^-?[0-9]+$ || ! "${ret}" =~ ^-?[0-9]+$ ]]; then
    echo "can not parse args/ret from function integrity log"
    cat "${ROBOT_IPC_LOG_DIR}/hf_integrity_caller_${suffix}.log"
    exit 1
  fi

  if [[ "${ret}" -ne "$((args - 1))" ]]; then
    echo "integrity check failed: args=${args}, ret=${ret}, expected=$((args - 1))"
    cat "${ROBOT_IPC_LOG_DIR}/hf_integrity_caller_${suffix}.log"
    exit 1
  fi
}

print_test "host_function integrity with C receiver"
run_integrity_case "${receiver_c}" "c"

print_test "host_function integrity with C++ receiver"
run_integrity_case "${receiver_cpp}" "cpp"
