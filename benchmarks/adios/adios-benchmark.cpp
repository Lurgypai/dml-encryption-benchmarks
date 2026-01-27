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

void doWrite(adios2::ADIOS &adios, bool doCrypt, int rank, int width, int height, int thread_w, int thread_h)
{
    adios2::IO io = adios.DeclareIO("hello-world-writer");
    adios2::Variable<std::int32_t> var = io.DefineVariable<std::int32_t>(
        "Var",
        {width, height},                    // base dimensions
        {thread_w * rank, thread_h * rank}, // thread offset
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

    writer.BeginStep();
    std::vector<std::int32_t> data;
    data.resize(width * height);
    for(int i = 0; i != width * height; ++i) {
        data[i] = i;
    }
    
    writer.Put(var, data.data());

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

    adios2::Box<adios2::Dims> sel({thread_w * rank, thread_h * rank}, {thread_w, thread_h});
    var.SetSelection(sel);

    std::vector<std::int32_t> data;
    reader.Get(var, data);

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
    printf("rank=%d size=%d pid=%d\n", rank, size, getpid());

    // not evenly divisible
    if(w % size != 0 || h % size != 0) {
        if(rank == 0) std::cout << "Dimensions are not evenly divisible, aborting.\n";
        return 1;
    }

    std::uint64_t my_w = w / size;
    std::uint64_t my_h = h / size;

    if(rank == 0) {
        std::cout << "Ranks: " << size << ", w: " << w << ", h: " << h << ", per rank w: " << my_w << ", per rank h: " << my_h << '\n';
    }


    try
    {
        adios2::ADIOS adios(MPI_COMM_WORLD);

        // do and time write
        auto start = high_resolution_clock::now();
        doWrite(adios, doCrypt, rank, w, h, my_w, my_h);
        duration<double> elapsed = high_resolution_clock::now() - start;
        if(rank == 0) std::cout << "Write: " << elapsed.count() << '\n';

        start = high_resolution_clock::now();
        doRead(adios, doCrypt, rank, my_w, my_h);
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
