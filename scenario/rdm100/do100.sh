#!/bin/sh

for i in `seq 10`
do
cd $i &&
ns sbm-rdm100.tcl &&
cd ../ 
done
