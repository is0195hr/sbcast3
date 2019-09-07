#!/bin/sh

for i in `seq 10`
do
cd $i 
rm *.tr 
rm *.nam 
rm *.csv 
cd ../ 
done
