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

void doWrite(adios2::ADIOS &adios, bool doCrypt, int rank, int w_chunks, int h_chunks, int my_w_chunks, int my_h_chunks)
{

    size_t w_units = w_chunks * CHUNK_DIM;
    size_t h_units = h_chunks * CHUNK_DIM;

    size_t my_w_units = my_w_chunks * CHUNK_DIM;
    size_t my_h_units = my_h_chunks * CHUNK_DIM;

    size_t threadOffsetX = 0;
    size_t threadOffsetY = my_h_units * rank;

    adios2::IO io = adios.DeclareIO("hello-world-writer");
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

        var.AddOperation("plugin", params);
    }

    adios2::Engine writer = io.Open(FILENAME, adios2::Mode::Write);

    std::vector<std::int32_t> data;
    data.resize(CHUNK_SIZE);
    for(int i = 0; i != CHUNK_SIZE; ++i) {
        data[i] = i;
    }

    writer.BeginStep();

    int ySteps = my_h_chunks;
    int xSteps = my_w_chunks;
    for(int yStep = 0; yStep != ySteps; ++yStep) {
        for(int xStep = 0; xStep != xSteps; ++xStep) {
            adios2::Box<adios2::Dims> sel({xStep * CHUNK_DIM + threadOffsetX, yStep * CHUNK_DIM + threadOffsetY}, {CHUNK_DIM, CHUNK_DIM});
            var.SetSelection(sel);
            writer.Put(var, data.data());
        }
    }

    writer.EndStep();
    writer.Close();
}

void doRead(adios2::ADIOS &adios, bool doCrypt, int rank, int my_w_chunks, int my_h_chunks)
{
    adios2::IO io = adios.DeclareIO("hello-world-reader");
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
        var.AddOperation("plugin", params);
    }


    std::vector<std::int32_t> data;

    int ySteps = my_h_chunks;
    int xSteps = my_w_chunks;
    int threadOffsetX = 0;
    int threadOffsetY = my_h_chunks * CHUNK_DIM * rank;
    for(int yStep = 0; yStep != ySteps; ++yStep) {
        for(int xStep = 0; xStep != xSteps; ++xStep) {
            adios2::Box<adios2::Dims> sel({xStep * CHUNK_DIM + threadOffsetX, yStep * CHUNK_DIM + threadOffsetY}, {CHUNK_DIM, CHUNK_DIM});
            var.SetSelection(sel);
            reader.Get(var, data);
        }
    }

    reader.EndStep();
    reader.Close();
}

int main(int argc, char *argv[])
{

    MPI_Init(&argc, &argv);

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

    int rank, rank_count;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &rank_count); 

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

    if(rank == 0) {
        std::cout << "Ranks: " << rank_count << ", w_kbytes: " << w_kbytes << ", h_kbytes: " << h_kbytes << ", per rank w: "
            << my_w_chunks * CHUNK_DIM_BYTES << ", per rank h: " << my_h_chunks * CHUNK_DIM_BYTES << '\n';
    }


    try
    {
        adios2::ADIOS adios(MPI_COMM_WORLD);
        
        MPI_Barrier(MPI_COMM_WORLD);

        // do and time write
        if( modeWrite ) {
            auto start = high_resolution_clock::now();
            doWrite(adios, doCrypt, rank, w_chunks, h_chunks, my_w_chunks, my_h_chunks);
            MPI_Barrier(MPI_COMM_WORLD);
            duration<double> elapsed = high_resolution_clock::now() - start;
            if(rank == 0) std::cout << "Write: " << elapsed.count() << '\n';
        } else {
            auto start = high_resolution_clock::now();
            doRead(adios, doCrypt, rank, my_w_chunks, my_h_chunks);
            MPI_Barrier(MPI_COMM_WORLD);
            duration<double> elapsed = high_resolution_clock::now() - start;
            if(rank == 0) std::cout << "Read: " << elapsed.count() << '\n';
        }
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: ADIOS2 exception: " << e.what() << "\n";
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    MPI_Finalize();
    return 0;
}
