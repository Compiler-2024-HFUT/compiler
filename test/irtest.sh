#!/bin/bash 
source import.sh
f=$(ls testcase/*.sy)
names=""

rm_file

#get name
for file in $f
do
    name=(${file//./ })
    pass_test $name
done