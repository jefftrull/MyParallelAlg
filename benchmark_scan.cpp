// Benchmarking implementation for inclusive_scan
// Copyright 2019 Jeff Trull <edaskel@att.net>

#include "benchmark_scan.hpp"

// instantiate globals
int n_threads = 2;
std::vector<PMUEvent> events_to_count;

// range setter
void
nt_range_setter(benchmark::internal::Benchmark* b)
{
    for (int sz = 1 << 1; sz <= (1 << 27); sz *= 2)
        for (int nt = 2; nt <= 8; ++nt)
            b->Args({sz, nt});
}
