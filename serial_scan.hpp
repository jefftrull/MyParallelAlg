// Building blocks for parallel inclusive_scan
// Copyright 2019 Jeff Trull <edaskel@att.net>

#ifndef SERIAL_SCAN_HPP
#define SERIAL_SCAN_HPP

template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
std::pair<OutputIt, T>
inclusive_scan_seq_impl(InputIt start, InputIt end, OutputIt d_start, T init = T{})
{
    T sum = init;

//    bool overwrite_mode = (&(*start) == &(*d_start));

    // do 16 (one cache line) at a time
    const std::size_t items_per_line = 16;
    auto sz = std::distance(start, end);   // this may be costly for fwd iterators

    // take care of any items that are not chunks of 16 first
    auto preamble_sz = sz % items_per_line;
    for (std::size_t ofs = 0; ofs < preamble_sz; ++ofs)
    {
        sum += *start++;
        *d_start++ = sum;
    }

    // now work 16 at a time
    auto pf = start; std::advance(pf, items_per_line);
    auto d_pf = d_start; std::advance(d_pf, items_per_line);
    for (; start != end; std::advance(pf, items_per_line), std::advance(d_pf, items_per_line))
    {
        __builtin_prefetch(&(*pf), 0, 3);
        __builtin_prefetch(&(*d_pf), 1, 2);
/*
        if (overwrite_mode)
            __builtin_prefetch(&(*pf), 1, 3);
        else
        {
            __builtin_prefetch(&(*pf), 0, 2);
            __builtin_prefetch(&(*d_pf), 1, 2);
        }
*/

        // iterate over the cache line
        while (start != pf)
        {
            sum += *start++;
            *d_start++ = sum;
        }
    }
    return std::make_pair(d_start, sum);
}

template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
OutputIt
inclusive_scan_seq(InputIt start, InputIt end, OutputIt d_start, T init = T{})
{
    return inclusive_scan_seq_impl(start, end, d_start, init).first;
}

#endif // SERIAL_SCAN_HPP
