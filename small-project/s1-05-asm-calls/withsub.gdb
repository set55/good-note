break *main+16
run
x/i $pc
info registers rsp rbp
x/gx $rbp-8
stepi
x/i $pc
info registers rsp
x/gx $rbp-8
continue
