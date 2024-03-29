// Comparing a plain std::partial_sum vs std::accumulate plus std::partial_sum in a second thread
// Copyright 2019 Jeff Trull <edaskel@att.net>
#include "benchmark_scan.hpp"
#include "serial_scan.hpp"

#include <thread>
#include <future>
#include <iterator>
#include <iostream>
#include <vector>
#include <numeric>

template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
OutputIt
inclusive_scan_2t(InputIt start, InputIt end, OutputIt d_start, T init)
{
    // calculate partition point (halfway)
    std::size_t sz = std::distance(start, end);
    InputIt part = start;
    std::advance(part, sz / 2);

    // launch first partition thread
    std::promise<T> p1_sum_prom;
    std::thread p1(
        [start, part, d_start, &p1_sum_prom, &init](){
            // calculate the sum of the first partition
            T p1_result = std::accumulate(start, part, T{});
            // send it to be used for the second partition
            p1_sum_prom.set_value(p1_result);
            // make a second pass to store the intermediate values
            inclusive_scan_seq<InputIt, OutputIt, T>(start, part, d_start, init);
        });

    // complete calculation using accumulation result from partition 1
    T p1_sum = p1_sum_prom.get_future().get();
    OutputIt d_part = d_start;
    std::advance(d_part, sz / 2);
    OutputIt d_end = inclusive_scan_seq(part, end, d_part, p1_sum);

    // wait for the first partition to complete
    p1.join();

    return d_end;
}

int main(int argc, char* argv[])
{
    // process and remove gbench arguments
    benchmark::Initialize(&argc, argv);

    register_benchmark(
        "std::partial_sum",
        partial_sum<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);

    register_benchmark(
        "inclusive_scan (2T)",
        inclusive_scan_2t<
            std::vector<int>::const_iterator,
            std::vector<int>::iterator>);

    benchmark::RunSpecifiedBenchmarks();
}
