"""
Flash the ESP32 harness through the Pico bridge.

Usage:
    python3 hardware/flash.py                  # auto-detect Pico port
    python3 hardware/flash.py --port /dev/cu.usbmodemXXX
    python3 hardware/flash.py --status         # show detected ports and exit
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time

import serial
import serial.tools.list_ports

# ── paths ────────────────────────────────────────────────────────────────────

HERE        = os.path.dirname(os.path.abspath(__file__))
SKETCH_DIR  = os.path.join(HERE, "esp")
FQBN        = "esp32:esp32:esp32"
BAUD        = 115200
BRIDGE_SECS = 90

# ── Pico detection ────────────────────────────────────────────────────────────

PICO_VID_PID = (0x2E8A, 0x000A)   # Raspberry Pi Pico running CDC firmware


def find_pico_port() -> str | None:
    for p in serial.tools.list_ports.comports():
        if (getattr(p, "vid", None), getattr(p, "pid", None)) == PICO_VID_PID:
            return p.device
    return None


def list_ports():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("  (no serial ports visible)")
        return
    for p in ports:
        vid = f"{p.vid:04X}" if p.vid else "----"
        pid = f"{p.pid:04X}" if p.pid else "----"
        print(f"  {p.device}  [{vid}:{pid}]  {p.description}")

# ── Pico bridge handshake ─────────────────────────────────────────────────────

def enter_bridge(port: str) -> bool:
    """Tell the Pico to enter transparent UART bridge mode for BRIDGE_SECS seconds."""
    print(f"[flash] opening {port} to send BRIDGE command ...")
    with serial.Serial(port, BAUD, timeout=0.5) as ser:
        # flush any stale bytes from a previous session
        ser.write(b"\n")
        ser.flush()
        time.sleep(0.3)
        ser.reset_input_buffer()

        ser.write(f"BRIDGE {BRIDGE_SECS}\n".encode())
        ser.flush()

        deadline = time.monotonic() + 5.0
        buf = b""
        while time.monotonic() < deadline:
            chunk = ser.read(64)
            if chunk:
                buf += chunk
                if b"ACK bridge" in buf:
                    print("[flash] Pico ACK — bridge active.")
                    return True
            else:
                time.sleep(0.05)

    print(f"[flash] no ACK from Pico (received: {buf!r})")
    return False

# ── compile + flash ───────────────────────────────────────────────────────────

def run(cmd: list[str], **kwargs) -> int:
    print(f"    $ {' '.join(cmd)}")
    result = subprocess.run(cmd, **kwargs)
    return result.returncode


def flash(port: str) -> int:
    if not shutil.which("arduino-cli"):
        print("[flash] arduino-cli not found — install it first:")
        print("  https://arduino.github.io/arduino-cli/latest/installation/")
        return 1

    build_dir = tempfile.mkdtemp(prefix="sidechain-build-")
    try:
        # 1. Compile
        print(f"[flash] compiling {SKETCH_DIR} ...")
        rc = run([
            "arduino-cli", "compile",
            "--fqbn", FQBN,
            "--output-dir", build_dir,
            SKETCH_DIR,
        ])
        if rc != 0:
            print("[flash] compile failed.")
            return rc

        # 2. Find the app .bin (skip bootloader / partitions)
        app_bin = next(
            (os.path.join(build_dir, f) for f in sorted(os.listdir(build_dir))
             if f.endswith(".bin")
             and not any(f.endswith(s) for s in (".bootloader.bin", ".partitions.bin", ".merged.bin"))),
            None,
        )
        if app_bin is None:
            print(f"[flash] no app .bin found in {build_dir}: {os.listdir(build_dir)}")
            return 1

        # 3. Enter bridge mode
        if not enter_bridge(port):
            print("[flash] aborting — Pico did not enter bridge mode.")
            return 1
        time.sleep(0.4)   # let the USB stack settle

        # 4. Flash via esptool (--before/after no-reset: chip is already in bootloader)
        print(f"[flash] flashing {os.path.basename(app_bin)} via Pico bridge ...")
        rc = run([
            sys.executable, "-m", "esptool",
            "--chip", "esp32",
            "--port", port,
            "--baud", str(BAUD),
            "--before", "no-reset",
            "--after",  "no-reset",
            "write_flash", "0x10000", app_bin,
        ])
        if rc != 0:
            print("[flash] esptool failed.")
            return rc

        print("[flash] done — waiting for ESP32 to boot ...")
        time.sleep(8)
        print("[flash] ESP32 should be running. Open a serial monitor on the Pico port to read telemetry.")
        return 0

    finally:
        import shutil as _sh
        _sh.rmtree(build_dir, ignore_errors=True)

# ── CLI ───────────────────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", default=None,
                    help="Pico serial port (auto-detected if omitted).")
    ap.add_argument("--status", action="store_true",
                    help="Print detected serial ports and exit.")
    args = ap.parse_args()

    if args.status:
        print("Serial ports:")
        list_ports()
        sys.exit(0)

    port = args.port or find_pico_port()
    if port is None:
        print("[flash] could not auto-detect the Pico. Visible ports:")
        list_ports()
        print("  Pass --port /dev/<...> to specify it manually.")
        sys.exit(1)

    print(f"[flash] using Pico port: {port}")
    sys.exit(flash(port))


if __name__ == "__main__":
    main()
