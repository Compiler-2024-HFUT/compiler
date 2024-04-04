f=$(ls testcase/*.sy)
# echo $f
for file in $f
do
    "../build/linux/x86_64/debug/irtest" $file > /dev/null
    if [ "$?" != 0  ];then
        #echo "\$?==0"
        echo $file
    fi
done