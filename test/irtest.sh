#!/bin/bash 
source import.sh
f=$(ls testcase/*.sy)
names=""

rm_file

#get name
for file in $f
do
    name=(${file//./ })
    compile_pass $name
done
for file in $f
do
    name=(${file//./ })
    pass_test $name
done