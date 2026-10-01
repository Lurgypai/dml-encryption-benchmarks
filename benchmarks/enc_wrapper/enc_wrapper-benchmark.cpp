#include <iostream>
#include <cstdint>
#include <vector>

#include <mpi.h>

extern "C" {
#include <enc_init.h>
#include <enc_store.h>
#include <enc_wrapper.h>
}

// what are the args
// ${MODE} ${RW} ${DIM0} ${DIM1}
// MODE: one of "crypt" or "def"
// RW: one of "write" or "read"
// DIM0, DIM1: IO dims, to be multiplied by 1024.


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    enc_init();

    if(argc != 5) {
        std::cerr << "Error: Incorrect args.\n";
        std::cerr << "Usage: enc_wrapper-benchmark <mode> <rw> <dim0> <dim1>\n";
    }

    std::string mode{argv[1]};
    std::string rw{argv[2]};

    size_t dim0 = std::stoi(argv[3]);
    size_t dim1 = std::stoi(argv[4]);

    size_t element_size = sizeof(std::int32_t);

    // there's a mistake in the math for the others (something about converting to chunks and then to elements incorrectly)
    // so just divide by 4 for now
    size_t total_size_bytes = (dim0 * 1024) * (dim1 * 1024) / 4;
    size_t total_size_elements = total_size_bytes / element_size;

    size_t grain_size_elements = 512 * 512;

    size_t grain_count = total_size_elements / grain_size_elements;


    char key_[256] = {0}, *key = key_;

    int rank, rank_count;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &rank_count);

    size_t my_io_size = total_size_bytes / rank_count;
    size_t my_offset = my_io_size * rank;

    std::vector<char> data{};
    data.resize(my_io_size);

    if(rw == "write") {
        enc_config meta_cfg = {
            aes256,
            enc_lib_gcrypt
        };

        enc_grain_meta grain_template = {
            static_cast<std::int64_t>(grain_size_elements * element_size),
            meta_cfg
        };

        enc_store store = enc_store_create("test.store", meta_cfg);
        enc_store_add_object(&store, "test_object", enc_object_layout_joined);

        for(int grain_idx = 0; grain_idx != grain_count; ++grain_idx) {
            enc_store_add_grain(&store, "test_object", grain_template, key);
        }

        enc_store_write(&store, "test_object", my_offset, my_io_size, NULL, key);
        enc_store_index_write(&store, "test_object", key);

        enc_store_close(&store, key);
    }
    else if (rw == "read") {
        enc_store store = enc_store_open("test.store", key);
        enc_store_index_read(&store, "test.store", key);

        enc_store_read(&store, "test_object", my_offset, my_io_size, NULL, key);
    }

}

// make the io verifiable by using the rank number as the value written
// add timers 
