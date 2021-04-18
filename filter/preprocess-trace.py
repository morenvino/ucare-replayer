#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("file", help="trace file to process")
parser.add_argument("-f", "--filter", help="filter only read/write events", choices=['write', 'read'])
parser.add_argument("-o", "--offset", help="start time offset", type=float, default=0.0)
args = parser.parse_args()

type_filter = -1
if args.filter == "write":
    type_filter = 0	
elif args.filter == "read":
    type_filter = 1

with open(args.file) as f:
	# skip header
	for line in f:
            if line[:9] == "EndHeader":
	        break
	first_line = True
	for line in f:		
		tok = map(str.lstrip, line.split(","))
		flags = -1

		if tok[0] == "DiskWrite":
			flags = 0
		elif tok[0] == "DiskRead":
			flags = 1

		if flags == -1:
			continue
		if type_filter != -1 and type_filter != flags:
                        continue
				
		if first_line:
			args.offset = -float(tok[1]) / 1000.0
			first_line = False

		t = {
			"time": (float(tok[1]) / 1000.0) + args.offset,
			"devno": int(tok[8]),
			"blkno": int(tok[5], 16) / 512,
			"bcount": int(tok[6], 16) / 512,
			"flags": flags,
		};

		print "%s %d %d %d %d" % ("{0:.3f}".format(t['time']), t['devno'], t['blkno'], t['bcount'], t['flags'])

