#!/bin/bash 
source import.sh
f=$(ls testcase/performance/*.sy)
names=""

if [ -f error ]
then
    rm error
fi

#get name
for file in $f
do
    name=(${file//./ })
    names="$names  $name"
done

#run
for name in $names
do
    pass_test $name
done
