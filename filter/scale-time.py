#!/usr/bin/env python

import argparse, sys, math

parser = argparse.ArgumentParser()
parser.add_argument("file", help="trace file to process", nargs='?', 
	type=argparse.FileType('r'), default=sys.stdin)
parser.add_argument("-s", help="scale time", required=True, type=float)

args = parser.parse_args()
for line in args.file:
	token = line.split(" ")
	time = float(token[0].strip()) * args.scale
	sys.stdout.write("{:.3f} {} {} {} {}".format(
		round(time,3), token[1], token[2], token[3], token[4]))	
