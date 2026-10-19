#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [[ $# -gt 1 || ( $# -eq 1 && $1 != --dry-run ) ]]; then
    echo "Usage: bash scripts/install-build-deps.sh [--dry-run]" >&2
    exit 2
fi
command -v rpmspec >/dev/null || { echo "Install rpm-build first." >&2; exit 1; }
dependency_text=$(rpmspec --query --buildrequires "$project_dir/packaging/plasma6-applet-icontaskspacers.spec")
mapfile -t dependencies < <(printf '%s\n' "$dependency_text" | tr -d '[:blank:]' | sort -u)
if [[ ${1:-} == --dry-run ]]; then
    printf '%q ' zypper --non-interactive install --no-recommends "${dependencies[@]}"
    printf '\n'
    exit 0
fi
if [[ $(id -u) != 0 ]]; then
    echo "Run with sudo to install build dependencies, or use --dry-run." >&2
    exit 1
fi
exec zypper --non-interactive install --no-recommends "${dependencies[@]}"
