#!/bin/bash -x

traces_dir='DAP-PS DTRS RAD-BE'

for dir in $traces_dir
do
	echo In $dir
	mkdir -p $dir-out
	for trace in `ls $dir`
	do
		echo Processing $dir/$trace
		python preprocess.py $dir/$trace > $dir-out/$trace 
	done
done


