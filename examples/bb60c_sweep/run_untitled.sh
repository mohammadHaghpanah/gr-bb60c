#!/usr/bin/env bash
# Compatibility wrapper for examples/bb60c_sweep.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
exec "${ROOT}/run_bb60c_sweep_gui.sh"
