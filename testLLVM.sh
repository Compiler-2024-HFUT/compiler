xmake run irTest $1 > main.ll
llvm-as main.ll -o main.bc
llvm-link -o output.bc main.bc ./lib/sylib.bc

lli output.bc

echo ""
echo "return value:"
echo $?

rm -rf main.bc output.bc