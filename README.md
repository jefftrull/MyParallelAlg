# MyParallelAlg
I made my own parallel algorithm in C++17. This repo records its evolution.

## Comparing Read and Write Latency
Parallelizing scan algorithms requires some form of "lookahead" that can be performed by another thread, in parallel, faster than the primary work. In this case we measure the difference between:

1. Reading a value and adding it to a running total
2. Reading a value, adding it to a running total, and storing the intermediate result

The `std::accumulate` and `std::partial_sum` algorithms, respectively, are good examples for benchmarking.

For a nice clear plot of the results - which show a ratio of between 2.4 and 3.4 in runtime on my system - try using [this useful script](https://github.com/lakshayg/google_benchmark_plot). My command line is:

`./avps --benchmark_format=csv | python ../../google_benchmark_plot/plot.py --logx --logy`
