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

#include <stdexcept>

#include <cstdint>
#include <cstring>

#include <adios2.h>
#include <mpi.h>

using namespace std::chrono;

std::string key;

// 16 ints X 16 ints is 4KiB
// 512 X 512 ins is 1MiB
constexpr std::size_t CHUNK_DIM{512};
constexpr std::size_t CHUNK_SIZE{ CHUNK_DIM * CHUNK_DIM };

void doWrite(adios2::ADIOS &adios, bool doCrypt, int rank, int width, int height, int thread_w, int thread_h)
{

    int threadOffsetX = thread_w * rank;
    int threadOffsetY = thread_h * rank;

    adios2::IO io = adios.DeclareIO("hello-world-writer");
    adios2::Variable<std::int32_t> var = io.DefineVariable<std::int32_t>(
        "Var",
        {width, height},                    // base dimensions
        {threadOffsetX, threadOffsetY},     // thread offset
        {thread_w, thread_h}                // thread region
    );

    if(doCrypt) {
        io.SetEngine("BPFile");
        adios2::Params params;
        params["PluginName"] = "MyOperator";
        params["PluginLibrary"] = "EncryptionOperator";
        params["SecretKeyFile"] = "secret-key";

        var.AddOperation("plugin", params);
    }

    adios2::Engine writer = io.Open("benchmark_out.bp", adios2::Mode::Write);

    std::vector<std::int32_t> data;
    data.resize(CHUNK_SIZE);
    for(int i = 0; i != CHUNK_SIZE; ++i) {
        data[i] = i;
    }

    writer.BeginStep();

    int xSteps = thread_w / CHUNK_DIM;
    int ySteps = thread_h / CHUNK_DIM;
    for(int xStep = 0; xStep != xSteps; ++xStep) {
        for(int yStep = 0; yStep != ySteps; ++yStep) {
            adios2::Box<adios2::Dims> sel({xStep * CHUNK_DIM + threadOffsetX, yStep * CHUNK_DIM + threadOffsetY}, {CHUNK_DIM, CHUNK_DIM});
            var.SetSelection(sel);
            writer.Put(var, data.data());
        }
    }


    writer.EndStep();
    writer.Close();
}

void doRead(adios2::ADIOS &adios, bool doCrypt, int rank, int thread_w, int thread_h)
{
    adios2::IO io = adios.DeclareIO("hello-world-reader");
    adios2::Engine reader = io.Open("benchmark_out.bp", adios2::Mode::Read);
    reader.BeginStep();
    adios2::Variable<std::int32_t> var =
        io.InquireVariable<std::int32_t>("Var");

    if(doCrypt) {
        io.SetEngine("BPFile");
        adios2::Params params;
        params["PluginName"] = "cryptop";
        params["PluginLibrary"] = "EncryptionOperator";
        params["SecretKeyFile"] = "secret-key";
        var.AddOperation("plugin", params);
    }

    std::vector<std::int32_t> data;

    int xSteps = thread_w / CHUNK_DIM;
    int ySteps = thread_h / CHUNK_DIM;
    int threadOffsetX = thread_w * rank;
    int threadOffsetY = thread_h * rank;
    for(int xStep = 0; xStep != xSteps; ++xStep) {
        for(int yStep = 0; yStep != ySteps; ++yStep) {
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
    if (argc != 4) {
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

    std::uint64_t w = atoi(argv[2]);
    std::uint64_t h = atoi(argv[3]);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size); 

    // not evenly divisible
    if(w % size != 0 || h % size != 0) {
        if(rank == 0) std::cout << "Dimensions are not evenly divisible, aborting.\n";
        return 1;
    }

    std::uint64_t my_w = w / size;
    std::uint64_t my_h = h / size;

    if(my_w % CHUNK_DIM != 0 || my_h % CHUNK_DIM != 0) {
        if(rank == 0) std::cout << "Per thread dimensions are not evenly divisble, aborting.\n";
        return 1;
    }

    if(rank == 0) {
        std::cout << "Ranks: " << size << ", w: " << w << ", h: " << h << ", per rank w: " << my_w << ", per rank h: " << my_h << '\n';

        // setup key
        // I'm lazy so everything just uses a 0 initialized key
        key.resize(16);
    }


    try
    {
        adios2::ADIOS adios(MPI_COMM_WORLD);
        
        MPI_Barrier(MPI_COMM_WORLD);

        // do and time write
        auto start = high_resolution_clock::now();
        doWrite(adios, doCrypt, rank, w, h, my_w, my_h);
        MPI_Barrier(MPI_COMM_WORLD);
        duration<double> elapsed = high_resolution_clock::now() - start;
        if(rank == 0) std::cout << "Write: " << elapsed.count() << '\n';

        start = high_resolution_clock::now();
        doRead(adios, doCrypt, rank, my_w, my_h);
        MPI_Barrier(MPI_COMM_WORLD);
        elapsed = high_resolution_clock::now() - start;
        if(rank == 0) std::cout << "Read: " << elapsed.count() << '\n';
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: ADIOS2 exception: " << e.what() << "\n";
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    MPI_Finalize();
    return 0;
}
