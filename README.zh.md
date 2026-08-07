<div align="center">

# MoYu - Lang

一门编程语言及其解释器，由 **C++17** 编写，原生系统调用由 **Rust** 实现

[![version](https://img.shields.io/badge/version-v0.1--beta-blue.svg)]()
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)]()
[![Rust](https://img.shields.io/badge/Rust-1.80-000000.svg)]()
[![license](https://img.shields.io/badge/license-MIT-green.svg)]()

</div>

---

[看不懂英文？]()

## ✨ 特性

- ⚡ **带类型标注的变量** — `let x : i32 = 10;` 或让类型检查器自动推断
- 🧩 **丰富的类型** — `i32` `u64` `f64` `string` `list` `map` `bool` `any` `lambda` ...
- 🔎 **编译期类型检查器** — 在运行前捕获类型错误，不改变程序行为
- 🎛 **类 C 控制流** — `if/elif/else`、`switch`、`for`、`while`
- 🏗 **结构体 + @method** — 点运算符方法、map 字面量构造器、lambda 字段
- 🧬 **注解** — `@native` `@async` `@platform` `@method`
- 📦 **模块系统** — `import` / `export`，懒加载
- 🗃 **容器方法** — `list.size()` `map.get()` `string.contains()` ...
- 🦀 **Rust 系统调用** — 通过 C ABI 桥接实现 fs/io/os/time，静态链接
- 💬 **REPL** — 内置交互式命令行

## 📚 文档

| 主题 | 链接 |
|---|---|
| 语言语法与句法 | [grammar](docs/grammar/v0.1.md) |
| 构建、运行与限制 | [build](docs/build/build.md) |

快速示例：

```my
import io;

let a : i32 = 10;
fn add(x : i32, y : i32) : i32 { return x + y; }
io.println(add(a, 2));        # 12

let s : string = "hello";
io.println(s.size());          # 5
```

## 📄 许可证

[MIT](LICENSE)
