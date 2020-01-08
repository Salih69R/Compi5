@.intFormat = internal constant [4 x i8] c"%d\0A\00"
define i32 @fn_fib(i32) {
fn_fib_entry:
  %1 = icmp sle i32 %0, 1
  br i1 %1, label %fn_fib_entry.if, label %fn_fib_entry.endif
 fn_fib_entry.if:
  ret i32 %0
fn_fib_entry.endif:
  %2 = sub i32 %0, 1
  %3 = sub i32 %0, 2
  %4 = call i32 @fn_fib(i32 %2)
  %5 = call i32 @fn_fib(i32 %3)
  %6 = add i32 %4, %5 ret i32 %6
}

define i32 @main() {
entry:
  %0 = call i32 @fn_fib(i32 10)
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0), i32 %0)
  %2 = add i32 0, 4
			; alloca i8
			; store i8 4, i8* %2
  

  %3 = call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0) , i32 %2)
  ret i32 0
}
declare i32 @printf(i8*, ...)
