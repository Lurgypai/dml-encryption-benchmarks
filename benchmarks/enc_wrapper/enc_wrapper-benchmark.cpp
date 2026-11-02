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

#include <chrono>
using namespace std::chrono;

struct Timer {
    time_point<high_resolution_clock> start;

    Timer() : start{high_resolution_clock::now()}
    { }
    void reset() {
        start = high_resolution_clock::now();
    }
    double getTime() {
        duration<double> elapsed = high_resolution_clock::now() - start;
        return elapsed.count();
    }
};

int main(int argc, char** argv) {
    Timer fullTimer;
    Timer setupTimer;
    MPI_Init(&argc, &argv);
    enc_init();

    if(argc != 5) {
        std::cerr << "Error: Incorrect args.\n";
        std::cerr << "Usage: enc_wrapper-benchmark <mode> <rw> <dim0> <dim1>\n";
        return -1;
    }

    std::string mode{argv[1]};
    std::string rw{argv[2]};

    size_t dim0 = std::stoi(argv[3]);
    size_t dim1 = std::stoi(argv[4]);

    std::string fileName = mode + '-' + std::to_string(dim0) + '-' + std::to_string(dim1) + ".store";

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

    if(grain_count % rank_count != 0) {
        if(rank == 0) std::cout << "Error: Grain count not divisible by rank count." << std::endl;
        return -1;
    }

    size_t my_grain_count = grain_count / rank_count;

    std::vector<std::int32_t> data{};
    data.resize(grain_size_elements);

    double setupTime = setupTimer.getTime();
    double createMetaTime, ioTime, objMetaTime, storeMetaTime;

    enc_config meta_cfg = {
        aes256,
        enc_lib_dummy
    };
    if(mode == "crypt") {
        meta_cfg.lib = enc_lib_gcrypt;
    }

    enc_grain_meta grain_template = {
        static_cast<std::int64_t>(grain_size_elements * element_size),
        meta_cfg
    };

    if(rw == "write") {
        MPI_Barrier(MPI_COMM_WORLD);

        Timer createMetaTimer;
        enc_store store = enc_store_create(fileName.c_str(), meta_cfg);
        enc_store_add_object(&store, "test_object", enc_object_layout_joined);
        for(int grain_idx = 0; grain_idx != grain_count; ++grain_idx) {
            enc_store_add_grain(&store, "test_object", grain_template, key);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        createMetaTime = createMetaTimer.getTime();
        

        Timer ioTimer;
        for(int i = 0; i != my_grain_count; ++i) {
            enc_store_write(&store, "test_object", my_offset, grain_size_elements * element_size, data.data(), key);
            my_offset += grain_size_elements * element_size;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        ioTime = ioTimer.getTime();

        Timer objMetaTimer;
        enc_store_index_write(&store, "test_object", key);
        MPI_Barrier(MPI_COMM_WORLD);
        objMetaTime = objMetaTimer.getTime();

        Timer storeMetaTimer;
        enc_store_close(&store, key);
        MPI_Barrier(MPI_COMM_WORLD);
        storeMetaTime = storeMetaTimer.getTime();
    }
    else if (rw == "read") {
        MPI_Barrier(MPI_COMM_WORLD);
        Timer createMetaTimer;
        enc_store store = enc_store_open(fileName.c_str(), key);
        MPI_Barrier(MPI_COMM_WORLD);
        createMetaTime = createMetaTimer.getTime();

        Timer objMetaTimer;
        enc_store_index_read(&store, "test_object", key);
        MPI_Barrier(MPI_COMM_WORLD);
        objMetaTime = objMetaTimer.getTime();

        Timer ioTimer;
        for(int i = 0; i != my_grain_count; ++i) {
            enc_store_read(&store, "test_object", my_offset, grain_size_elements * element_size, data.data(), key);
            my_offset += grain_size_elements * element_size;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        ioTime = ioTimer.getTime();

        Timer storeMetaTimer;
        enc_store_close(&store, key);
        MPI_Barrier(MPI_COMM_WORLD);
        storeMetaTime = storeMetaTimer.getTime();
    }

    MPI_Finalize();

    double fullTime = fullTimer.getTime();


    if(rank == 0) {
        std::cout << setupTime << ", " << createMetaTime << ", " << ioTime << ", " << objMetaTime << ", " << storeMetaTime << ", " << fullTime << std::endl;
    }
}

// make the io verifiable by using the rank number as the value written
// add timers 
