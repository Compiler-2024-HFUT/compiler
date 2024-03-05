f=$(ls ../src/sy/*.sy)
# echo $f
for file in $f
do
    "../build/linux/x86_64/release/astTest" $file > /dev/null
    if [ "$?" != 0  ];then
        #echo "\$?==0"
        echo $file
    fi
done