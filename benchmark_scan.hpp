// Benchmarking code for inclusive_scan
// Copyright 2019 Jeff Trull <edaskel@att.net>

#ifndef BENCHMARK_SCAN_HPP
#define BENCHMARK_SCAN_HPP

#include <iostream>
#include <iterator>
#include <vector>
#include <numeric>
#include <random>
#include <string>

#include <benchmark/benchmark.h>

int n_threads = 2;

//
// break out all the code necessary to benchmark an inclusive_scan implementation here
//

// a fixture that sets up the input for us
// I started with the gbench fixture one but I can't see how to register
// a nice name for it - always comes out FIXTURENAME/BENCHNAME
template<typename T>
struct RandomInputFixture
{
    RandomInputFixture(benchmark::State const & state)
    {
        // create and fill random vector of desired size
        std::random_device rnd_device;
        std::mt19937 mersenne_engine{rnd_device()};
        std::uniform_int_distribution<T> dist{1, 20};

        // get input size from first range argument (supplied below)
        auto sz = state.range(0);
        data.resize(sz);
        std::generate(data.begin(), data.end(),
                      [&]() { return dist(mersenne_engine); });
    }

    std::vector<T> data;
};

// create and register benchmark on a scan function
template <typename InputIt, typename OutputIt>
void
register_benchmark(const char * name,
                   OutputIt (*fn)(InputIt, InputIt, OutputIt))
{
    using T = typename std::iterator_traits<InputIt>::value_type;

    benchmark::RegisterBenchmark(
        name,
        [fn](benchmark::State & state)
        {
            RandomInputFixture<T> inp(state);
            std::vector<T> result = inp.data;
            for (auto _ : state) {
                fn(result.begin(), result.end(), result.begin());
                benchmark::DoNotOptimize(result);
            }
        })->Range(512, 1 << 27)->UseRealTime();
}

// custom Range approach as described in docs
void
nt_range_setter(benchmark::internal::Benchmark* b)
{
    for (int sz = 512; sz <= (1 << 27); sz *= 8)
        for (int nt = 2; nt <= 8; ++nt)
            b->Args({sz, nt});
}

// a benchmark that uses multiple threads
template <typename InputIt, typename OutputIt>
void
register_benchmark_mt(const char * name,
                      OutputIt (*fn)(InputIt, InputIt, OutputIt))
{
    using T = typename std::iterator_traits<InputIt>::value_type;

    benchmark::RegisterBenchmark(
        name,
        [fn](benchmark::State & state)
        {
            RandomInputFixture<T> inp(state);
            n_threads = state.range(1);
            std::vector<T> result = inp.data;
            for (auto _ : state) {
                fn(result.begin(), result.end(), result.begin());
                benchmark::DoNotOptimize(result);
            }
        })->Apply(nt_range_setter)->UseRealTime();
}

#endif // BENCHMARK_SCAN_HPP
