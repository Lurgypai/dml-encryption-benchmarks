"""
Parse benchmark output into four CSVs:
  read_encrypted.csv, write_encrypted.csv, read_def.csv, write_def.csv

Usage:
  python3 format_results.py input.txt [output_dir]
  cat input.txt | python3 format_results.py - [output_dir]
"""
import csv
import re
import sys
from pathlib import Path

INCLUDE_W_KBYTES = True  # set False to drop the w_kbytes column

NUM = r"([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)"


def grab(pattern, text):
    m = re.search(pattern, text)
    return m.group(1) if m else ""


def split_runs(text):
    # Each run starts at a line beginning with "Ranks:"
    parts = re.split(r"(?m)^(?=\s*Ranks:)", text)
    return [p for p in parts if p.strip()]


def parse(run):
    is_read = "Get Time" in run
    encrypted = "DEBUG_enc_elapsed" in run

    row = {
        "ranks": grab(r"Ranks:\s*(\d+)", run),
        "w_kbytes": grab(r"w_kbytes:\s*(\d+)", run),
        "put": grab(r"Put Time \(s\):\s*" + NUM, run),
        "close": grab(r"End/Close Time \(s\):\s*" + NUM, run),
        "get": grab(r"Get Time \(s\):\s*" + NUM, run),
        "validate": grab(r"Validation time \(s\):\s*" + NUM, run),
        "total": grab(r"Total Runtime \(s\):\s*" + NUM, run),
    }

    # Encrypted runs print an enc/dec pair before validation and another
    # after it (writes) or one before (reads). The pair that belongs to
    # validation is ignored:
    #   write: validation's pair is the first one -> use enc after validation
    #   read:  validation's pair is the second one -> use dec before validation
    before, _, after = run.partition("Performing validation...")
    if encrypted:
        if is_read:
            vals = re.findall(r"DEBUG_dec_elapsed:\s*" + NUM, before)
            row["dec"] = sum(float(v) for v in vals)
        else:
            vals = re.findall(r"DEBUG_enc_elapsed:\s*" + NUM, after)
            row["enc"] = sum(float(v) for v in vals)

    return ("read" if is_read else "write"), encrypted, row


COLUMNS = {
    ("write", False): ["ranks", "w_kbytes", "put", "close", "validate", "total"],
    ("write", True):  ["ranks", "w_kbytes", "put", "close", "validate", "total", "enc"],
    ("read", False):  ["ranks", "w_kbytes", "get", "validate", "total"],
    ("read", True):   ["ranks", "w_kbytes", "get", "validate", "total", "dec"],
}

FILENAMES = {
    ("read", True): "read_encrypted.csv",
    ("write", True): "write_encrypted.csv",
    ("read", False): "read_def.csv",
    ("write", False): "write_def.csv",
}


def main():
    src = sys.argv[1] if len(sys.argv) > 1 else "-"
    out_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(".")
    out_dir.mkdir(parents=True, exist_ok=True)

    text = sys.stdin.read() if src == "-" else Path(src).read_text()

    groups = {key: [] for key in COLUMNS}
    for run in split_runs(text):
        kind, encrypted, row = parse(run)
        groups[(kind, encrypted)].append(row)

    for key, rows in groups.items():
        cols = [c for c in COLUMNS[key] if INCLUDE_W_KBYTES or c != "w_kbytes"]
        path = out_dir / FILENAMES[key]
        with open(path, "w", newline="") as f:
            w = csv.writer(f, lineterminator="\n")
            w.writerow(cols)
            for r in rows:
                w.writerow([r[c] for c in cols])
        print(f"{path}: {len(rows)} rows")


if __name__ == "__main__":
    main()
