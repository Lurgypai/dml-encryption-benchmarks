#parses output from "cat *" in a process/rank directory (lowest level)

import csv
import re

nodes=32
processes=64

data = """
Ranks: 2048, w_kbytes: 128, h_kbytes: 8192, per rank w: 131072, per rank h: 4096
Read: 3.57011
Ranks: 2048, w_kbytes: 128, h_kbytes: 8192, per rank w: 131072, per rank h: 4096
Write: 12.8638
Ranks: 2048, w_kbytes: 256, h_kbytes: 8192, per rank w: 262144, per rank h: 4096
Read: 6.83971
Ranks: 2048, w_kbytes: 256, h_kbytes: 8192, per rank w: 262144, per rank h: 4096
Write: 23.3838
Ranks: 2048, w_kbytes: 512, h_kbytes: 8192, per rank w: 524288, per rank h: 4096
Read: 16.8946
Ranks: 2048, w_kbytes: 512, h_kbytes: 8192, per rank w: 524288, per rank h: 4096
Write: 50.6867
Ranks: 2048, w_kbytes: 1024, h_kbytes: 8192, per rank w: 1048576, per rank h: 4096
Read: 37.3307
Ranks: 2048, w_kbytes: 1024, h_kbytes: 8192, per rank w: 1048576, per rank h: 4096
Write: 97.9536
Ranks: 2048, w_kbytes: 128, h_kbytes: 8192, per rank w: 131072, per rank h: 4096
Read: 3.49524
Ranks: 2048, w_kbytes: 128, h_kbytes: 8192, per rank w: 131072, per rank h: 4096
Write: 12.0971
Ranks: 2048, w_kbytes: 256, h_kbytes: 8192, per rank w: 262144, per rank h: 4096
Read: 6.76577
Ranks: 2048, w_kbytes: 256, h_kbytes: 8192, per rank w: 262144, per rank h: 4096
Write: 24.4682
Ranks: 2048, w_kbytes: 512, h_kbytes: 8192, per rank w: 524288, per rank h: 4096
Read: 16.4492
Ranks: 2048, w_kbytes: 512, h_kbytes: 8192, per rank w: 524288, per rank h: 4096
Write: 49.745
Ranks: 2048, w_kbytes: 1024, h_kbytes: 8192, per rank w: 1048576, per rank h: 4096
Read: 36.5979
Ranks: 2048, w_kbytes: 1024, h_kbytes: 8192, per rank w: 1048576, per rank h: 4096
Write: 112.872
"""

rows = []
pattern = re.compile(r"^(Read|Write):\s*([\d.]+)")

for line in data.splitlines():
    line = line.strip()
    if not line or line.startswith("Ranks:"):
        continue
    match = pattern.match(line)
    if match:
        direction, time = match.groups()
        rows.append({
            "dir": "r" if direction == "Read" else "w",
            "time": time
        })

filename=f'{nodes}-{processes}'
with open(f'{filename}.csv', "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=["dir", "time"])
    writer.writeheader()
    writer.writerows(rows)

print(f"Wrote {len(rows)} rows to output.csv")
