#!/bin/bash 
source import.sh
f=$(ls t/functional/*.sy)
names=""

rm_file

#get name
for file in $f
do
    name=(${file//./ })
    asm_test $name
done