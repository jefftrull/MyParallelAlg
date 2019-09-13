// A multi-threaded inclusive_scan with std::accumulate "lookahead"
// Copyright 2019 Jeff Trull <edaskel@att.net>

#include "benchmark_scan.hpp"
#include "serial_scan.hpp"

#include <iterator>
#include <future>

template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
OutputIt
inclusive_scan_mt(InputIt start, InputIt end, OutputIt d_start, T init = T{})
{
    // use n_threads global (set by benchmarking code) to spawn partitions
    std::size_t sz = std::distance(start, end);
/*
    if (sz < 40000)        // arbitrary heuristic based on experiment
        // faster just to run sequentially
        return inclusive_scan_seq<InputIt, OutputIt, T>(start, end, d_start, init);
*/

    std::size_t psize = sz / n_threads;
    std::vector<std::promise<T>> part_sum_prom(n_threads - 1);
    std::vector<std::thread> part_threads(n_threads - 1);
    for (int p = 0; p < n_threads-1; ++p)
    {
        InputIt p_end = start;
        std::advance(p_end, psize);
        part_threads[p] = std::thread(
            [p, start, p_end, d_start, &part_sum_prom, &init](){
                T p_result = std::accumulate(start, p_end, T{});
                // wait for result of previous partition's accumulate
                T acc_so_far;
                if (p == 0)
                {
                    acc_so_far = init;
                } else {
                    acc_so_far = part_sum_prom[p-1].get_future().get();
                }
                // store for use by next higher partition
                part_sum_prom[p].set_value(acc_so_far + p_result);
                // lastly, store the local intermediate results
                inclusive_scan_seq<InputIt, OutputIt, T>(start, p_end, d_start, acc_so_far);
            });
        start = p_end;
        std::advance(d_start, psize);
    }

    // the last partition is special:
    // - it may not have exactly the same size as the others due to rounding
    // - there is no need to do the "accumulate" part
    // - we do it directly in this thread
    T acc_so_far = part_sum_prom.back().get_future().get();
    OutputIt d_end = inclusive_scan_seq<InputIt, OutputIt, T>(start, end, d_start, acc_so_far);

    // join all the threads we created
    for (auto & t : part_threads)
    {
        t.join();
    }

    return d_end;
}

