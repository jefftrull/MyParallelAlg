// Implementation of wrappers for performance monitoring on Linux
// Copyright 2019, Jeff Trull <edaskel@att.net>

#include "pmu.hpp"

#include <string.h>

extern "C" {
#include <asm/unistd.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
}

#include <stdexcept>
#include <iostream>

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                  group_fd, flags);
    return ret;
}

PMUEvent::PMUEvent(uint32_t type, uint64_t config, std::string name)
    : type_(type), config_(config), name_(std::move(name))
{
    _open(type, config);
}

PMUEvent::~PMUEvent()
{
    _close();
}

void
PMUEvent::_open(uint32_t type, uint64_t config)
{
    // fill out perf_event_attr
    struct perf_event_attr pe;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.size = sizeof(struct perf_event_attr);
    pe.type = type;
    pe.config = config;

    // special featues
    pe.disabled = 1;        // start in "off" (exists, but not counting) state
    pe.inherit = 1;         // child threads get counters as well
    pe.inherit_stat = 1;    // saved on context switch. Seems like a good idea!
    pe.exclude_kernel = 1;  // measure only user events
    pe.exclude_idle = 1;
    // BOZO consider "counter groups"

    // ask kernel to set up
    fd_ = perf_event_open(&pe, 0, -1, -1, 0);

    if (fd_ == -1)
    {
        throw std::runtime_error("could not create PMU event");
    }
    ioctl(fd_, PERF_EVENT_IOC_RESET, 0);
}

void
PMUEvent::_close()
{
    if (fd_ != -1)
    {
        close(fd_);
        fd_ = -1;
    }
}

PMUEvent::PMUEvent(PMUEvent && other)
    : type_(other.type_), config_(other.config_), name_(other.name_), fd_(other.fd_)
{
    other.fd_ = -1;  // invalidate moved-from fd
}

void
PMUEvent::on()
{
    ioctl(fd_, PERF_EVENT_IOC_ENABLE, 0);
}

void
PMUEvent::off()
{
    ioctl(fd_, PERF_EVENT_IOC_DISABLE, 0);
}

void
PMUEvent::reset()
{
    // it seems clear from reviewing the Linux source that child counts are not reset
    // by PERF_EVENT_IOC_RESET. See _perf_event_reset() and perf_event_count() in event.c.
    // Since I am collecting child events via the "inherit" flag I have to take the unpleasant
    // approach of simply closing and reopening the underlying file descriptor

    _close();
    _open(type_, config_);

    if (count() != 0)
        throw std::runtime_error("performance counter should be 0 immediately after reset");
}

long long
PMUEvent::count() const
{
    long long ct;
    auto err = read(fd_, &ct, sizeof(long long));
    if (err == -1)
    {
        throw std::runtime_error("could not read PMU event " + name_ + ": " + std::string(strerror(errno)));
    }

    return ct;
}

std::string
PMUEvent::name() const
{
    return name_;
}
