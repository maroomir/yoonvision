#!/bin/bash

set -euo pipefail

BUILD_DIR=./build
INSTALL_DIR=./install
BUILD_TYPE=${BUILD_TYPE:-Release}
OS_NAME=${OS_NAME:-macos}
BUILD_TESTS=${BUILD_TESTS:-ON}

get_jobs() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  elif command -v sysctl >/dev/null 2>&1; then
    sysctl -n hw.ncpu
  else
    echo 4
  fi
}

cmake_build() {
  local jobs
  jobs=$(get_jobs)

  local configure_cmd
  configure_cmd="cmake -S . -B ${BUILD_DIR} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DBUILD_TESTS=${BUILD_TESTS} \
    -DPLATFORM_NAME=${OS_NAME}"

  echo ""
  echo "=========================================================="
  echo "${configure_cmd}"
  echo "=========================================================="
  eval "${configure_cmd}"

  echo "================================"
  echo "Build: cmake --build ${BUILD_DIR} --target all -- -j ${jobs}"
  echo "================================"
  cmake --build "${BUILD_DIR}" --target all -- -j "${jobs}"

  echo "================================"
  echo "Install: cmake --install ${BUILD_DIR}"
  echo "================================"
  cmake --install "${BUILD_DIR}"
}

clean_build() {
  rm -rf "${BUILD_DIR}" "${INSTALL_DIR}"
  echo "================================"
  echo "Clear build/install directory"
  echo "================================"
}

MODE="${1:-cmake}"

case "${MODE}" in
  cmake)
    cmake_build
    ;;
  clean)
    clean_build
    ;;
  help|-h|--help)
    echo "Usage: ./build.sh [cmake|clean|help]"
    echo ""
    echo "Commands:"
    echo "  cmake   Configure + Build + Install"
    echo "  clean   Remove build/ and install/ directories"
    echo "  help    Show this help message"
    echo ""
    echo "Environment variables (optional):"
    echo "  OS_NAME                Target platform name"
    echo "                         - macos (default)"
    echo "                         - linux_x86_64"
    echo "                         - window_x86_64"
    echo "  BUILD_TYPE             CMake build type (default: Release)"
    echo "  BUILD_TESTS            ON/OFF (default: ON)"
    echo ""
    echo "Examples:"
    echo "  ./build.sh"
    echo "  ./build.sh cmake"
    echo "  BUILD_TYPE=Debug ./build.sh cmake"
    echo "  OS_NAME=linux_x86_64 ./build.sh cmake"
    echo "  ./build.sh clean"
    ;;
  *)
    echo "[ERROR] Unknown command: ${MODE}"
    echo "Usage: ./build.sh [cmake|clean|help]"
    echo "Try: ./build.sh help"
    exit 1
    ;;
esac
