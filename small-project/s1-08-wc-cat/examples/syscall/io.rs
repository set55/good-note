// io.rs：Rust 讀 in.txt 印到 stdout
use std::fs;
use std::io::Write;

fn main() {
    let data = fs::read("in.txt").unwrap();
    std::io::stdout().write_all(&data).unwrap();
}
