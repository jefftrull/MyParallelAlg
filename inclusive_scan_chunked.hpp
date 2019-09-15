// An inclusive_scan implementation that "chunks" the input before dispatching to multi-threaded code
// Copyright 2019 Jeff Trull <edaskel@att.net>

#ifndef INCLUSIVE_SCAN_CHUNKED_HPP
#define INCLUSIVE_SCAN_CHUNKED_HPP

#include "benchmark_scan.hpp"
#include "inclusive_scan_mt.hpp"

#include <iterator>

template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
OutputIt
inclusive_scan_chunked(InputIt start, InputIt end, OutputIt d_start, T init = T{})
{
    std::size_t const chunksize = 400000;   // the sweet spot - experimentally *shrug*
    // Reader, I want you to know I tried to science this but experiment won out!

    std::size_t sz = std::distance(start, end);

    std::size_t nchunks = sz / chunksize;

    if (nchunks == 0)
    {
        return inclusive_scan_mt(start, end, d_start, init);
    }

    for (std::size_t chunk = 0; chunk < (nchunks - 1); ++chunk)
    {
        auto p_end = start; std::advance(p_end, chunksize);
        std::tie(d_start, init) = inclusive_scan_mt_impl(start, p_end, d_start, init);
        std::advance(start, chunksize);
    }

    // do the final (irregular size) chunk
    return inclusive_scan_mt(start, end, d_start, init);
}

#endif // INCLUSIVE_SCAN_CHUNKED_HPP
