#!/usr/bin/env python3
"""
MoYu from-zero build script.

Builds the MoYu interpreter from scratch, platform-aware (Windows / Linux).

Steps:
  1. detect platform
  2. check toolchain (g++ / cmake / cargo / rustup target)
  3. configure cmake
  4. build
  5. smoke test

Usage:
    python3 script/build.py            # configure + build + smoke test
    python3 script/build.py --cmake-only   # only configure
    python3 script/build.py --skip-smoke   # build, skip smoke test
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR = os.path.join(ROOT, "build")
SMOKE_TEST = os.path.join(ROOT, "script", "smoke.my")

# platform-dependent configuration
# Windows: MinGW generator, Rust cross-compiles to x86_64-pc-windows-gnu
# Linux:   Unix Makefiles, Rust builds for the native gnu target
CFG = {
    "windows": {
        "cmake_generator": "MinGW Makefiles",
        "rust_target": "x86_64-pc-windows-gnu",
        "exe": "moyu.exe",
    },
    "linux": {
        "cmake_generator": "Unix Makefiles",
        "rust_target": "x86_64-unknown-linux-gnu",
        "exe": "moyu",
    },
}


def die(msg):
    print(f"[ERROR] {msg}", file=sys.stderr)
    sys.exit(1)


def run(cmd, cwd=None, check=True):
    print(f"[RUN] {' '.join(cmd)}")
    try:
        subprocess.run(cmd, cwd=cwd, check=check)
    except FileNotFoundError:
        die(f"command not found: {cmd[0]}")


def which(cmd):
    return shutil.which(cmd) is not None

def cmd_of(cmd):
    """Return the absolute path of a command (for display/execution)."""
    return shutil.which(cmd) or cmd


def detect_platform():
    sysname = platform.system().lower()
    if sysname == "windows":
        return "windows"
    if sysname == "linux":
        return "linux"
    die(f"unsupported platform: {sysname} (only Windows and Linux)")


def check_toolchain(plat):
    print("\n=== 1. toolchain check ===")
    missing = []
    for tool in ("g++", "cmake"):
        if not which(tool):
            missing.append(tool)
    if missing:
        die(f"missing tools: {', '.join(missing)}")

    # cargo / rustup
    if not which("cargo"):
        die("missing: cargo (install Rust via rustup)")
    if plat == "windows" and not which("rustup"):
        die("missing: rustup (needed to add the GNU target)")

    # verify versions
    run([cmd_of("g++"), "--version"])
    run([cmd_of("cmake"), "--version"])

    # rustup target: only required on Windows (cross-compile to GNU)
    if plat == "windows":
        target = CFG[plat]["rust_target"]
        installed = subprocess.run(
            ["rustup", "target", "list", "--installed"],
            capture_output=True, text=True,
        ).stdout
        if target not in installed:
            print(f"[INFO] adding rust target {target} ...")
            run(["rustup", "target", "add", target])
    print("[OK] toolchain ready")


def configure(plat):
    print("\n=== 2. cmake configure ===")
    gen = CFG[plat]["cmake_generator"]
    # fresh configure: drop stale build dir if generator differs
    # (kept simple: reuse existing build dir, cmake reconfigures it)
    run(["cmake", "-G", gen, "-S", ROOT, "-B", BUILD_DIR])
    print("[OK] configured")


def build(plat):
    print("\n=== 3. build ===")
    run(["cmake", "--build", BUILD_DIR])
    exe = CFG[plat]["exe"]
    path = os.path.join(BUILD_DIR, exe)
    if not os.path.exists(path):
        die(f"build produced no executable: {path}")
    print(f"[OK] built {path}")


def smoke_test(plat):
    print("\n=== 4. smoke test ===")
    exe = os.path.join(BUILD_DIR, CFG[plat]["exe"])
    # create the smoke script if it does not exist
    if not os.path.exists(SMOKE_TEST):
        with open(SMOKE_TEST, "w", encoding="utf-8") as f:
            f.write(
                'import io;\n'
                'let s : string = "hello";\n'
                'io.println(s);\n'
                'io.println(s.size());\n'
            )
    run([exe, "run", SMOKE_TEST])
    print("[OK] smoke test passed")


def main():
    parser = argparse.ArgumentParser(description="MoYu from-zero build")
    parser.add_argument("--cmake-only", action="store_true",
                        help="only configure, do not build")
    parser.add_argument("--skip-smoke", action="store_true",
                        help="build but skip the smoke test")
    args = parser.parse_args()

    plat = detect_platform()
    print(f"Platform: {plat}")
    print(f"Project root: {ROOT}")

    check_toolchain(plat)
    configure(plat)
    if args.cmake_only:
        print("\n[OK] configured only. Run: cmake --build " + BUILD_DIR)
        return

    build(plat)
    if not args.skip_smoke:
        smoke_test(plat)
    else:
        print("\n[OK] build complete (smoke test skipped)")


if __name__ == "__main__":
    main()
