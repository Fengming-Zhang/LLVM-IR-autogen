;assembly
;func_def_syntax
;block_syntax
;var_def_stmt_syntax
;literal_syntax
;var_def_stmt_syntax
;literal_syntax
;var_def_stmt_syntax
;binop_expr_syntax
;literal_syntax
;binop_expr_syntax
;literal_syntax
;literal_syntax
;literal_syntax
;literal_syntax
;literal_syntax
;while_stmt_syntax
;cond_syntax
;lval_syntax
;binop_expr_syntax
;lval_syntax
;literal_syntax
;block_syntax
;assign_stmt_syntax
;lval_syntax
;lval_syntax
;lval_syntax
;assign_stmt_syntax
;lval_syntax
;binop_expr_syntax
;lval_syntax
;literal_syntax
;assign_stmt_syntax
;lval_syntax
;binop_expr_syntax
;lval_syntax
;literal_syntax
;assign_stmt_syntax
;lval_syntax
;lval_syntax
;func_call_stmt_syntax
;assign_stmt_syntax
;lval_syntax
;lval_syntax
;lval_syntax
;func_call_stmt_syntax
;assign_stmt_syntax
;lval_syntax
;lval_syntax
;func_call_stmt_syntax
;assign_stmt_syntax
;lval_syntax
;lval_syntax
;func_call_stmt_syntax
; ModuleID = 'test2.c1'
source_filename = "test2.c1"

@input_ivar = global i32 0
@input_fvar = global double 0.000000e+00
@output_ivar = global i32 0
@output_fvar = global double 0.000000e+00

declare void @inputInt_impl(i32*)

declare void @inputFloat_impl(double*)

declare void @outputInt_impl(i32*)

declare void @outputFloat_impl(double*)

define void @inputInt() {
entry:
  call void @inputInt_impl(i32* @input_ivar)
  ret void
}

define void @inputFloat() {
entry:
  call void @inputFloat_impl(double* @input_fvar)
  ret void
}

define void @outputInt() {
entry:
  call void @outputInt_impl(i32* @output_ivar)
  ret void
}

define void @outputFloat() {
entry:
  call void @outputFloat_impl(double* @output_fvar)
  ret void
}

define void @main() {
  %i = alloca i32
  store i32 9, i32* %i
  %j = alloca i32
  store i32 3, i32* %j
  %a = alloca [10 x i32]
  %1 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 0
  store i32 1, i32* %1
  %2 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 1
  store i32 2, i32* %2
  %3 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 2
  store i32 3, i32* %3
  br label %4

; <label>:4:                                      ; preds = %9, %0
  %5 = load i32, i32* %i
  %6 = load i32, i32* %j
  %7 = sub i32 %6, 3
  %8 = icmp sgt i32 %5, %7
  br i1 %8, label %9, label %30

; <label>:9:                                      ; preds = %4
  %10 = load i32, i32* %i
  %11 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 %10
  %12 = load i32, i32* %11
  %13 = load i32, i32* %j
  store i32 %13, i32* %11
  %14 = load i32, i32* %j
  %15 = load i32, i32* %j
  %16 = add i32 %15, 1
  store i32 %16, i32* %j
  %17 = load i32, i32* %i
  %18 = load i32, i32* %i
  %19 = sub i32 %18, 1
  store i32 %19, i32* %i
  %20 = load i32, i32* @output_ivar
  %21 = load i32, i32* %i
  store i32 %21, i32* @output_ivar
  call void @outputInt()
  %22 = load i32, i32* @output_ivar
  %23 = load i32, i32* %i
  %24 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 %23
  %25 = load i32, i32* %24
  store i32 %25, i32* @output_ivar
  call void @outputInt()
  %26 = load i32, i32* @output_ivar
  %27 = load i32, i32* %j
  store i32 %27, i32* @output_ivar
  call void @outputInt()
  %28 = load i32, i32* @output_ivar
  %29 = load i32, i32* %i
  store i32 %29, i32* @output_ivar
  call void @outputInt()
  br label %4

; <label>:30:                                     ; preds = %4
  ret void
}
