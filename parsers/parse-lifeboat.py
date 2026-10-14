#!/usr/bin/env python3
"""Usage: python parse_crypt.py input.txt > output.csv"""
import sys
import csv

ALG = {"0": "AES256", "1": "TwoFish", "2": "AES128"}
MODE = {"0": "CBC", "1": "CTR"}

def main():
    with open(sys.argv[1]) as f:
        lines = [l.strip() for l in f if l.strip()]

    out = csv.writer(sys.stdout)
    out.writerow(["dir", "size", "encrypted", "alg", "mode", "time"])

    # Lines alternate: filename, then "Read: x" / "Write: x"
    for name, result in zip(lines[0::2], lines[1::2]):
        parts = name.removesuffix(".txt").split("-")

        if parts[0] == "def":
            # def-<size>-<size2>-<dir>
            enc, alg, mode = "def", "", ""
            size, _, direction = parts[1:4]
        else:
            # <alg>-<mode>-crypt-<size>-<size2>-<dir>
            a, m, enc, size, _, direction = parts
            alg, mode = ALG[a], MODE[m]

        time = result.split(":", 1)[1].strip()
        out.writerow([direction, int(size), enc, alg, mode, time])

if __name__ == "__main__":
    main()
