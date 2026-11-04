# io.py：Python 的 os 模組，讀 in.txt 印到 stdout
import os
fd = os.open("in.txt", os.O_RDONLY)
data = os.read(fd, 16)
os.close(fd)
os.write(1, data)
