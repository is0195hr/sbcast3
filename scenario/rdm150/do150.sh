#!/bin/sh

for i in `seq 10`
do
echo $i 'sim start at '`date` >> autosimlog.tr &&
cd $i &&
ns sbm-rdm150.tcl &&
cd ../ &&
echo $i 'sim end at '`date` >> autosimlog.tr
done
