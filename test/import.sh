#!/bin/bash 
#参数是文件名字不带后缀
function rm_file(){
    if [ -f error ]
    then
        rm error
    fi
    if [ -f "compiler wrong" ]
    then
        rm "compiler wrong"
    fi
}
function compare_out(){
    name=$1
    outputfile="`pwd`/$name.output";
    outfile="`pwd`/$name.out";
    outputcontext=`cat $outputfile` 
    outputcontext=${outputcontext//$'\n'/}
    outcontext=`cat $outfile ` 
    outcontext=${outcontext//$'\n'/}

    if [ "$outcontext" == "$outputcontext" ]
    then
        echo "$name.sy complete"
    else
        echo  $name.sy >> "error"
    fi
}

function asm_test() {
    name=$1
    ../build/linux/x86_64/release/targettest "`pwd`/$name.sy" > "$name.s"
    if [ $? -ne 0 ]
    then
        echo "`pwd`$name.sy error 1145141919810" >> "compiler wrong"
    else
        riscv64-linux-gnu-gcc "$name.s"  ../lib/libsysy.a ../lib/memset.c --static -o "$name.o"

        #run
        if [ -f "`pwd`/$name.in"  ] 
        then
            cat "`pwd`/$name.in"|qemu-riscv64 "$name.o" > "`pwd`/$name.output"
        else
            qemu-riscv64 "$name.o" > "`pwd`/$name.output"
        fi
        ret=$?
        echo "$ret" >>  "`pwd`/$name.output"
        compare_out $name
    fi
}

function pass_test(){
    name=$1
    ../build/linux/x86_64/debug/passtest "`pwd`/$name.sy" > "$name.ll"
    if [ $? -ne 0 ]
    then
        echo "`pwd`$name.sy error 1145141919810" >> "compiler wrong"
    else
        llvm-as "$name.ll" -o "$name.tmp.bc"
        llvm-link -o "$name.bc" "$name.tmp.bc" ../lib/sylib.bc ../lib/memset.bc

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
        echo "$ret" >>  "`pwd`/$name.output"
        compare_out $name
    fi
    
}