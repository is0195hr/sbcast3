#!/bin/sh

for i in 150
do
cd rdm$i &&
mkdir data-AFC
for j in 1 2 3 4 5 6 7 8 9 10
do
cd $j &&
mv mytrace.tr mytrace$i-$j.tr &&
mv res.tr res$i-$j.tr &&
mv mytrace$i-$j.tr  res$i-$j.tr ../data-AFC &&
cd ../ 
done
cd ../ 
done
