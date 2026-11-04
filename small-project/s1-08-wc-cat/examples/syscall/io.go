// io.go：Go 讀 in.txt 印到 stdout
package main

import "os"

func main() {
	data, _ := os.ReadFile("in.txt")
	os.Stdout.Write(data)
}
