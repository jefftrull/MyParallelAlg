// Compare std::accumulate vs std::partial_sum to measure raw read vs. write latency
// Copyright 2019 Jeff Trull <edaskel@att.net>

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

    // first benchmark: std::accumulate
    benchmark::RegisterBenchmark(
        "std::accumulate",
        [&](benchmark::State &state) {
            // create and fill random vector of desired size
            std::random_device rnd_device;
            std::mt19937 mersenne_engine{rnd_device()};
            std::uniform_int_distribution<int> dist{1, 20};

            // get input size from first range argument (supplied below)
            auto sz = state.range(0);
            std::vector<int> data(sz);
            std::generate(data.begin(), data.end(),
                          [&]() { return dist(mersenne_engine); });
            for (auto _ : state) {
                int result = std::accumulate(data.begin(), data.end(), 0);
                benchmark::DoNotOptimize(result);
            }
        })->Range(10, 40000000)->UseRealTime();

    benchmark::RegisterBenchmark(
        "std::partial_sum",
        [&](benchmark::State &state) {
            // create and fill random vector of desired size
            std::random_device rnd_device;
            std::mt19937 mersenne_engine{rnd_device()};
            std::uniform_int_distribution<int> dist{1, 20};

            auto sz = state.range(0);
            std::vector<int> data(sz);
            std::generate(data.begin(), data.end(),
                          [&]() { return dist(mersenne_engine); });
            std::vector<int> result = data;
            for (auto _ : state) {
                std::partial_sum(result.begin(), result.end(), result.begin());
                benchmark::DoNotOptimize(result);
            }
        })->Range(10, 40000000)->UseRealTime();

    benchmark::RunSpecifiedBenchmarks();
}
