// Compare std::accumulate vs std::partial_sum to measure raw read vs. write latency
// Copyright 2019 Jeff Trull <edaskel@att.net>

#include "benchmark_scan.hpp"

#include <iostream>
#include <vector>
#include <numeric>
#include <random>

// introduce Google Benchmark
#include <benchmark/benchmark.h>

int main(int argc, char* argv[])
{
    // process and remove gbench arguments
    benchmark::Initialize(&argc, argv);

    benchmark::RegisterBenchmark(
        "std::accumulate",
        [](benchmark::State &state)
        {
            RandomInputFixture<int> inp(state);
            for (auto _ : state) {
                int result = std::accumulate(inp.data.begin(), inp.data.end(), 0);
                benchmark::DoNotOptimize(result);
            }
        })->Range(8, 1 << 25)->UseRealTime();

    register_benchmark(
        "std::partial_sum",
        std::partial_sum<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);


    benchmark::RunSpecifiedBenchmarks();
}
