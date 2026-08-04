<div align="center">

#  MoYu - Lang

A programing language · Interpreter in **C++17**, native syscalls in **Rust**

[![version](https://img.shields.io/badge/version-0.1--beta-blue.svg)]()
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)]()
[![Rust](https://img.shields.io/badge/Rust-1.80-000000.svg)]()
[![license](https://img.shields.io/badge/license-MIT-green.svg)]()

</div>

---

## ✨ Features

- ⚡ **Type-annotated** variables — `let x : i32 = 10;`
- 🧩 **Rich types** — `i32` `u64` `f64` `string` `list` `map` `bool` ...
- 🎛 **C-like control flow** — `if/elif/else`, `switch`, `for`, `while`
- 🏗 **struct + @method** — dot-operator methods, map-literal constructors
- 🧬 **Annotations** — `@native` `@async` `@platform` `@object`
- 📦 **Modules** — `import` / `export`, lazy-loaded
- 🦀 **Rust syscalls** — fs/io through a C ABI bridge
- 💬 **REPL** — interactive shell built in

## 🚀 Build

```bash
cd rust/fs_syscall && cargo build --release
cmake -G "MinGW Makefiles" -S . -B build
cmake --build build
```

## ▶️ Run

```bash
./build/moyu.exe              # REPL
./build/moyu.exe run x.my     # run script
./build/moyu.exe --version
```

## 📖 Grammar

Full spec: **[MoYu Grammar v0.1](MoYu/docs/grammar/v0.1.md)**

```my
let a : i32 = 10;
fn add(x : i32, y : i32) : i32 { return x + y; }
io.println(add(1, 2));
```

## 📄 License

[MIT](LICENSE)
