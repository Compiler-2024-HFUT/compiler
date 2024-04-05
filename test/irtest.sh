#!/bin/sh 
f=$(ls testcase/*.sy)
names=""

#get name
for file in $f
do
    name=(${file//./ })
    names="$names  $name"
done

#run
for name in $names
do
    source="$name.sy"
    ../build/linux/x86_64/debug/irtest "`pwd`/$source" > "$name.ll"
    if [ $? -ne 0 ]
    then
        echo "$name.sy error 1145141919810\n\n\n\n\n"
    else
        llvm-as "$name.ll" -o "$name.tmp.bc"
        llvm-link -o "$name.bc" "$name.tmp.bc" ../lib/sylib.bc

        rm  "`pwd`/$name.tmp.bc" 
        #run
        if [ -f "`pwd`/$name.in"  ] 
        then
            cat "`pwd`/$name.in"|lli "$name.bc" > "`pwd`/$name.output"
        else
            lli "$name.bc" > "`pwd`/$name.output"
        fi
        ret=$?
        rm "`pwd`/$name.bc" 
        echo "$name complete"
        echo $ret
    fi

done