#!/bin/sh 
#the argv1 only need filename don't add path before filename
    name=(${1//./ })
    name=testcase/$name
    source="$name.sy"
    ../build/linux/x86_64/debug/irtest "`pwd`/$source" > "$name.ll"
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
    rm "`pwd`/$name.bc" "`pwd`/$name.output"
    echo $ret