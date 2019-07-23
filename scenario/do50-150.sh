#!/bin/sh

for i in 50 75 100 125 150
do
cd rdm$i &&
sh do$i.sh &&
cd ../ 
done
