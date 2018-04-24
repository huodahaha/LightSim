#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np  
import matplotlib.pyplot as plt  

policies = ["Random", "LRU", "LIP", "BIP", "DIP"]
benchmarks = ["astar_23B", "h264ref_178B", "omnetpp_4B", "bzip2_183B", "gcc_13B", "milc_744B"]

def read_duo_L2(policy, benchmark):
    path = "./%s/%s"%(benchmark, policy)
    hit_ratio = 0
    with open(path) as f:
        buf = f.read()
        line1 = buf.split("\n")[12]
        line2 = buf.split("\n")[13]
        hit = float(line1.split(" ")[-1])
        miss = float(line2.split(" ")[-1])
        hit_ratio = hit/(hit + miss)
    return hit_ratio

def read_alone_L2(policy, benchmark):
    path = "./%s/%s"%(benchmark, policy)
    with open(path) as f:
        buf = f.read()
        line1 = buf.split("\n")[8]
        line2 = buf.split("\n")[9]
        hit = float(line1.split(" ")[-1])
        miss = float(line2.split(" ")[-1])
        hit_ratio = hit/(hit + miss)
    return hit_ratio

def drawBarChartPoseRatio(data):
    n_groups = 6

    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity   = 0.4

    rects1 = plt.bar(index, data["Random"], bar_width/2, alpha=opacity, color='r', label='Random')
    rects2 = plt.bar(index+bar_width/2, data["LRU"], bar_width/2, alpha=opacity, color='g', label='LRU')
    rects3 = plt.bar(index+bar_width, data["LIP"], bar_width/2, alpha=opacity, color='c', label='LIP')
    rects4 = plt.bar(index+bar_width*3/2, data["BIP"], bar_width/2, alpha=opacity, color='m', label='BIP')
    rects5 = plt.bar(index+bar_width*2, data["DIP"], bar_width/2, alpha=opacity, color='y', label='DIP')

    plt.xlabel('Benchmarks', fontsize = 18)
    plt.ylabel('Hit Ratio', fontsize = 18)
    plt.title('Hit Ratio in LLC', fontsize = 18)

    plt.xticks(index - 0.2+ 2*bar_width, ("astar", "h264ref", "omnetpp", "bzip2", "gcc", "milc"), fontsize =18)

    plt.yticks(fontsize =18)  #change the num axis size

    plt.ylim(0,1)
    plt.legend()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    data = {}
    for policy in policies:
        data[policy] = []
        for benchmark in benchmarks:
            data[policy].append(readL2(policy, benchmark))

    drawBarChartPoseRatio(data)
