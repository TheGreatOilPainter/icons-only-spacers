#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Create reproducible source releases, or explicitly versioned binary bundles."""
import argparse
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import shutil
import tarfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIRECTORIES = ("applet", "cmake", "config", "LICENSES", "scripts", "tests", "docs", "packaging", ".github")
SOURCE_FILES = ("CMakeLists.txt", "README.md", "UPSTREAM.md", "CHANGELOG.md", "LICENSE", "CONTRIBUTING.md", "install.sh", ".gitignore")
PLUGIN = "org.matteo.icontaskspacers.so"


def source_files():
    for name in SOURCE_FILES:
        path = ROOT / name
        if not path.is_file():
            raise RuntimeError("Missing release input: " + name)
        yield path
    for name in SOURCE_DIRECTORIES:
        directory = ROOT / name
        if not directory.is_dir():
            raise RuntimeError("Missing release directory: " + name)
        for path in sorted(directory.rglob("*")):
            if "__pycache__" in path.parts or path.suffix == ".pyc":
                continue
            if path.is_symlink():
                raise RuntimeError("Symlinks are not release inputs: " + str(path))
            if path.is_file():
                yield path


def archive(destination, prefix, entries, epoch):
    # No account names, host paths, or timestamps from the local build machine.
    with destination.open("wb") as output, gzip.GzipFile(filename="", fileobj=output, mode="wb", mtime=epoch) as compressed:
        with tarfile.open(fileobj=compressed, mode="w|", format=tarfile.PAX_FORMAT) as tar:
            for relative, data, executable in sorted(entries, key=lambda entry: entry[0]):
                member = tarfile.TarInfo(prefix + "/" + relative)
                member.size = len(data)
                member.mode = 0o755 if executable else 0o644
                member.mtime = epoch
                member.uid = member.gid = 0
                member.uname = member.gname = "root"
                tar.addfile(member, io.BytesIO(data))
    digest = hashlib.sha256(destination.read_bytes()).hexdigest()
    destination.with_name(destination.name + ".sha256").write_text(digest + "  " + destination.name + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", action="store_true", help="also include a plugin built for the exact recorded Plasma/Qt versions")
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "dist")
    args = parser.parse_args()
    version = json.loads((ROOT / "applet/metadata.json").read_text())["KPlugin"]["Version"]
    cmake_version = re.search(r"project\(icontaskspacers VERSION ([0-9.]+)", (ROOT / "CMakeLists.txt").read_text())
    if not cmake_version or cmake_version.group(1) != version:
        parser.error("CMake and widget versions disagree")
    epoch = int(os.environ.get("SOURCE_DATE_EPOCH", "0"))
    if epoch < 0:
        parser.error("SOURCE_DATE_EPOCH must not be negative")
    entries = []
    for path in source_files():
        relative = path.relative_to(ROOT).as_posix()
        executable = relative == "install.sh" or relative.startswith("scripts/") and path.suffix in (".py", ".sh")
        entries.append((relative, path.read_bytes(), executable))
    prefix = "icons-only-spacers-" + version
    if args.binary:
        build_dir = args.build_dir.resolve()
        info_path = build_dir / "release-build-info.json"
        if not info_path.is_file():
            parser.error("Configure a fresh build directory first; release-build-info.json is missing")
        info = json.loads(info_path.read_text())
        if info["project_version"] != version:
            parser.error("The selected build belongs to a different project version")
        plugin = build_dir / "bin/plasma/applets" / PLUGIN
        if not plugin.is_file():
            parser.error("Build the plugin before requesting a binary bundle")
        for key in ("plasma_version", "qt_version", "system", "architecture"):
            if not re.fullmatch(r"[A-Za-z0-9_.+-]+", info[key]):
                parser.error("Invalid build identity: " + key)
        versions = subprocess.check_output(["readelf", "--version-info", str(plugin)], text=True)
        info["qt_private_api_requirements"] = sorted(set(re.findall(r"Name: (Qt_[A-Za-z0-9_.]+_PRIVATE_API)", versions)))
        prefix += "-plasma" + info["plasma_version"] + "-qt" + info["qt_version"] + "-" + info["system"].lower() + "-" + info["architecture"]
        with tempfile.TemporaryDirectory(prefix="spacer-binary-") as temporary:
            stripped = Path(temporary) / PLUGIN
            shutil.copy2(plugin, stripped)
            subprocess.run(["strip", "--strip-unneeded", str(stripped)], check=True)
            binary_data = stripped.read_bytes()
        entries.extend((("prebuilt/" + PLUGIN, binary_data, False),
                        ("prebuilt/build-info.json", (json.dumps(info, indent=2) + "\n").encode(), False)))
    args.output_dir.mkdir(parents=True, exist_ok=True)
    destination = args.output_dir / (prefix + ".tar.gz")
    archive(destination, prefix, entries, epoch)
    print(destination)


if __name__ == "__main__":
    main()
