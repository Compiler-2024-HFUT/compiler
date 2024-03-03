assert() {
  # 程序运行的 期待值 为参数1
    # compiler="$1"
    compiler="../build/linux/x86_64/release/astTest"
  # 输入值 为参数2
  input="$2"

  # 运行程序，传入期待值，将生成结果写入tmp.s汇编文件。
  # 如果运行不成功，则会执行exit退出。成功时会短路exit操作
  $compiler "$input" #> tmp.s || exit
  # 编译rvcc产生的汇编文件
  if [ "$?" = 0  ];then
    echo "\$?==0"
    else
        echo "\$?==0"
  fi
  # $RISCV/bin/riscv64-unknown-linux-gnu-gcc -static -o tmp tmp.s tmp2.o

  # 运行生成出来目标文件
  # $RISCV/bin/qemu-riscv64 -L $RISCV/sysroot ./tmp
  # $RISCV/bin/spike --isa=rv64gc $RISCV/riscv64-unknown-linux-gnu/bin/pk ./tmp

  # 获取程序返回值，存入 实际值
  actual="$?"

  # 判断实际值，是否为预期值
#   if [ "$actual" = "$expected" ]; then
#     echo "$input => $actual"
#   else
#     echo "$input => $expected expected, but got $actual"
#     exit 1
#   fi
}

# assert 期待值 输入值
# [1] 返回指定数值
echo $f
#ls /home/user | grep ".*\.png\|.*\.jpg"
for file in $f
do
    "../build/linux/x86_64/release/astTest" $file > /dev/null
    if [ "$?" != 0  ];then
        #echo "\$?==0"
        echo $file
    fi
done