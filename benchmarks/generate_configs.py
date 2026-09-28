import itertools
import os

modes = ["crypt", "def"]
io_directions = ["write"]
# 16, 32, 64, 128 GiB
# dims0 = [16384 * 4, 16384 * 8, 16384 * 16, 16384 * 32 ]
# dims1 = [16384 * 4]

# numbers are dimensions, and they're multiplied times a 1024x1024 square.

# adios2 (256g -> 2T)
# dims0 = [128, 256, 512, 1024]
# minimum number of bytes for each rank to have a row with 4096 ranks
# dims1 = [8192]

# lifeboat
dims0 = [64, 128, 256]
dims1 = [1024]

# debug small, 8 GiB
# dims0 = [32]
# dims1 = [1024]

# debug kinda less small (128 GiB)
# dims0 = [512]
# dims1 = [1024]

output_dir = "configs"

# Generate combinations and write config files
for mode, dim0, dim1 in itertools.product(modes, dims0, dims1):
    for rw in io_directions:
        filename = f"{mode}-{dim0:09d}-{dim1:09d}-{rw}"
        filepath = os.path.join(output_dir + f"/{rw}", filename)

        with open(filepath, "w") as f:
            f.write(f"MODE={mode}\n")
            f.write(f"DIM0={dim0}\n")
            f.write(f"DIM1={dim1}\n")
            f.write(f"RW={rw}")
