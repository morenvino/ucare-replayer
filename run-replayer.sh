#!/bin/bash -x

#mode="shelter"
mode="default"
#traces="DAP-PS"
#traces="DAP-PS RAD-BE DTRS"
#traces="RAD-BE-10000 DAP-PS-10000 DTRS-10000"
traces="RAD-BE-180s DAP-PS-180s DTRS-180s"
nthreads=200
logdir=/log
outdir=.

sudo hdparm -W0 /dev/sdb
for trace in $traces
do
	tracefile="$trace.trace"
	i=5 # 5: hdparm -W0

	
	# run replayer
	sudo ./replayer.out $tracefile $nthreads
	sync
	sleep 5

	# collect log
	#logdir=$trace-$mode-$i 
	#mkdir $logdir
	outlog=$trace-$mode-$i
	cat $logdir/* | sort -n >> $outdir/$outlog.log
	sudo rm $logdir/*
done
