# MoYu 构建指南（从零开始）

**- 版本：v0.1-beta**

本指南将带你在一台全新的机器上从零开始构建 MoYu —— 安装工具链、交叉编译 Rust 系统调用库，并生成 `moyu.exe` 解释器。

---

## 1. 你需要什么

| 工具 | 最低版本 | 用途 |
|---|---|---|
| **MinGW-w64 (g++)** | C++17 | 编译解释器 |
| **CMake** | 3.10 | 构建系统 |
| **Rust (rustup)** | 1.80 | 编译原生系统调用库 |
| **Git**（可选） | 任意 | 克隆仓库 |

MoYu 在 **Windows** 上构建。解释器使用 C++17 编写，原生系统调用（fs / os / time）是 Rust 库，以静态链接方式编译进可执行文件。

---

## 2. 安装工具链

### 2.1 MinGW-w64 (g++)

下载 MinGW-w64，并将其 `bin` 目录加入 `PATH`。验证：

```bash
g++ --version
# g++ (MinGW-W64) 15.2.0
```

你必须能够编译 C++17：

```bash
echo 'int main(){return 0;}' > /tmp/t.cpp && g++ -std=c++17 /tmp/t.cpp -o /tmp/t.exe && /tmp/t.exe && echo OK
# OK
```

### 2.2 CMake

安装 CMake 并验证：

```bash
cmake --version
# cmake version 3.x.x
```

### 2.3 带 GNU 目标的 Rust

通过 rustup 安装 Rust，然后添加 **GNU** 目标。这一步至关重要 —— 见下方说明。

```bash
rustup target add x86_64-pc-windows-gnu
```

验证：

```bash
rustup target list --installed
# 必须包含：x86_64-pc-windows-gnu
```

> **为什么是 GNU？**
> Windows 上的 Rust 工具链默认使用 MSVC ABI（`x86_64-pc-windows-msvc`）。为 MSVC 编译的静态库**无法**被 MinGW 的 g++ 链接。因此 cargo 必须交叉编译到 `x86_64-pc-windows-gnu`，生成 MinGW 可以链接的 `.a` 文件。如果缺少该目标，CMake 步骤将因链接错误而失败。

---

## 3. 获取代码

```bash
git clone https://github.com/Breamit/MoYu-lang.git MoYu
cd MoYu
```

目录结构：

```
MoYu/
├── CMakeLists.txt          # 构建入口
├── src/                    # C++ 解释器
├── rust/
│   ├── fs_syscall/         # Rust：文件系统系统调用
│   ├── os_syscall/         # Rust：操作系统系统调用
│   └── time_syscall/       # Rust：时间系统调用
├── src/lib/                # MoYu 标准库（.my 模块）
├── docs/                   # 文档
└── test/                   # 集成测试
```

### 关于 Rust 库：只提供源码，不预编译二进制

MoYu 的 Rust 部分**只以源码形式分发**。我们不会向仓库或发布页上传预编译的二进制产物（`.a` 静态库或 `.dll` 文件）。

- 仓库中包含的是 `rust/fs_syscall/src/*.rs`、`rust/os_syscall/src/*.rs`、`rust/time_syscall/src/*.rs` —— 即 Rust 源码。
- 编译出的静态库（`libfs_syscall.a`、`libos_syscall.a`、`libtime_syscall.a`）只存在于你本地构建后的 `target/` 目录中，不属于仓库（通常已被 git 忽略）。

这是一个有意为之的选择：

1. **可复现性** —— 每个用户都从同一份源码构建，完全相同的 Rust 代码产出相同的二进制。不存在"信任上传的 .a"的问题。
2. **ABI 可移植性** —— GNU 的 `.a` 必须为 `x86_64-pc-windows-gnu` 构建。预编译的二进制绑定在某个工具链上，可能在其他机器的 MinGW 上悄悄出错。本地构建能保证 ABI 匹配。
3. **仓库更小** —— 静态库有几十 MB，而源码只有几 KB。

构建步骤（第 4 节）会通过 CMake 自动执行 `cargo build`，除非你要修改 Rust 代码，否则完全不用手动碰 Rust 工具链。唯一的前置要求是第 2.3 步中的 `x86_64-pc-windows-gnu` 目标。

---

## 4. 构建

在项目根目录下执行：

```bash
# 1. 使用 MinGW 生成器配置
cmake -G "MinGW Makefiles" -S . -B build

# 2. 构建全部内容
cmake --build build
```

首次构建会比较慢，因为 cargo 需要从头编译三个 Rust 库。后续构建是增量式的。

### CMake 做了什么

1. 检测 Rust 库（`libfs_syscall.a`、`libos_syscall.a`、`libtime_syscall.a`）。
2. 对每个库执行 `cargo build --release --target x86_64-pc-windows-gnu`，**仅当** Rust 源码发生变化时才重新编译。
3. 将它们与 C++ 解释器一起静态链接到 `moyu.exe` 中。

### 生成的二进制文件

在 `build/` 目录下：

| 二进制文件 | 说明 |
|---|---|
| `moyu.exe` | 解释器（REPL + 单文件运行）— 这是你要分发的文件 |
| `my_lexer_test.exe` | 词法分析器单元测试 |
| `my_parser_test.exe` | 解析器单元测试 |
| `my_repl_test.exe` | REPL 测试入口 |
| `my_json_test.exe` | JSON 解析器测试 |

`moyu.exe` 是**自包含的** —— 不需要附带任何 DLL。

---

## 5. 运行

```bash
./build/moyu.exe               # 交互式 REPL
./build/moyu.exe run x.my      # 运行脚本文件
./build/moyu.exe --version     # 打印版本
./build/moyu.exe -v            # 等同于 --version
```

### 中文 Windows 控制台中的 REPL

默认控制台代码页是 GBK（936），但 MoYu 源码和输出都是 UTF-8。在 REPL 中输入中文可能会出现乱码。解决方法：

```bash
chcp 65001                      # 将控制台切换为 UTF-8
./build/moyu.exe
```

或在 PowerShell 中：

```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
.\build\moyu.exe
```

另外，将中文字符串/路径写入以 UTF-8 编码保存的 `.my` 文件中 —— 这样在任何地方都能正常工作。

---

## 6. 冒烟测试

创建 `hello.my`：

```my
import io;

let s : string = "hello";
io.println(s);
io.println(s.size());
```

运行它：

```bash
./build/moyu.exe run hello.my
# hello
# 5
```

---

## 7. 如果构建失败

### `undefined reference to 'recv'` / `WSASend` / `WSAGetLastError`

Rust 标准库依赖于 Windows 系统库，而 MinGW 不会自动添加这些库。CMake 会显式链接 `ws2_32 advapi32 userenv bcrypt ntdll`。如果你仍然看到这些错误，你可能是在用 MSVC 目标构建 —— 请重新检查第 2.3 步。

### `cannot open output file moyu.exe: Permission denied`

`moyu.exe` 正在某处运行（REPL 或测试进程）。关闭它，或：

```bash
rm -f build/moyu.exe
cmake --build build
```

### 即使没有改动，`cargo build` 每次都运行

CMake 将 Rust 源文件（`.rs`、`Cargo.toml`、`Cargo.lock`）作为依赖项进行追踪。如果 `Cargo.toml` 的修改时间发生了变化（例如通过 git 操作），cargo 会重新运行。这是正常且无害的 —— cargo 本身是增量式的。

---

## 8. 测试

集成测试位于 `test/` 目录下：

```bash
./build/moyu.exe run ./test/test.my            # 端到端示例
./build/moyu.exe run ./test/typecheck_ok.my    # 正确代码，期望 0 个错误
./build/moyu.exe run ./test/typecheck_bad.my   # 错误代码，期望有错误
./build/moyu.exe run ./test/methods_test.my    # list/string/map 方法
./build/moyu.exe run ./test/platform_test.my   # @platform 过滤
```

---

## 9. 已知限制（beta 版）

- **类型检查器错误** 目前报告时不包含行号/列号位置信息。
- **自定义模块**（用户 `import` 非内置 `.my` 文件）可以加载和运行，但其导出的类型尚未经过类型检查。
- **`@platform`** 过滤发生在编译期 —— 其他平台的代码在类型检查之前就被移除了，这是有意设计的。
