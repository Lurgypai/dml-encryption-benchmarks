import itertools
import os

modes = ["crypt", "def"]
# 16, 32, 64, 128 GiB
dims0 = [16384 * 4, 16384 * 8, 16384 * 16, 16384 * 32 ]
# dims0 = [16384]
dims1 = [16384 * 4]

# dims0 = [1024]
# dims1 = [1024]

output_dir = "configs"

# Generate combinations and write config files
for mode, dim0, dim1 in itertools.product(modes, dims0, dims1):
    for rw in ["read", "write"]:
        filename = f"{mode}-{dim0:09d}-{dim1:09d}-{rw}"
        filepath = os.path.join(output_dir + f"/{rw}", filename)

        with open(filepath, "w") as f:
            f.write(f"MODE={mode}\n")
            f.write(f"DIM0={dim0}\n")
            f.write(f"DIM1={dim1}\n")
            f.write(f"RW={rw}")
