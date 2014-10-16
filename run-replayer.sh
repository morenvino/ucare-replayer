#!/bin/bash -x

#mode="shelter"
mode="b15m0d"
#traces="DAP-PS"
#traces="DAP-PS RAD-BE DTRS"
#traces="RAD-BE-10000 DAP-PS-10000 DTRS-10000"
traces="RAD-BE DTRS"
nthreads=8
logdir=/log
outdir=/home/morenvino/replayer-log

sudo hdparm -W0 /dev/sdb
for trace in $traces
do
	tracefile="$trace-$mode.trace"
	i=210 
	
	# run replayer
	sudo ./replayer.out $tracefile 
	sync
	sleep 10

	# collect log
	#logdir=$trace-$mode-$i 
	#mkdir $logdir
	outlog=$trace-$mode-bare-$i
	cat $logdir/* | sort -n >> $outdir/$outlog.log
	sudo rm $logdir/*
done

