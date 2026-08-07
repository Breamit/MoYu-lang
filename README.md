<div align="center">

# MoYu - Lang

A programming language · Interpreter in **C++17**, native syscalls in **Rust**

[![version](https://img.shields.io/badge/version-v0.1--beta-blue.svg)]()
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)]()
[![Rust](https://img.shields.io/badge/Rust-1.80-000000.svg)]()
[![license](https://img.shields.io/badge/license-MIT-green.svg)]()

</div>

---

[don't understand English?]()

## ✨ Features

- ⚡ **Type-annotated variables** — `let x : i32 = 10;` or let the type checker infer it
- 🧩 **Rich types** — `i32` `u64` `f64` `string` `list` `map` `bool` `any` `lambda` ...
- 🔎 **Compile-time type checker** — catches type errors before running, without changing behavior
- 🎛 **C-like control flow** — `if/elif/else`, `switch`, `for`, `while`
- 🏗 **struct + @method** — dot-operator methods, map-literal constructors, lambda fields
- 🧬 **Annotations** — `@native` `@async` `@platform` `@method`
- 📦 **Modules** — `import` / `export`, lazy-loaded
- 🗃 **Container methods** — `list.size()` `map.get()` `string.contains()` ...
- 🦀 **Rust syscalls** — fs/io/os/time through a C ABI bridge, statically linked
- 💬 **REPL** — interactive shell built in

## 📚 Documentation

| topic | link |
|---|---|
| Language grammar & syntax | [grammar](docs/grammar/v0.1.md) |
| Build, run & limitations (EN) | [build.en.md](docs/build/build.en.md) |
| Build, run & limitations (ZH) | [build.zh.md](docs/build/build.zh.md) |

Quick example:

```my
import io;

let a : i32 = 10;
fn add(x : i32, y : i32) : i32 { return x + y; }
io.println(add(a, 2));        # 12

let s : string = "hello";
io.println(s.size());          # 5
```

## 📄 License

[MIT](LICENSE)
