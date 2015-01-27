#!/usr/bin/env python

import argparse, sys

parser = argparse.ArgumentParser()
parser.add_argument("file", help="trace file to process", nargs='?', 
	type=argparse.FileType('r'), default=sys.stdin)
parser.add_argument("-d", help="devno to include", required=True, type=int)

args = parser.parse_args()
for line in args.file:
	token = line.split(" ")
	devno = int(token[1].strip())
	if devno == args.d:
		sys.stdout.write(line)
