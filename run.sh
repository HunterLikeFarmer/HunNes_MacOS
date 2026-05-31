#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

JOBS="${JOBS:-$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)}"
make -j"$JOBS"
exec ./build/HunNes
