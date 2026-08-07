<div align="center">

# MoYu - Lang

一门编程语言及其解释器，由 **C++17** 编写，原生系统调用由 **Rust** 实现

[![version](https://img.shields.io/badge/version-v0.1--beta-blue.svg)]()
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)]()
[![Rust](https://img.shields.io/badge/Rust-1.80-000000.svg)]()
[![license](https://img.shields.io/badge/license-MIT-green.svg)]()

</div>

---

[don't understand Chinese?](README.md)

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
- 
## ⏳还未实现 (v1.0)

- **协程 / `await`** — 虽然协程的语法代码可以被正常解析，但是当前版本并不支持异步，需要等待后续版本更新异步处理机制，当然，您现在依旧可以使用异步的语法，只是无法生效，这并不影响代码的顺序运行

- **Linux 平台** 当前项目还未在 Linux 平台上编译运行，所以如果在Linux平台上发生编译问题，请立刻提issue，我会尽快解决的。

## 📚 文档

| 主题 | 链接 |
|---|---|
| 语言语法与句法 | [grammar](docs/grammar/v1.0.zh.md.md) |
| 构建、运行与限制 | [build](docs/build/build.zh.md.md) |

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
