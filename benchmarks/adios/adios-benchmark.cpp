/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * hello-world.cpp : adios2 low-level API example to write and read a
 *                   std::string Variable with a greeting
 *
 *  Created on: Nov 14, 2019
 *      Author: William F Godoy godoywf@ornl.gov
 */
#include <unistd.h>

#include <iostream>
#include <chrono>

#include <cstdint>
#include <cstring>

#include <adios2.h>
#include <mpi.h>

using namespace std::chrono;

// 16 ints X 16 ints is 4KiB
// 512 X 512 ins is 1MiB
constexpr std::size_t CHUNK_DIM{512};
constexpr std::size_t CHUNK_DIM_BYTES{CHUNK_DIM * sizeof(int32_t)};
constexpr std::size_t CHUNK_SIZE{ CHUNK_DIM * CHUNK_DIM };

std::string FILENAME;

static int getChunkValue(size_t threadOffsetY, size_t w_units, size_t threadOffsetX, size_t chunkOffsetY, size_t w_chunks, size_t chunkOffsetX, size_t unit_id) {
    //      the global offset that this thread uses          the offset of the chunk we're writing into the thread          the offset of the value in the chunk
    return (threadOffsetY * w_units + threadOffsetX) + (chunkOffsetY * w_chunks + chunkOffsetX) * CHUNK_SIZE + unit_id;
}

void doWrite(adios2::ADIOS &adios, bool doCrypt, int rank, int w_chunks, int h_chunks, int my_w_chunks, int my_h_chunks)
{

    // prepare selection
    size_t w_units = w_chunks * CHUNK_DIM;
    size_t h_units = h_chunks * CHUNK_DIM;

    size_t my_w_units = my_w_chunks * CHUNK_DIM;
    size_t my_h_units = my_h_chunks * CHUNK_DIM;

    size_t threadOffsetX = 0;
    size_t threadOffsetY = my_h_units * rank;

    auto openStart = high_resolution_clock::now();

    adios2::IO io = adios.DeclareIO("writer");
    adios2::Variable<std::int32_t> var = io.DefineVariable<std::int32_t>(
        "Var",
        {w_units, h_units},                    // base dimensions
        {threadOffsetX, threadOffsetY},     // thread offset
        {my_w_units, my_h_units}                // thread region
    );

    if(doCrypt) {
        io.SetEngine("BPFile");
        adios2::Params params;
        params["PluginName"] = "cryptop";
        params["PluginLibrary"] = "EncryptionOperator";
        params["SecretKeyFile"] = "secret-key";
        params["debug_rank"] = std::to_string(rank);

        var.AddOperation("plugin", params);
    }


    // open
    adios2::Engine writer = io.Open(FILENAME, adios2::Mode::Write);

    duration<double> openTime = high_resolution_clock::now() - openStart;
    if(rank == 0) std::cout << openTime.count() << ", ";

    // allocate space
    std::vector<std::int32_t> data;
    data.resize(CHUNK_SIZE);


    // perform io
    writer.BeginStep();

    auto writeStart = high_resolution_clock::now();
    MPI_Barrier(MPI_COMM_WORLD);

    int ySteps = my_h_chunks;
    int xSteps = my_w_chunks;
    for(int yStep = 0; yStep != ySteps; ++yStep) {
        for(int xStep = 0; xStep != xSteps; ++xStep) {

            // set data buffer
            for(int i = 0; i != data.size(); ++i) {
                data[i] = getChunkValue(threadOffsetY, w_units, threadOffsetX, yStep, w_chunks, xStep, i);
            }

            // select io region
            adios2::Box<adios2::Dims> sel({xStep * CHUNK_DIM + threadOffsetX, yStep * CHUNK_DIM + threadOffsetY}, {CHUNK_DIM, CHUNK_DIM});
            var.SetSelection(sel);

            // perform and time io
            writer.Put(var, data.data(), adios2::Mode::Sync);
            writer.PerformDataWrite();
        }
        // writer.PerformDataWrite();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    duration<double> writeTime = high_resolution_clock::now() - writeStart;

    auto start = high_resolution_clock::now();
    writer.EndStep();
    writer.Close();
    duration<double> endCloseTime = high_resolution_clock::now() - start;

    if(rank == 0) {
        std::cout << writeTime.count() << ", ";
        std::cout << endCloseTime.count() << ", ";
    }
}

void doRead(adios2::ADIOS &adios, bool doCrypt, int rank, int my_w_chunks, int my_h_chunks)
{
    auto openStart = high_resolution_clock::now();

    // open file
    adios2::IO io = adios.DeclareIO("reader");
    if(doCrypt) {
        io.SetEngine("BPFile");
    }
    adios2::Engine reader = io.Open(FILENAME, adios2::Mode::Read);
    reader.BeginStep(); adios2::Variable<std::int32_t> var = io.InquireVariable<std::int32_t>("Var");
    if(doCrypt) {
        adios2::Params params;
        params["PluginName"] = "cryptop";
        params["PluginLibrary"] = "EncryptionOperator";
        params["SecretKeyFile"] = "secret-key";
        params["debug_rank"] = std::to_string(rank);
        var.AddOperation("plugin", params);
    }

    duration<double> openTime = high_resolution_clock::now() - openStart;
    if(rank == 0) std::cout << openTime.count() << ", ";

    auto readStart = high_resolution_clock::now();
    MPI_Barrier(MPI_COMM_WORLD);

    std::vector<std::int32_t> data;

    int ySteps = my_h_chunks;
    int xSteps = my_w_chunks;
    size_t threadOffsetX = 0;
    size_t threadOffsetY = my_h_chunks * CHUNK_DIM * rank;
    for(int yStep = 0; yStep != ySteps; ++yStep) {
        for(int xStep = 0; xStep != xSteps; ++xStep) {
            adios2::Box<adios2::Dims> sel({xStep * CHUNK_DIM + threadOffsetX, yStep * CHUNK_DIM + threadOffsetY}, {CHUNK_DIM, CHUNK_DIM});
            var.SetSelection(sel);
            reader.Get(var, data, adios2::Mode::Sync);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    duration<double> readTime = high_resolution_clock::now() - readStart;

    auto endCloseStart = high_resolution_clock::now();
    reader.EndStep();
    reader.Close();
    duration<double> endCloseTime = high_resolution_clock::now() - endCloseStart;

    if(rank == 0) std::cout << readTime.count() << ", ";
    if(rank == 0) std::cout << endCloseTime.count() << ", ";
}

void doValidate(adios2::ADIOS &adios, bool doCrypt, int rank, int w_chunks, int my_w_chunks, int my_h_chunks)
{
    // if(rank == 0) std::cout << "Performing validation..." << std::endl;

    // open file
    adios2::IO io = adios.DeclareIO("validater");
    if(doCrypt) {
        io.SetEngine("BPFile");
    }
    adios2::Engine reader = io.Open(FILENAME, adios2::Mode::Read);
    reader.BeginStep(); adios2::Variable<std::int32_t> var = io.InquireVariable<std::int32_t>("Var");
    if(doCrypt) {
        adios2::Params params;
        params["PluginName"] = "cryptop";
        params["PluginLibrary"] = "EncryptionOperator";
        params["SecretKeyFile"] = "secret-key";
        params["debug_rank"] = std::to_string(rank);
        var.AddOperation("plugin", params);
    }

    size_t w_units = w_chunks * CHUNK_DIM;

    std::vector<std::int32_t> data;

    double getTime = 0;

    int ySteps = my_h_chunks;
    int xSteps = my_w_chunks;
    size_t threadOffsetX = 0;
    size_t threadOffsetY = my_h_chunks * CHUNK_DIM * rank;
    for(int yStep = 0; yStep != ySteps; ++yStep) {
        for(int xStep = 0; xStep != xSteps; ++xStep) {
            adios2::Box<adios2::Dims> sel({xStep * CHUNK_DIM + threadOffsetX, yStep * CHUNK_DIM + threadOffsetY}, {CHUNK_DIM, CHUNK_DIM});
            var.SetSelection(sel);

            reader.Get(var, data, adios2::Mode::Sync);

            for(int i = 0; i != data.size(); ++i) {
                if(data[i] != getChunkValue(threadOffsetY, w_units, threadOffsetX, yStep, w_chunks, xStep, i)) {
                    std::cerr << "ERROR: Rank " << rank << " detected invalid data, xStep: " << xStep << ", yStep: " << yStep << ", offset: " << i << std::endl;
                    std::cerr << "\tExpected: " << getChunkValue(threadOffsetY, w_units, threadOffsetX, yStep, w_chunks, xStep, i) << std::endl;
                    std::cerr << "\tReceived: " << data[i] << std::endl;
                }
            }
        }
    }

    reader.EndStep();
    reader.Close();
    
    // if(rank == 0) std::cout << "Validation complete." << std::endl;
}
int main(int argc, char *argv[])
{
    auto totalStart = high_resolution_clock::now();

    MPI_Init(&argc, &argv);

    int rank, rank_count;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &rank_count); 

    // parse arg type
    if (argc != 5) {
        printf("ERROR: incorrect arg count\n");
        return -1;
    }

    bool doCrypt = false;
    if (strcmp(argv[1], "def") == 0) {
        doCrypt = false;
    }
    else if (strcmp(argv[1], "crypt") == 0) {
        doCrypt = true;
    }
    else {
        printf("ERROR: invalid argument\n");
        return -1;
    }

    bool modeWrite = false;
    if(strcmp(argv[2], "write") == 0) {
        modeWrite = true;
    } else if (strcmp(argv[2], "read") == 0) {
        modeWrite = false;
    }
    else {
        printf("ERROR: invalid argument\n");
        return -1;
    }

    std::uint64_t w_kbytes = atoi(argv[3]);
    std::uint64_t h_kbytes = atoi(argv[4]);

    FILENAME = "benchmark-" + std::to_string(w_kbytes) + "-" + std::to_string(h_kbytes) + "-" + argv[1] + ".bp";

    std::uint64_t w_bytes = w_kbytes * 1024;
    std::uint64_t h_bytes = h_kbytes * 1024;

    std::uint64_t w_chunks = w_bytes / CHUNK_DIM_BYTES;
    std::uint64_t h_chunks = h_bytes / CHUNK_DIM_BYTES;

    if(h_chunks % rank_count != 0) {
        if(rank == 0) std::cout << "The height value is too low (minimum for " << rank_count << " ranks is " << (CHUNK_DIM * rank_count) / 1024 << "KiB)\n";
        return 1;
    }

    std::uint64_t my_w_chunks = w_chunks;
    std::uint64_t my_h_chunks = h_chunks / rank_count;

    /*
    if(rank == 0) {
        std::cout << "Ranks: " << rank_count << ", w_kbytes: " << w_kbytes << ", h_kbytes: " << h_kbytes << ", per rank w: "
            << my_w_chunks * CHUNK_DIM_BYTES << ", per rank h: " << my_h_chunks * CHUNK_DIM_BYTES << std::endl;
    }
    */


    MPI_Barrier(MPI_COMM_WORLD);

    try
    {
        adios2::ADIOS adios(MPI_COMM_WORLD);
        
        MPI_Barrier(MPI_COMM_WORLD);

        // do and time write
        if( modeWrite ) {
            doWrite(adios, doCrypt, rank, w_chunks, h_chunks, my_w_chunks, my_h_chunks);
        } else {
            doRead(adios, doCrypt, rank, my_w_chunks, my_h_chunks);
        }

        auto validateStart = high_resolution_clock::now();
        doValidate(adios, doCrypt, rank, w_chunks, my_w_chunks, my_h_chunks);
        MPI_Barrier(MPI_COMM_WORLD);
        duration<double> validateTime = high_resolution_clock::now() - validateStart;
        if(rank == 0) std::cout << validateTime.count() << ", ";
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: ADIOS2 exception: " << e.what() << "\n";
        MPI_Abort(MPI_COMM_WORLD, -1);
    }


    MPI_Finalize();
    duration<double> totalTime = high_resolution_clock::now() - totalStart;
    if(rank == 0) std::cout << totalTime.count() << std::endl;
    return 0;
}
