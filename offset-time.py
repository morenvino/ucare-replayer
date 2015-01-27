#!/usr/bin/env python

import argparse, sys

parser = argparse.ArgumentParser()
parser.add_argument("file", help="trace file to process", nargs='?', 
	type=argparse.FileType('r'), default=sys.stdin)
args = parser.parse_args()

first = -1.0
for line in args.file:
	token = line.split(" ")
	time = float(token[0].strip())
	if first == -1.0:
		first = time
	sys.stdout.write("{:.3f} {} {} {} {}".format(
		round(time-first, 3), token[1], token[2], token[3], token[4]))
