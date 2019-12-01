; ModuleID = 'fib'
source_filename = "fib"

define i32 @fib(i32) {
entry:
  %1 = alloca i32
  %2 = alloca i32
  store i32 %0, i32* %2
  %3 = load i32, i32* %2
  %4 = alloca i32
  %5 = icmp eq i32 %3, 0
  br i1 %5, label %trueBB, label %falseBB

trueBB:                                           ; preds = %entry
  store i32 0, i32* %4
  br label %12

falseBB:                                          ; preds = %entry
  %6 = icmp eq i32 %3, 1
  br i1 %6, label %false_trueBB, label %false_falseBB

false_trueBB:                                     ; preds = %falseBB
  store i32 1, i32* %4
  br label %12

false_falseBB:                                    ; preds = %falseBB
  %7 = sub nsw i32 %3, 1
  %8 = call i32 @fib(i32 %7)
  %9 = sub nsw i32 %3, 2
  %10 = call i32 @fib(i32 %9)
  %11 = add nsw i32 %8, %10
  store i32 %11, i32* %4
  br label %12

; <label>:12:                                     ; preds = %false_falseBB, %false_trueBB, %trueBB
  %13 = load i32, i32* %4
  store i32 %13, i32* %1
  ret i32 %13
}

define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca float
  store float 8.000000e+00, float* %1
  %2 = alloca i32
  store i32 1, i32* %2
  %3 = load float, float* %1
  %4 = fptoui float %3 to i32
  br label %judgeBB

judgeBB:                                          ; preds = %loopBB, %entry
  %5 = load i32, i32* %2
  %6 = icmp slt i32 %5, %4
  br i1 %6, label %loopBB, label %7

; <label>:7:                                      ; preds = %judgeBB
  %8 = load i32, i32* %0
  ret i32 %8

loopBB:                                           ; preds = %judgeBB
  %9 = load i32, i32* %0
  %10 = call i32 @fib(i32 %5)
  %11 = add nsw i32 %9, %10
  store i32 %11, i32* %0
  %12 = add nsw i32 %5, 1
  store i32 %12, i32* %2
  br label %judgeBB
}
