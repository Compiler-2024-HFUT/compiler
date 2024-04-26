#!/bin/sh 
f=$(ls testcase/*.sy)
names=""
#!/bin/sh 
f=$(ls testcase/*.sy)
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
    source="$name.sy"
    ../build/linux/x86_64/debug/passtest "`pwd`/$source" > "$name.ll"
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
        outputfile="`pwd`/$name.output";
        outfile="`pwd`/$name.out";
            echo "$ret" >>  "`pwd`/$name.output"
	    outputcontext=`cat $outputfile` 
        outputcontext=${outputcontext//$'\n'/}
        outcontext=`cat $outfile ` 
        outcontext=${outcontext//$'\n'/}

        if [ "$outcontext" == "$outputcontext" ]
        then
            echo "$source complete"
        else
            echo  $source >> "error"
        fi
    fi

done
