xmake run irtest $1 > main.ll
llvm-as main.ll -o main.bc
llvm-link -o output.bc main.bc ./lib/sylib.bc

lli output.bc

echo -e "\nreturn_val:\n"$?

rm -rf main.bc output.bc