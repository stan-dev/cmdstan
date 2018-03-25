#!/usr/bin/python

import sys
import numpy as np

def geo_mean(iterable):
    a = np.log(iterable)
    return np.exp(a.sum()/len(a))

def get_times(csv):
    times = []
    with open(csv) as f:
        for line in f:
            times.append(float(line.split(",")[1]))
    return times

if __name__ == "__main__":
    csv1 = sys.argv[1]
    csv2 = sys.argv[2]
    ratios = [t1/t2 for t1, t2 in zip(get_times(csv1), get_times(csv2))]
    print(ratios)
    print(geo_mean(ratios))
