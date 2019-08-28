#!/usr/bin/env python
"""Script to visualize google-benchmark results for my MT scan algorithms"""
import sys
import pandas as pd
import matplotlib.pyplot as plt

# Usage:
# some_gbench_binary --benchmark_format=csv | scanplot.py

# My starting point was https://github.com/lakshayg/google_benchmark_plot
# which has a lot more features though not quite right for my needs

def parse_label(name):
    splits = name.split('/')
    if splits[0] == 'std::partial_sum':
        return splits[0]
    elif len(splits) > 3:
        # use alg/threadcount
        return "%s/%d" % (splits[0], int(splits[2]))
    else:
        raise RuntimeError('could not get thread count from %s' % name)


def parse_nthreads(label):
    splits = label.split('/')
    if len(splits) == 3:
        # no thread counts
        return 1
    if len(splits) != 4:
        import pdb;pdb.set_trace()
    return int(splits[2])


# Get data from benchmark CSV results
try:
    data = pd.read_csv(sys.stdin, usecols=['name', 'real_time'])
except ValueError:
    raise RuntimeError("is my input a valid CSV?")

# Add useful columns derived from "name" column
# a column for the benchmark data size
data['size'] = data['name'].apply(lambda n : int(n.split('/')[1]))
# one for the graph legend
data['label'] = data['name'].apply(parse_label)
# one for the thread count
data['nthreads'] = data['name'].apply(parse_nthreads)

# remove redundant benchmark "name" field (text output of gbench) from data
names_removed = data.loc[:, ['label', 'size', 'real_time']]

# reshape data so "size" is row label and columns are one per benchmark
reshaped = names_removed.pivot(index='size', columns='label', values='real_time')

# normalize all columns to partial_sum

normalized = reshaped.apply(lambda col : col / reshaped['std::partial_sum'])

# having our desired lines in columns we can now plot:
normalized.plot(logx=True)

# and tell matplotlib to display the result
plt.title('delay vs size (normalized to partial_sum)')
plt.show()

# show a second plot doing a "cross-section" - delay vs nthreads for size=16777216
df2 = data.loc[:, ['label', 'size', 'real_time', 'nthreads']].copy()

# grab all data of the desired size that isn't a partial_sum result
df2 = df2[(df2['size'] == 16777216) & (df2['label'] != "std::partial_sum")]

# pick only the two columns of interest and make nthreads the index
del_vs_nthreads = df2.loc[:, ['real_time', 'nthreads']].set_index('nthreads')

# normalize the delays to std::partial_sum
norm_const = float(names_removed[(names_removed['label'] == 'std::partial_sum') &
                                 (names_removed['size'] == 16777216)]['real_time'])
del_vs_nthreads['real_time'] = del_vs_nthreads['real_time'].apply(lambda t : t / norm_const)

del_vs_nthreads.plot()
plt.title('delay vs thread count (normalized) for 16M ints')
plt.show()

