import itertools
import os

modes = ["crypt", "def"]
dims0 = [16384, 16384 * 2, 16384 * 4, 16384 * 8, 16384 * 16] # 1, 2, 4, 8, 16 GiB
dims1 = [16384]

output_dir = "configs"

# Generate combinations and write config files
for mode, dim0, dim1 in itertools.product(modes, dims0, dims1):
    filename = f"{mode}-{dim0:06d}-{dim1:06d}"
    filepath = os.path.join(output_dir, filename)

    with open(filepath, "w") as f:
        f.write(f"MODE={mode}\n")
        f.write(f"DIM0={dim0}\n")
        f.write(f"DIM1={dim1}\n")
