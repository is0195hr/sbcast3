#!/bin/sh

for i in 100 125 150
do
cd rdm$i
cd data-AFC
for j in `seq 10`
do
cat resAll$i-$j.csv >> ALL$i.csv
done
cd ../
cd ../ 
done
