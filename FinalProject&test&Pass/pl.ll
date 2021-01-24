define void @main() {
entry:
  %retvalmain = alloca void
  %int = alloca i64, align 4
  %int1 = alloca i64, align 4
  %int2 = alloca i64, align 4
  %int3 = alloca i64, align 4
  %int4 = alloca i64, align 4
  store i64 3, i64* %int
  store i64 4, i64* %int1
  %0 = load i64, i64* %int1
  %1 = load i64, i64* %int
  %2 = add i64 %1, %0
  %3 = add i64 %2, 1
  store i64 %3, i64* %int2
  %4 = load i64, i64* %int1
  %5 = load i64, i64* %int
  %6 = mul i64 %5, %4
  %7 = mul i64 %6, 2
  store i64 %7, i64* %int3
  %8 = load i64, i64* %int2
  %9 = load i64, i64* %int3
  %10 = sdiv i64 %9, %8
  store i64 %10, i64* %int4
  %11 = load i64, i64* %int4
  call void @output(i64 %11)
  br label %terminator

terminator:                                       ; preds = %entry
  load void, void* %retvalmain, align 4
  ret void <badref>
}

