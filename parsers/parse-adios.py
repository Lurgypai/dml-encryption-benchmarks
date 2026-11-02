#!/usr/bin/env python3
"""Aggregate ADIOS2 crypt/def benchmark results into one CSV.

Layout expected:
  <root>/all-ADIOS2-<date>_<time>-<N>n-<P>p/
      crypt-000000512-000001024-read.txt        (open, io, close, validate, total)
      crypt-000000512-000001024-read-enc_times/ (000-enc_times.txt, 001-enc_times.txt, ...)

Usage:
  python aggregate.py /path/to/root -o results.csv [--stat avg|min|max|all]
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
STAT_ORDER = {"avg": 0, "min": 1, "max": 2}

COLUMNS = ["ranks", "mode", "io dir", "open", "io", "close", "other",
           "validate", "total", "clean total", "enc time", "name"]
# must match the order of "vals" in the per-run records
VALUE_COLS = ["open", "io", "close", "validate", "total", "other", "clean total"]
# VALUE_COLS = COLUMNS[3:10]   # open .. clean total (per-run numeric columns)


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
    xs = list(xs)
    return sum(xs) / len(xs) if xs else None


def agg(fn, xs):
    return fn(xs) if xs else None


STAT_FUNCS = {"avg": mean, "min": min, "max": max}


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("root", type=Path, help="top-level folder containing the run directories")
    ap.add_argument("-o", "--output", type=Path, default=Path("results.csv"))
    ap.add_argument("-s", "--stat", choices=["avg", "min", "max", "all"], default="avg",
                    help="avg/min/max: one row per group; "
                         "all: three rows per group (avg, min, max) with a 'stat' column "
                         "(default: avg)")
    args = ap.parse_args()

    # key -> list of per-run records
    groups = defaultdict(list)

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
                o, io, c, v, t = read_timing(txt)
            except ValueError as e:
                print(f"warning: skipping {txt}: {e}", file=sys.stderr)
                continue

            enc_dir = run_dir / (txt.stem + "-enc_times")
            enc = read_enc_times(enc_dir) if enc_dir.is_dir() else []

            groups[key].append({
                # open, io, close, validate, total, other, clean total
                "vals": [o, io, c, v, t, t - (o + io + c + v), t - v],
                "enc": enc,
            })

    columns = COLUMNS + (["stat"] if args.stat == "all" else [])
    rows = []

    stats = list(STAT_ORDER) if args.stat == "all" else [args.stat]

    for (ranks, mode, io_dir, a, b), runs in groups.items():
        size_gib = a * b * SIZE_UNIT / GIB
        all_enc = [x for r in runs for x in r["enc"]]   # every rank, every run

        for stat in stats:
            fn = STAT_FUNCS[stat]
            row = {
                "ranks": ranks, "mode": mode, "io dir": io_dir,
                "name": f"{size_gib:g} GiB - {MODE_LABEL[mode]}",
            }
            for col, series in zip(VALUE_COLS, zip(*(r["vals"] for r in runs))):
                row[col] = fn(series)
            row["enc time"] = agg(fn, all_enc)
            if args.stat == "all":
                row["stat"] = stat
            row["_sort"] = (STAT_ORDER[stat], ranks, IO_ORDER[io_dir], size_gib,
                MODE_ORDER[mode])
            rows.append(row)

    for row in rows:
        if row["enc time"] is None:
            row["enc time"] = ""

    rows.sort(key=lambda r: r["_sort"])

    with args.output.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=columns, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)

    print(f"wrote {len(rows)} rows to {args.output} (stat={args.stat})")


if __name__ == "__main__":
    main()
