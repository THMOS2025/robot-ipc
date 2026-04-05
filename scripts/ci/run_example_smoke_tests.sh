#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
EXAMPLES_DIR="${BUILD_DIR}/examples"

export LD_LIBRARY_PATH="${PWD}/${BUILD_DIR}:${LD_LIBRARY_PATH:-}"

cleanup_ipc_artifacts() {
  rm -f /dev/shm/host_variable /dev/shm/host_variable_struct || true
  rm -f /tmp/robot_ipc/host_function_req /tmp/robot_ipc/host_function_res || true
}

run_host_variable_c() {
  local writer="${EXAMPLES_DIR}/host_variable/writer_c"
  local reader="${EXAMPLES_DIR}/host_variable/reader_c"

  if [[ ! -x "${writer}" || ! -x "${reader}" ]]; then
    echo "[skip] C host_variable examples are not built"
    return 0
  fi

  echo "[test] host_variable C"
  cleanup_ipc_artifacts
  "${writer}" >/tmp/robot_ipc_hv_writer_c.log 2>&1

  set +e
  timeout 3s stdbuf -oL -eL "${reader}" >/tmp/robot_ipc_hv_reader_c.log 2>&1
  local rc=$?
  set -e

  if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
    echo "host_variable reader_c exited with ${rc}"
    cat /tmp/robot_ipc_hv_reader_c.log
    return 1
  fi

  if ! grep -q "data = 100" /tmp/robot_ipc_hv_reader_c.log; then
    echo "host_variable reader_c did not observe expected data"
    cat /tmp/robot_ipc_hv_reader_c.log
    return 1
  fi
}

run_host_variable_cpp() {
  local writer="${EXAMPLES_DIR}/host_variable/writer_cpp"
  local reader="${EXAMPLES_DIR}/host_variable/reader_cpp"

  if [[ ! -x "${writer}" || ! -x "${reader}" ]]; then
    echo "[skip] C++ host_variable examples are not built"
    return 0
  fi

  echo "[test] host_variable C++"
  cleanup_ipc_artifacts
  "${writer}" >/tmp/robot_ipc_hv_writer_cpp.log 2>&1

  set +e
  timeout 3s stdbuf -oL -eL "${reader}" >/tmp/robot_ipc_hv_reader_cpp.log 2>&1
  local rc=$?
  set -e

  if [[ ${rc} -ne 0 && ${rc} -ne 124 ]]; then
    echo "host_variable reader_cpp exited with ${rc}"
    cat /tmp/robot_ipc_hv_reader_cpp.log
    return 1
  fi

  if ! grep -q "data = 100" /tmp/robot_ipc_hv_reader_cpp.log; then
    echo "host_variable reader_cpp did not observe expected data"
    cat /tmp/robot_ipc_hv_reader_cpp.log
    return 1
  fi
}

run_host_variable_integrity() {
  local writer="${EXAMPLES_DIR}/host_variable_integrity_check/host_variable_integrity_test_writer"
  local reader="${EXAMPLES_DIR}/host_variable_integrity_check/host_variable_integrity_test_reader"

  if [[ ! -x "${writer}" || ! -x "${reader}" ]]; then
    echo "[skip] host_variable integrity examples are not built"
    return 0
  fi

  echo "[test] host_variable data integrity"
  cleanup_ipc_artifacts
  rm -f /dev/shm/test || true

  timeout 8s "${writer}" >/tmp/robot_ipc_integrity_writer.log 2>&1 &
  local writer_pid=$!
  sleep 0.3

  set +e
  timeout 5s stdbuf -oL -eL "${reader}" >/tmp/robot_ipc_integrity_reader.log 2>&1
  local reader_rc=$?
  set -e

  wait "${writer_pid}" || true

  if [[ ${reader_rc} -ne 0 && ${reader_rc} -ne 124 ]]; then
    echo "integrity reader exited with ${reader_rc}"
    cat /tmp/robot_ipc_integrity_reader.log
    return 1
  fi

  if grep -q "chksum failed" /tmp/robot_ipc_integrity_reader.log; then
    echo "integrity check reported checksum failure"
    cat /tmp/robot_ipc_integrity_reader.log
    return 1
  fi
}

run_host_variable_latency() {
  local writer="${EXAMPLES_DIR}/host_variable_latency_test/host_variable_latency_test_writer"
  local reader="${EXAMPLES_DIR}/host_variable_latency_test/host_variable_latency_test_reader"

  if [[ ! -x "${writer}" || ! -x "${reader}" ]]; then
    echo "[skip] host_variable latency examples are not built"
    return 0
  fi

  echo "[test] host_variable latency"
  cleanup_ipc_artifacts
  rm -f /dev/shm/latency_test || true

  timeout 8s "${writer}" >/tmp/robot_ipc_latency_writer.log 2>&1 &
  local writer_pid=$!
  sleep 0.2

  set +e
  timeout 5s stdbuf -oL -eL "${reader}" >/tmp/robot_ipc_latency_reader.log 2>&1
  local reader_rc=$?
  set -e

  wait "${writer_pid}" || true

  if [[ ${reader_rc} -ne 0 && ${reader_rc} -ne 124 ]]; then
    echo "latency reader exited with ${reader_rc}"
    cat /tmp/robot_ipc_latency_reader.log
    return 1
  fi

  if ! grep -q "ipc delay" /tmp/robot_ipc_latency_reader.log; then
    echo "latency reader did not produce latency output"
    cat /tmp/robot_ipc_latency_reader.log
    return 1
  fi
}

run_host_function_c() {
  local receiver="${EXAMPLES_DIR}/host_function/host_function_receiver_c"
  local caller="${EXAMPLES_DIR}/host_function/host_function_caller_c"

  if [[ ! -x "${receiver}" || ! -x "${caller}" ]]; then
    echo "[skip] C host_function examples are not built"
    return 0
  fi

  echo "[test] host_function C"
  cleanup_ipc_artifacts

  "${receiver}" >/tmp/robot_ipc_hf_receiver_c.log 2>&1 &
  local receiver_pid=$!
  sleep 0.2

  timeout 5s "${caller}" >/tmp/robot_ipc_hf_caller_c.log 2>&1
  wait "${receiver_pid}"

  if ! grep -q "Return:" /tmp/robot_ipc_hf_caller_c.log; then
    echo "host_function caller_c did not receive response"
    cat /tmp/robot_ipc_hf_caller_c.log
    return 1
  fi
}

run_host_function_cpp_receiver_with_c_caller() {
  local receiver="${EXAMPLES_DIR}/host_function/host_function_receiver_cpp"
  local caller="${EXAMPLES_DIR}/host_function/host_function_caller_c"

  if [[ ! -x "${receiver}" || ! -x "${caller}" ]]; then
    echo "[skip] cross-language host_function example is not available"
    return 0
  fi

  echo "[test] host_function C caller -> C++ receiver"
  cleanup_ipc_artifacts

  "${receiver}" >/tmp/robot_ipc_hf_receiver_cpp.log 2>&1 &
  local receiver_pid=$!
  sleep 0.2

  timeout 5s "${caller}" >/tmp/robot_ipc_hf_caller_cross.log 2>&1
  wait "${receiver_pid}"

  if ! grep -q "Return:" /tmp/robot_ipc_hf_caller_cross.log; then
    echo "cross-language host_function call did not receive response"
    cat /tmp/robot_ipc_hf_caller_cross.log
    return 1
  fi
}

echo "Running robot_ipc example smoke tests"
run_host_variable_c
run_host_variable_cpp
run_host_variable_integrity
run_host_variable_latency
run_host_function_c
run_host_function_cpp_receiver_with_c_caller
echo "All smoke tests passed"
