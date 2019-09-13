// Building blocks for parallel inclusive_scan
// Copyright 2019 Jeff Trull <edaskel@att.net>

#ifndef SERIAL_SCAN_HPP
#define SERIAL_SCAN_HPP

template <typename InputIt, typename OutputIt, typename T = typename std::iterator_traits<InputIt>::value_type>
std::pair<OutputIt, T>
inclusive_scan_seq_impl(InputIt start, InputIt end, OutputIt d_start, T init = T{})
{
    T sum = init;
    for (; start != end; start++, d_start++)
    {
        sum += *start;
        *d_start = sum;
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
