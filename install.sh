#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
plugin_id=org.matteo.icontaskspacers
build_dir="$project_dir/build"
explicit_build=false
dry_run=false
while [ "$#" -gt 0 ]; do
    case "$1" in
        --build-dir)
            [ "$#" -ge 2 ] || { echo "--build-dir requires a directory" >&2; exit 2; }
            case "$2" in /*) build_dir="$2" ;; *) build_dir="$project_dir/$2" ;; esac
            explicit_build=true
            shift 2
            ;;
        --dry-run) dry_run=true; shift ;;
        *) echo "Usage: sh install.sh [--build-dir DIRECTORY] [--dry-run]" >&2; exit 2 ;;
    esac
done

plugin_source="$build_dir/bin/plasma/applets/$plugin_id.so"
if [ ! -f "$plugin_source" ] && [ "$explicit_build" = false ]; then
    plugin_source="$project_dir/prebuilt/$plugin_id.so"
fi
if [ ! -f "$plugin_source" ]; then
    echo "No built plugin found. Build with CMake or select --build-dir; see README.md." >&2
    exit 1
fi
plugin_directory=$(qtpaths6 --query QT_INSTALL_PLUGINS)
destination="${DESTDIR:-}$plugin_directory/plasma/applets/$plugin_id.so"
if [ "$dry_run" = true ]; then
    printf 'Install %s\n     to %s\n' "$plugin_source" "$destination"
    exit 0
fi
if [ "$(id -u)" != 0 ] && [ -z "${DESTDIR:-}" ]; then
    echo "Installing in Qt's plugin directory requires administrator access." >&2
    echo "Run this command with sudo, or use sudo cmake --install BUILD_DIRECTORY." >&2
    exit 1
fi
install -Dm644 -- "$plugin_source" "$destination"
echo "Installed Icons-Only Task Manager with Spacers."
echo "Select it through Show Alternatives or the panel's widget chooser."
echo "Log out and back in to load the new library if the widget was already running."
