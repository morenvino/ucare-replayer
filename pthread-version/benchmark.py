#!/usr/bin/env python

# @author morenvino@uchicago.edu
# @file   Direct i/o benchmark.

# ------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------
import os, sys, subprocess, datetime
import ConfigParser

# ------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------
def run(command):
	'''Run the given shell command'''
	return subprocess.check_output(command.split())

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
config = ConfigParser.ConfigParser()
config.read(config_file)
disk_host  = config.get('config', 'disk_host')
log_file   = config.get('config', 'log_file')

# collect env info
curr_time   = now()
kernel_name = run('uname -s').strip() 
kernel_ver  = run('uname -r').strip()
platform    = run('uname -p').strip()
disk_info   = run('qemu-img info {}'.format(disk_host))

# run experiment
	sshq "sudo ./$(EXEC1) /dev/sdb 8192000"

# put env info into log
log  = config
log.add_section('log')
log.set('log', 'curr_time', curr_time)
log.set('log', 'kernel_name', kernel_name)
log.set('log', 'kernel_ver', kernel_ver)
log.set('log', 'platform', platform)
log.set('log', 'disk_info', disk_info)

# dump env info log into file
out_path = 'out' #curr_time 
if not os.path.exists(out_path): os.mkdir(out_path) 
os.chdir(out_path)
with open(log_file, 'wb+') as logf:
	log.write(logf)

# generate graph