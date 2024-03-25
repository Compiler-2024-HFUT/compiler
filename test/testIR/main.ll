declare i32 @getint()

declare float @getfloat()

declare i32 @getch()

declare i32 @getarray(i32*)

declare i32 @getfarray(float*)

declare void @putint(i32)

declare void @putfloat(float)

declare void @putch(i32)

declare void @putarray(i32, i32*)

declare void @putfarray(i32, float*)

declare void @_sysy_starttime(i32)

declare void @_sysy_stoptime(i32)

define i32 @main() {
label_entry:
  %op0 = alloca i32
  store i32 1, i32* %op0
  br label %label_ret
label_ret:                                                ; preds = %label_entry
  %op1 = load i32, i32* %op0
  ret i32 %op1
}