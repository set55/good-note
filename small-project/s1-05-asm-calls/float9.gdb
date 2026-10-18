break *main
break *main+113
run
echo == 進 main 時 ==\n
info registers rsp
continue
echo == 執行 call printf 之前 ==\n
info registers rsp
x/gx $rsp
x/fg $rsp
p $xmm0.v2_double[0]
p $xmm7.v2_double[0]
p $rax
