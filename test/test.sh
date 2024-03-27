f=$(ls sy/*.sy)
# echo $f
for file in $f
do
    "../build/linux/x86_64/release/irTest" $file > /dev/null
    if [ "$?" != 0  ];then
        #echo "\$?==0"
        echo $file
    fi
done