// Benchmarking code for inclusive_scan
// Copyright 2019 Jeff Trull <edaskel@att.net>

#ifndef BENCHMARK_SCAN_HPP
#define BENCHMARK_SCAN_HPP

#include "pmu.hpp"

#include <benchmark/benchmark.h>

#include <iostream>
#include <iterator>
#include <vector>
#include <numeric>
#include <random>
#include <string>

//
// GLOBALS
// we need to supply some sideband information from the benchmark which should not pass
// through the normal scan algorithm interface. This is my solution for now (sigh)
// Maybe the "Passing Arbitrary Arguments to a Benchmark" section has some ideas
//

extern int n_threads;
extern std::vector<PMUEvent> events_to_count;

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
template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
void
register_benchmark(const char * name,
                   OutputIt (*fn)(InputIt, InputIt, OutputIt, T))
{
    benchmark::RegisterBenchmark(
        name,
        [fn](benchmark::State & state)
        {
            RandomInputFixture<T> inp(state);
            std::vector<T> result(state.range(0));
            for (auto _ : state) {
                // enable event counting
                state.PauseTiming();
                for (auto & evt : events_to_count)
                {
                    evt.on();
                }
                state.ResumeTiming();

                // run target code
                fn(inp.data.begin(), inp.data.end(), result.begin(), T{});
                benchmark::DoNotOptimize(result);

                // disable event counting
                state.PauseTiming();
                for (auto & evt : events_to_count)
                {
                    evt.off();
                }
                state.ResumeTiming();

            }

            // record counter results
            for (auto & evt : events_to_count)
            {
                auto ct = evt.count();
                state.counters[evt.name()] = ct / state.iterations();;
                evt.reset();
            }

       })->RangeMultiplier(2)->Range(1 << 1, 1 << 27)->UseRealTime();
}

// custom Range approach as described in docs
// allows us to cover threadcount linearly and size exponentially
void
nt_range_setter(benchmark::internal::Benchmark* b);

// a benchmark that uses multiple threads
template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
void
register_benchmark_mt(const char * name,
                      OutputIt (*fn)(InputIt, InputIt, OutputIt, T))
{

    benchmark::RegisterBenchmark(
        name,
        [fn](benchmark::State & state)
        {
            RandomInputFixture<T> inp(state);
            n_threads = state.range(1);
            std::vector<T> result(state.range(0));
            for (auto _ : state) {
                // enable event counting
                state.PauseTiming();
                for (auto & evt : events_to_count)
                {
                    evt.on();
                }
                state.ResumeTiming();

                // run target code
                fn(inp.data.begin(), inp.data.end(), result.begin(), T{});
                benchmark::DoNotOptimize(result);

                // disable event counting
                state.PauseTiming();
                for (auto & evt : events_to_count)
                {
                    evt.off();
                }
                state.ResumeTiming();

            }

            // record counter results
            for (auto & evt : events_to_count)
            {
                state.counters[evt.name()] = evt.count() / state.iterations();
                evt.reset();    // does not seem to reset the counter for some reason
                if (evt.count() > 100)
                {
                    std::cerr << "counter value: " << evt.count() << "\n";
                    throw std::runtime_error("counter was not reset!\n");
                }
            }

        })->Apply(nt_range_setter)->UseRealTime();
}

// wrap std::partial_sum so it takes an initial value
template <typename InputIt, typename OutputIt,
          typename T = typename std::iterator_traits<InputIt>::value_type>
OutputIt
partial_sum(InputIt start, InputIt stop, OutputIt d_stop, T /* just discard, for benchmarking */)
{
    return std::partial_sum(start, stop, d_stop);
}

#endif // BENCHMARK_SCAN_HPP
