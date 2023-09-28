#!/bin/bash

libc=0
ft=0
num=100

for (( i = 0; i<$num; i++ ))
do
	libc=$(($(/usr/bin/time -v $1 2>&1 | grep 'Minor' | awk '{ print $7 }')+$libc))
done
echo $(($libc/$num))

for (( i = 0; i<$num; i++ ))
do
	ft=$(($(./run.sh /usr/bin/time -v $1 2>&1 | grep 'Minor' | awk '{ print $7 }')+$ft))
done
echo $(($ft/$num))
