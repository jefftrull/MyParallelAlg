// Wrappers for performance monitoring on Linux
// Copyright 2019, Jeff Trull <edaskel@att.net>

// The idea here is to allow you to set up some PMU events to monitor
// then enable/disable them at will to indicate "regions of interest"
// e.g. when running the parts of benchmarks that you want to measured
// this ought to be a good fit with gbench

#ifndef PMU_HPP
#define PMU_HPP

#include <cinttypes>
#include <string>

extern "C" {
#include <linux/perf_event.h>
}

struct PMUEvent {
    PMUEvent(uint32_t type, uint64_t config, std::string name);
    ~PMUEvent();

    // prevent double-closing of our stored file by making this move-only
    PMUEvent(PMUEvent const&) = delete;
    PMUEvent(PMUEvent &&);

    void off();
    void on();
    void reset();
    long long count() const;

    std::string name() const;

private:
    void _open(uint32_t, uint64_t);
    void _close();

    uint32_t type_;
    uint64_t config_;
    std::string name_;

    long fd_;

};

#endif // PMU_HPP
