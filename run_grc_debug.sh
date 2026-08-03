#!/usr/bin/env bash
# Open GNU Radio Companion with workspace GRC blocks preferred over system copies.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ -d "${ROOT}/lib_runtime" ]]; then
  export LD_LIBRARY_PATH="${ROOT}/lib_runtime${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
fi
if [[ -d "${ROOT}/py_overlay" ]]; then
  export PYTHONPATH="${ROOT}/py_overlay${PYTHONPATH:+:${PYTHONPATH}}"
fi

# Later paths overwrite earlier ones — put workspace/grc last.
export GRC_BLOCKS_PATH="/usr/share/gnuradio/grc/blocks:/usr/local/share/gnuradio/grc/blocks:${ROOT}/grc"

python3 - <<'PY'
from gnuradio import bb60c  # noqa: F401
import os
print("bb60c import OK")
print("GRC_BLOCKS_PATH=", os.environ.get("GRC_BLOCKS_PATH"))
print("Hint: Reload Block Library, then delete and re-add BB60C Sweep.")
print("Time Sink: Trigger=Tag, Tag Key=sweep_start")
PY

exec gnuradio-companion "$@"
