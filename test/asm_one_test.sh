#!/bin/bash 
#the argv1 only need filename don't add path before filename
name=(${1//./ })
name=$name
source="$name.sy"
../build/linux/x86_64/release/targettest "`pwd`/$source" > "$name.s"
if [ $? -ne 0 ]
then
    echo "`pwd`$name.sy error 1145141919810" >> "compiler wrong"
else
    riscv64-linux-gnu-gcc "$name.s"  ../lib/libsysy.a ../lib/memset.c -o "$name.o"

    #run
    if [ -f "`pwd`/$name.in"  ] 
    then
        cat "`pwd`/$name.in"|qemu-riscv64 "$name.o" > "`pwd`/$name.output"
    else
        qemu-riscv64 "$name.o" > "`pwd`/$name.output"
    fi
    ret=$?
    outputfile="`pwd`/$name.output";
    outfile="`pwd`/$name.out";
        echo "$ret" >>  "`pwd`/$name.output"
    outputcontext=`cat $outputfile` 
    outputcontext=${outputcontext//$'\n'/}
    outcontext=`cat $outfile ` 
    outcontext=${outcontext//$'\n'/}

    if [ "$outcontext"=="$outputcontext" ]
    then
        echo "$source complete"
    else
        echo  $source >> "error"
    fi
fi