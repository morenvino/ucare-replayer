#!/usr/bin/env python

# @author morenvino@uchicago.edu
# @file   Direct i/o benchmark.

# ------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------
import os, sys, subprocess, datetime
import ConfigParser
from matplotlib import pyplot as plt

# ------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------
def run(command):
	'''Run the given shell command'''
	return subprocess.check_output(command.split())

def sshq(command):
	'''Run the given command in guest QEMU'''
	return subprocess.check_output(['sshq', command])

def scpq(filename):
	'''Copy the given file to guest QEMU'''
	#return '{} scpq {}'.format('done' if status == 0 else 'error', filename) 
	subprocess.call(['scpq', filename])
	
def now():
	'''Return the current time in string'''
	return datetime.datetime.now().strftime('%Y%m%d_%H%m%S')

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
repetition  = config.getint('config', 'repeat')
disk_host   = config.get('config', 'disk_host')
log_file    = config.get('config', 'log_file')
binary_file = config.get('config', 'binary_file')
nthreads_rnd = config.getint('config', 'nthreads_rnd')

# collect env info
print 'Collecting env info'
curr_time   = now()
kernel_name = run('uname -s').strip() 
kernel_ver  = run('uname -r').strip()
platform    = run('uname -p').strip()
disk_info   = run('qemu-img info {disk}'.format(disk=disk_host))

# log env
log  = config
log.add_section('host')
log.set('host', 'curr_time', curr_time)
log.set('host', 'kernel_name', kernel_name)
log.set('host', 'kernel_ver', kernel_ver)
log.set('host', 'platform', platform)
log.set('host', 'disk_info', disk_info)

# compile
qemu_src = '~/qemu-src'
qemu_vm  = '~/vm'

# prepare guest
print 'Copying files to guest'
scpq(binary_file)
scpq(config_file)

# run benchmark
print 'Running benchmark'
nthreads = [i for i in range(nthreads_rnd+1) if i % 2 == 0 or i == 1]
results  = {}
for nt in nthreads:
	print 'with', nt, 'threads of random write'
	results[nt] = 0.0
	for n in range(repetition):
		throughput = sshq("sudo ./{bin} {nt}".format(bin=binary_file, nt=nt))
		throughput = float(throughput)
		print ' throughput:', round(throughput, 2), 'MB/s'  
		results[nt] = results[nt] + throughput
	results[nt] = results[nt] / repetition
	print ' average   :', round(results[nt], 2), 'MB/s' 

# log result into csv
#log.add_section('log')
#for nt, result in results.iteritems(): #zip(nthreads, results):
#	log.set('log', 't-{}'.format(nt), '{} MB/s'.format(result))

# dump log into log_file
out_path = 'out' #curr_time 
if not os.path.exists(out_path): os.mkdir(out_path) 
os.chdir(out_path)
with open(log_file, 'wb+') as logf:
	log.write(logf)

# also log result into csv file
with open('log.csv', 'a') as logf:
	for nt, result in results.iteritems():
		logf.write(str(result))
		logf.write(',')
	logf.write('\n')

# generate graph
# plt.plot(results.keys(), results.values(), marker='.', label='unmodified')
# plt.axis([min(results.keys()), max(results.keys()), 0.0, 100.0])
# plt.grid(True)
# plt.legend()
# plt.xlabel('number of random write threads')
# plt.ylabel('sequential write throughput (MB/s)')
# plt.title('I/O throughtput')
# plt.savefig('graph.png')
