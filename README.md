<div align="center">

# MoYu - Lang

A programming language · Interpreter in **C++17**, native syscalls in **Rust**

[![version](https://img.shields.io/badge/version-v1.0--beta-blue.svg)]()
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)]()
[![Rust](https://img.shields.io/badge/Rust-1.80-000000.svg)]()
[![license](https://img.shields.io/badge/license-MIT-green.svg)]()

</div>

---

[看不懂英文？](./README.zh.md)

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

## ⏳ Not yet implemented (v1.0)

- **Coroutines / `await`** — the syntax is parsed, but `await` currently evaluates its argument synchronously (it does not suspend). Planned for a future version.

- **Linux Platform** The current project has not yet been compiled and run on the Linux platform. Therefore, if there are any compilation issues on the Linux platform, please raise an issue immediately. I will resolve it as soon as possible.

## 📚 Documentation

| topic | link |
|---|---|
| Language grammar & syntax (EN) | [v1.0.en.md](docs/grammar/v1.0.en.md) |
| Language grammar & syntax (ZH) | [v1.0-zh.md](docs/grammar/v1.0-zh.md) |
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
