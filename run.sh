#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

JOBS="${JOBS:-$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"
make -j"$JOBS"
exec ./build/HunNes
