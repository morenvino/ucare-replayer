#!/usr/bin/env python

# @author morenvino@uchicago.edu
# @file   Graph i/o benchmark.

# ------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------
import os, sys, subprocess, datetime
import ConfigParser
from matplotlib import pyplot as plt

# ------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------

# config file
config_file = 'config.ini' # default config file
if len(sys.argv) > 1: # config file is given in arg 
	config_file = sys.argv[1]

# read config
print 'Reading configuration'
config = ConfigParser.ConfigParser()
config.read(config_file)
log_file    = 'out/log.csv' #config.get('config', 'log_file')
nthreads_rnd = config.getint('config', 'nthreads_rnd')

# read log
#nthreads = [i for i in range(nthreads_rnd+1) if i % 2 == 0 or i == 1]
nthreads = [i for i in range(16+1) if i % 2 == 0 or i == 1]
nthreads.append(32)
#nthreads = [0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 32]
print nthreads
results  = {}
print 'Reading log'
with open(log_file, 'r') as logf:
	for mode in ['unmodified', 'modified', 'modified-nfs']:
		data = logf.readline().split(',')
		results[mode] = [] 
		for nt, i in zip(nthreads, range(len(data))):
			#results[mode][nt] = float(data[i]) 
			results[mode].append(float(data[i])) 

# generate graph
print 'Generate graph'
for mode in results.keys():
	#plt.plot(results[mode].keys(), results[mode].values(), marker='.', label=mode)
	plt.plot(nthreads, results[mode], marker='.', label=mode)
plt.axis([min(nthreads), max(nthreads), 0.0, 100.0])
plt.grid(True)
plt.legend()
plt.xticks(nthreads)
plt.xlabel('number of random write threads')
plt.ylabel('sequential write throughput (MB/s)')
plt.title('I/O throughtput')
plt.savefig('graph.png')
