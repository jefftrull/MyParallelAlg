// Comparing a sequential scan vs. a chunked, multi-threaded version
// Copyright 2019 Jeff Trull <edaskel@att.net>
#include "inclusive_scan_chunked.hpp"

#include "benchmark_scan.hpp"
#include "serial_scan.hpp"

#include <thread>
#include <iostream>
#include <vector>
#include <numeric>

int main(int argc, char* argv[])
{
    // process and remove gbench arguments
    benchmark::Initialize(&argc, argv);

    register_benchmark(
        "inclusive_scan_seq",
        inclusive_scan_seq<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);

    register_benchmark_mt(
        "inclusive_scan_chunked",
        inclusive_scan_chunked<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);


    benchmark::RunSpecifiedBenchmarks();
}
