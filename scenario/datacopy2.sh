#!/bin/sh

for i in 100 125 150
do
cd rdm$i &&
mkdir data-AFC
for j in 1 2 3 4 5 6 7 8 9 10
do
cd $j &&
mv res.tr res$i-$j.tr &&
mv resAll.csv resAll$i-$j.csv &&
mv res$i-$j.tr  resAll$i-$j.csv ../data-AFC &&
cd ../ 
done
cd ../ 
done
