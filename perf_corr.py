#!/usr/bin/env python
"""Plotting (and otherwise analyzing) the results of "perf stat" as applied to my benchmarks"""

# Copyright (c) 2019 Jeff Trull

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
import pandas as pd
import matplotlib.pyplot as plt
import argparse
import subprocess

parser = argparse.ArgumentParser(description='Run benchmarks while counting performance events')
parser.add_argument('--benchmark', dest='bm_name', required=True,
                    help='path to benchmark executable')
args = parser.parse_args()

#
# run my benchmarks under "perf stat" for a specified list of measurements,
# collecting runtimes.
#

stat_names = [
    "LLC-load-misses",
    "LLC-store-misses",
    "L1-dcache-load-misses",
    "alignment-faults",
    "dTLB-load-misses",
    "dTLB-store-misses",
]

# construct "perf stat" commandline from stat names
statcmd = ["perf", "stat"]
for stat in stat_names:
    statcmd.extend(["-e", stat])
# add the benchmark executable
statcmd.append(args.bm_name)

data = {}
for stat in stat_names:
    data[stat] = []
data['runtime'] = []

for nthreads in range(2, 8+1):

    cmd = statcmd.copy()
    bm_pattern='inclusive_scan/16777216/%d/real_time' % nthreads
    cmd.append('--benchmark_filter=%s' % bm_pattern)

    # run and parse arguments
    result = subprocess.run(cmd, capture_output=True)

    # store results in dataframe

    # stdout has the runtime info
    runtime = None
    for ln in result.stdout.decode('utf-8').splitlines():
        words = ln.split()
        if words[0] == bm_pattern:
            runtime = int(words[1])
            data['runtime'].append(runtime)
    if not runtime:
        raise RuntimeError('could not find runtime for benchmark pattern %s' % bm_pattern)

    # stderr has the perf stats
    stats = {}
    for ln in result.stderr.decode('utf-8').splitlines():
        words = ln.split()
        if len(words) in range(2, 3+1):
            statname = words[1]
            if statname in stat_names:
                stats[statname] = int(words[0].replace(',', ''))
                data[statname].append(stats[statname])

    # print('for %d threads I got %d runtime plus the following stats:' % (nthreads, runtime))
    # for k, v in stats.items():
    #     print('%s : %d' % (k, v))
    # print('\n')

    print('I ran this command: %s' % (' ').join(cmd))
    print('and got:\n%s' % result.stdout.decode('utf-8'))
    print('with stderr:\n%s' % result.stderr.decode('utf-8'))

# construct a dataframe from results
df = pd.DataFrame(data=data)

print(df)

# calculate correlation coefficients vs runtime:
coeffs = df.corr().loc['runtime', stat_names]   # the runtime column minus "runtime" (1.0)

#
# see which statistic best correlate with runtime
#

# remove any NaNs
coeffs = coeffs.dropna()

# sort rows high (most correlated) to low
coeffs = coeffs.sort_values(ascending=False)

print(coeffs)

