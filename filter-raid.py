#!/usr/bin/env python

import argparse, sys, math

parser = argparse.ArgumentParser()
parser.add_argument("file", help="trace file to process", nargs='?', 
	type=argparse.FileType('r'), default=sys.stdin)
parser.add_argument("-ndisk", help="number of raid disks", type=int, default=2)
parser.add_argument("-odisk", help="observed disk", type=int, default=0)
parser.add_argument("-stripe", help="stripe unit size", type=int, default=4096)

args = parser.parse_args()

blk_size = 512
ndisk, odisk, stripe = args.ndisk, args.odisk, args.stripe
scaler = stripe / blk_size

def calculate_raid_blk(blk_start, blk_count, time=0):
	blk_count_per_disk = [0] * ndisk
	blk_start_per_disk = [0] * ndisk
	
	current_blk = blk_start
	for _ in range(blk_count):
		current_disk = (current_blk / scaler) % ndisk
		blk_count_per_disk[current_disk] += 1
		current_blk += 1

	current_disk = (blk_start / scaler) % ndisk
	blk_start_per_disk[current_disk] = blk_start
	for i in range(1, ndisk):
		current_disk = (current_disk + 1) % ndisk
		blk_start_per_disk[current_disk] = ((blk_start / scaler) + i) * scaler

	# sanity check ...
	#if sum(blk_count_per_disk) != blk_count:
	#	print 'error in time:', time,
	#	'blk_count:', blk_count, 'but sum:', sum(blk_count_per_disk)
	#print '>', blk_count, blk_count_per_disk,  blk_count_per_disk[odisk]
	
	return blk_start_per_disk[odisk], blk_count_per_disk[odisk]

for line in args.file:
	token = line.split(" ")
	time = token[0]
	devno = token[1]
	blkno = int(token[2].strip())
	blkcount = int(token[3].strip())
	flags = token[4]
	
	blkno, blkcount = calculate_raid_blk(blkno, blkcount, time=time)
	if blkcount != 0:
		sys.stdout.write("{} {} {} {} {}".format(
			time, devno, blkno, blkcount, flags))
