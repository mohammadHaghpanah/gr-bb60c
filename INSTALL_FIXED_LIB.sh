#!/usr/bin/env bash
# Install the built bb60c library and GRC YAML into /usr/local (requires sudo).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="${ROOT}/build"

if [[ ! -d "${BUILD}" ]]; then
  echo "error: ${BUILD} not found. Configure and build first:" >&2
  echo "  mkdir -p build && cd build && cmake .. && cmake --build ." >&2
  exit 1
fi

LIB="$(find "${BUILD}/lib" -name 'libgnuradio-bb60c.so*' -type f | head -n1)"
PYSO="$(find "${BUILD}/python" -name 'bb60c_python*.so' -type f | head -n1)"

if [[ -z "${LIB}" || -z "${PYSO}" ]]; then
  echo "error: build artifacts not found under ${BUILD}" >&2
  exit 1
fi

sudo cp -v "${LIB}" /usr/local/lib/x86_64-linux-gnu/
sudo cp -v "${PYSO}" /usr/local/lib/python3.10/dist-packages/gnuradio/bb60c/
sudo cp -v "${ROOT}/grc/bb60c_bb60c_sweep.block.yml" /usr/local/share/gnuradio/grc/blocks/
if [[ -f "${ROOT}/grc/bb60c_set_time_sink_nsamps.block.yml" ]]; then
  sudo cp -v "${ROOT}/grc/bb60c_set_time_sink_nsamps.block.yml" /usr/local/share/gnuradio/grc/blocks/
fi
sudo ldconfig

echo "Installed gnuradio-bb60c."
echo "Restart GRC, Reload Block Library, delete and re-add BB60C Sweep."
