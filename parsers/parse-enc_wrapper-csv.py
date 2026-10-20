#!/usr/bin/env python3
"""Convert benchmark timing CSV (no header) into a sorted CSV with headers.

Sort order: writes before reads, then size ascending, then def before crypt.

Usage:
    python convert_bench.py input.csv output.csv
    python convert_bench.py input.csv            # prints to stdout
"""
import csv
import sys

HEADERS = ["filename", "dir", "size", "mode", "setup", "create/open", "io", "object meta", "store meta flush", "total"]
DIR_ORDER = {"write": 0, "read": 1}
MODE_ORDER = {"def": 0, "crypt": 1}


def parse_row(row):
    filename, setup, createopen, io, object_meta, store_meta, total = [c.strip() for c in row[:7]]
    # e.g. crypt-000000128-000008192-read.txt
    mode, size_str, _, tail = filename.split("-", 3)
    direction = tail.rsplit(".", 1)[0]
    size = int(size_str)  # int() drops the leading zeros
    return [filename, direction, size, mode, setup, createopen, io, object_meta, store_meta, total]


def sort_key(r):
    direction, size, mode = r[1], r[2], r[3]
    return (DIR_ORDER[direction], size, MODE_ORDER[mode])


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)

    with open(sys.argv[1], newline="") as f:
        rows = [parse_row(r) for r in csv.reader(f) if r and any(c.strip() for c in r)]

    rows.sort(key=sort_key)

    out = open(sys.argv[2], "w", newline="") if len(sys.argv) > 2 else sys.stdout
    try:
        w = csv.writer(out)
        w.writerow(HEADERS)
        w.writerows(rows)
    finally:
        if out is not sys.stdout:
            out.close()


if __name__ == "__main__":
    main()
