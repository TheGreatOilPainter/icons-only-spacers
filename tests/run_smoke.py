#!/usr/bin/env python3
"""Exercise the real task model on a private X server, with no real windows."""
import os
from pathlib import Path
import selectors
import subprocess
import sys
import tempfile

env = dict(os.environ, QT_FORCE_STDERR_LOGGING="1", QT_QPA_PLATFORM="xcb",
           QT_QUICK_BACKEND="software", QT_QPA_PLATFORMTHEME="",
           DBUS_SESSION_BUS_ADDRESS="unix:path=/tmp/nonexistent-plasma-test-bus")
with tempfile.TemporaryFile(mode="w+") as display_log, subprocess.Popen(["Xvfb", "-displayfd", "1", "-screen", "0", "1920x1080x24", "-nolisten", "tcp"],
                      stdout=subprocess.PIPE, stderr=display_log, text=True) as display:
    try:
        selector = selectors.DefaultSelector()
        selector.register(display.stdout, selectors.EVENT_READ)
        if not selector.select(5):
            raise RuntimeError("Private X server did not start")
        number = display.stdout.readline().strip()
        if not number:
            raise RuntimeError("Private X server failed")
        env["DISPLAY"] = ":" + number
        binary_dir = Path(sys.argv[1]).resolve()
        arguments = sys.argv[2:]
        expect_failure = "--expect-failure" in arguments
        arguments = [argument for argument in arguments if argument != "--expect-failure"]
        result = subprocess.run([str(binary_dir / "spacer-smoke"), str(binary_dir), *arguments], env=env, timeout=50)
        if expect_failure:
            # A signal/core dump must not count as an expected assertion failure.
            sys.exit(0 if result.returncode == 1 else 1)
        sys.exit(result.returncode)
    finally:
        display.terminate()
