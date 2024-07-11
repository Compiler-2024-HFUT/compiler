./build/linux/x86_64/debug/testAnalysis $1 > tmp.ll
opt -enable-new-pm=0 -analyze -scalar-evolution tmp.ll
rm tmp.ll