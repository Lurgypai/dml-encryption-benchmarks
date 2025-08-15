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

#include <iostream>
#include <chrono>

#include <stdexcept>

#include <cstdint>
#include <cstring>

#include <adios2.h>
#if ADIOS2_USE_MPI
#include <mpi.h>
#endif

using namespace std::chrono;

void doWrite(adios2::ADIOS &adios, bool doCrypt, int width, int height)
{
    adios2::IO io = adios.DeclareIO("hello-world-writer");
    adios2::Variable<std::int32_t> var = io.DefineVariable<std::int32_t>(
        "Var",
        {width, height},    // base dimensions
        {0, 0},             // threat offset
        {width, height}     // thread region
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
    
    auto start = high_resolution_clock::now();
    writer.Put(var, data.data());
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    std::cout << "Write: " << elapsed.count() << '\n';

    writer.EndStep();
    writer.Close();
}

void doRead(adios2::ADIOS &adios, bool doCrypt)
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
    auto start = high_resolution_clock::now();
    reader.Get(var, data);
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    std::cout << "Read: " << elapsed.count() << '\n';

    reader.EndStep();
    reader.Close();
}

int main(int argc, char *argv[])
{

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

#if ADIOS2_USE_MPI
    MPI_Init(&argc, &argv);
#endif

    try
    {
#if ADIOS2_USE_MPI
        adios2::ADIOS adios(MPI_COMM_WORLD);
#else
        adios2::ADIOS adios;
#endif

        doWrite(adios, doCrypt, w, h);
        doRead(adios, doCrypt);
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: ADIOS2 exception: " << e.what() << "\n";
#if ADIOS2_USE_MPI
        MPI_Abort(MPI_COMM_WORLD, -1);
#endif
    }

#if ADIOS2_USE_MPI
    MPI_Finalize();
#endif

    return 0;
}
