// Comparing a plain std::partial_sum vs std::accumulate/std::partial_sum in multiple threads
// Copyright 2019 Jeff Trull <edaskel@att.net>
#include "benchmark_scan.hpp"
#include "serial_scan.hpp"

#include <thread>
#include <future>
#include <iterator>
#include <iostream>
#include <vector>
#include <numeric>

template <typename InputIt, typename OutputIt>
OutputIt
inclusive_scan_mt(InputIt start, InputIt end, OutputIt d_start)
{
    using T = typename std::iterator_traits<InputIt>::value_type;

    // use n_threads global (set by benchmarking code) to spawn partitions
    std::size_t sz = std::distance(start, end);
    if (sz < 40000)        // arbitrary heuristic based on experiment
        // faster just to run sequentially
        return inclusive_scan_serial<InputIt, OutputIt, T>(start, end, d_start, T{});

    std::size_t psize = sz / n_threads;
    std::vector<std::promise<T>> part_sum_prom(n_threads - 1);
    std::vector<std::thread> part_threads(n_threads - 1);
    for (int p = 0; p < n_threads-1; ++p)
    {
        InputIt p_end = start;
        std::advance(p_end, psize);
        part_threads[p] = std::thread(
            [p, start, p_end, d_start, &part_sum_prom](){
                T p_result = std::accumulate(start, p_end, T{});
                // wait for result of previous partition's accumulate
                T acc_so_far;
                if (p == 0)
                {
                    acc_so_far = T{};
                } else {
                    acc_so_far = part_sum_prom[p-1].get_future().get();
                }
                // store for use by next higher partition
                part_sum_prom[p].set_value(acc_so_far + p_result);
                // lastly, store the local intermediate results
                inclusive_scan_serial<InputIt, OutputIt, T>(start, p_end, d_start, acc_so_far);
            });
        start = p_end;
        std::advance(d_start, psize);
    }

    // the last partition is special:
    // - it may not have exactly the same size as the others due to rounding
    // - there is no need to do the "accumulate" part
    // - we do it directly in this thread
    T acc_so_far = part_sum_prom.back().get_future().get();
    OutputIt d_end = inclusive_scan_serial<InputIt, OutputIt, T>(start, end, d_start, acc_so_far);

    // join all the threads we created
    for (auto & t : part_threads)
    {
        t.join();
    }

    return d_end;
}

int main(int argc, char* argv[])
{
    // process and remove gbench arguments
    benchmark::Initialize(&argc, argv);

    // set up event counters
    events_to_count.emplace_back(PERF_TYPE_HARDWARE,
                                 PERF_COUNT_HW_INSTRUCTIONS,
                                 "instructions");
    events_to_count.emplace_back(PERF_TYPE_HARDWARE,
                                 PERF_COUNT_HW_CPU_CYCLES,
                                 "cycles");
    register_benchmark(
        "std::partial_sum",
        std::partial_sum<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);

    register_benchmark_mt(
        "inclusive_scan",
        inclusive_scan_mt<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);

    benchmark::RunSpecifiedBenchmarks();
}
