break *add3+35
run
info registers rsp rbp
print &a
print &s
x/5gx $rbp-0x28
