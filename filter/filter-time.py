#!/usr/bin/env python

import argparse, sys

parser = argparse.ArgumentParser()
parser.add_argument("file", help="trace file to process", nargs='?', 
	type=argparse.FileType('r'), default=sys.stdin)
parser.add_argument("-s", help="start from this second", 
	type=int, default=0)
parser.add_argument("-f", help="finish to this second", 
	type=int, default=0)

args = parser.parse_args()
start = args.s * 1000 # convert to ms
finish = args.f * 1000
for line in args.file:
	token = line.split(" ")
	time  = float(token[0].strip())
	if time < start:
		continue
	if time > finish:
		break
	sys.stdout.write(line)
