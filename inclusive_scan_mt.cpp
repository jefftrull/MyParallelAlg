// Comparing a sequential scan vs. a multi-threaded version with std::accumulate "lookahead"
// Copyright 2019 Jeff Trull <edaskel@att.net>
#include "inclusive_scan_mt.hpp"

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

/*
    // set up event counters
    events_to_count.emplace_back(PERF_TYPE_HARDWARE,
                                 PERF_COUNT_HW_INSTRUCTIONS,
                                 "instructions");
    events_to_count.emplace_back(PERF_TYPE_HARDWARE,
                                 PERF_COUNT_HW_CPU_CYCLES,
                                 "cycles");
    events_to_count.emplace_back(PERF_TYPE_HARDWARE,
                                 PERF_COUNT_HW_STALLED_CYCLES_FRONTEND,
                                 "frontend-stalls");
    events_to_count.emplace_back(PERF_TYPE_HARDWARE,
                                 PERF_COUNT_HW_STALLED_CYCLES_BACKEND,
                                 "backend-stalls");
*/


    register_benchmark(
        "inclusive_scan_seq",
        inclusive_scan_seq<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);

    register_benchmark_mt(
        "inclusive_scan",
        inclusive_scan_mt<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);


    benchmark::RunSpecifiedBenchmarks();
}
