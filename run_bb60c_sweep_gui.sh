#!/usr/bin/env bash
# Launch the polished BB60C Sweep GUI (safe path — no spaces).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLE="${ROOT}/examples/bb60c_sweep"

# Prefer a freshly built library from the workspace when present.
if [[ -d "${ROOT}/lib_runtime" ]]; then
  export LD_LIBRARY_PATH="${ROOT}/lib_runtime${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
fi
if [[ -d "${ROOT}/py_overlay" ]]; then
  export PYTHONPATH="${ROOT}/py_overlay${PYTHONPATH:+:${PYTHONPATH}}"
fi

exec python3 "${EXAMPLE}/bb60c_sweep_gui.py"
