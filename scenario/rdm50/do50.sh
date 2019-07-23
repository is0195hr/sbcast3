#!/bin/sh

for i in `seq 10`
do
cd $i &&
ns sbm-rdm50.tcl &&
cd ../ 
done
