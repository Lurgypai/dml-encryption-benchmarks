import itertools
import os

modes = ["crypt", "def"]
# 16, 32, 64, 128 GiB
# dims0 = [16384 * 16, 16384 * 32, 16384 * 64, 16384 * 128 ]
# dims0 = [16384]
# dims1 = [16384]

dims0 = [64]
dims1 = [64]

output_dir = "configs"

# Generate combinations and write config files
for mode, dim0, dim1 in itertools.product(modes, dims0, dims1):
    filename = f"{mode}-{dim0:09d}-{dim1:09d}"
    filepath = os.path.join(output_dir, filename)

    with open(filepath, "w") as f:
        f.write(f"MODE={mode}\n")
        f.write(f"DIM0={dim0}\n")
        f.write(f"DIM1={dim1}\n")
