#!/bin/sh 
#the argv1 only need filename don't add path before filename
    name=(${1//./ })
    name=testcase/$name
    source="$name.sy"
    ../build/linux/x86_64/debug/irtest "`pwd`/$source" > "$name.ll"
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
        echo "$name complete"
        outfile="`pwd`/$name.output";

        # if [ $(tail -n1 $outfile | wc -l) -eq 1 ];then
            echo "$ret" >>  "`pwd`/$name.output"
        # else
            # echo -e "\n$ret" >>  "`pwd`/$name.output"
        # fi
	    
    fi