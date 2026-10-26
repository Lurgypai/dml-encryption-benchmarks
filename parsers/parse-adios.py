#!/usr/bin/env python3
"""Aggregate ADIOS2 crypt/def benchmark results into one averaged CSV.

Layout expected:
  <root>/all-ADIOS2-<date>_<time>-<N>n-<P>p/
      crypt-000000512-000001024-read.txt        (open, io, close, validate, total)
      crypt-000000512-000001024-read-enc_times/ (000-enc_times.txt, 001-enc_times.txt, ...)

Usage:
  python aggregate.py /path/to/root -o results.csv
"""
import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

RUN_DIR_RE = re.compile(r"-(\d+)n-(\d+)p$")
RESULT_RE = re.compile(r"^(crypt|def)-(\d+)-(\d+)-(read|write)\.txt$")
SIZE_UNIT = 1024 * 256          # multiplied by both size values
GIB = 1024 ** 3
MODE_LABEL = {"crypt": "Crypt", "def": "Default"}   # used in the name column

IO_ORDER = {"write": 0, "read": 1}      # writes first
MODE_ORDER = {"def": 0, "crypt": 1}     # def first

COLUMNS = ["ranks", "mode", "io dir", "open", "io", "close",
           "validate", "total", "other", "clean_total", "enc time", "name"]


def read_timing(path: Path):
    """Parse 'open, io, close, validate, total' from the first non-empty line."""
    for line in path.read_text().splitlines():
        if line.strip():
            vals = [float(v) for v in line.split(",")]
            if len(vals) != 5:
                raise ValueError(f"expected 5 values, got {len(vals)}")
            return vals
    raise ValueError("file is empty")


def read_enc_times(folder: Path):
    """Return a list of enc/dec times, one per rank file in the folder."""
    times = []
    for f in sorted(folder.glob("*-enc_times.txt")):
        try:
            for line in f.read_text().splitlines():
                if line.strip():
                    times.append(float(line.split(",")[1]))
                    break
        except (ValueError, IndexError) as e:
            print(f"warning: skipping {f}: {e}", file=sys.stderr)
    return times


def mean(xs):
    return sum(xs) / len(xs) if xs else None


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("root", type=Path, help="top-level folder containing the run directories")
    ap.add_argument("-o", "--output", type=Path, default=Path("results.csv"))
    args = ap.parse_args()

    # key -> {"timings": [[5 floats], ...], "enc": [floats]}
    groups = defaultdict(lambda: {"timings": [], "enc": []})

    for run_dir in sorted(p for p in args.root.iterdir() if p.is_dir()):
        m = RUN_DIR_RE.search(run_dir.name)
        if not m:
            print(f"warning: skipping {run_dir.name} (no -Nn-Pp suffix)", file=sys.stderr)
            continue
        ranks = int(m.group(1)) * int(m.group(2))

        for txt in sorted(run_dir.glob("*.txt")):
            rm = RESULT_RE.match(txt.name)
            if not rm:
                continue
            mode, a, b, io_dir = rm.group(1), int(rm.group(2)), int(rm.group(3)), rm.group(4)
            key = (ranks, mode, io_dir, a, b)

            try:
                groups[key]["timings"].append(read_timing(txt))
            except ValueError as e:
                print(f"warning: skipping {txt}: {e}", file=sys.stderr)
                continue

            enc_dir = run_dir / (txt.stem + "-enc_times")
            if enc_dir.is_dir():
                groups[key]["enc"].extend(read_enc_times(enc_dir))

    rows = []
    for (ranks, mode, io_dir, a, b), data in groups.items():
        cols = list(zip(*data["timings"]))
        o, io, c, v, t = (mean(col) for col in cols)
        other = t - (o + io + c + v)
        enc = mean(data["enc"])
        size_gib = a * b * SIZE_UNIT / GIB
        rows.append({
            "_sort": (ranks, IO_ORDER[io_dir], size_gib, MODE_ORDER[mode]),
            "ranks": ranks, "mode": mode, "io dir": io_dir,
            "open": o, "io": io, "close": c, "validate": v, "total": t,
            "other": other,
            "clean_total": t - v,
            "enc time": "" if enc is None else enc,
            "name": f"{size_gib:g} GiB - {MODE_LABEL[mode]}",
        })

    rows.sort(key=lambda r: r["_sort"])

    with args.output.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)

    print(f"wrote {len(rows)} rows to {args.output}")


if __name__ == "__main__":
    main()
