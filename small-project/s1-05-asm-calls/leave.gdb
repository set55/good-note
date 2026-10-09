break *main+68
run
x/i $pc
info registers rsp rbp
x/gx $rbp
stepi
x/i $pc
info registers rsp rbp
x/gx $rsp
continue
