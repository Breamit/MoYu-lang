# MoYu Build Guide (from zero)

**- version : v0.1-beta**

This guide walks you through building MoYu from scratch on a fresh machine — installing the toolchain, cross-compiling the Rust syscall libraries, and producing the `moyu.exe` interpreter.

---

## 1. What you need

| tool | minimum version | why |
|---|---|---|
| **MinGW-w64 (g++)** | C++17 | compiles the interpreter |
| **CMake** | 3.10 | build system |
| **Rust (rustup)** | 1.80 | compiles the native syscall libraries |
| **Git** (optional) | any | clone the repo |

MoYu is built on **Windows**. The interpreter is written in C++17, and the native syscalls (fs / os / time) are Rust libraries linked statically into the executable.

---

## 2. Install the toolchain

### 2.1 MinGW-w64 (g++)

Download MinGW-w64 and put its `bin` directory on your `PATH`. Verify:

```bash
g++ --version
# g++ (MinGW-W64) 15.2.0
```

You must be able to compile C++17:

```bash
echo 'int main(){return 0;}' > /tmp/t.cpp && g++ -std=c++17 /tmp/t.cpp -o /tmp/t.exe && /tmp/t.exe && echo OK
# OK
```

### 2.2 CMake

Install CMake and verify:

```bash
cmake --version
# cmake version 3.x.x
```

### 2.3 Rust with the GNU target

Install Rust via rustup, then add the **GNU** target. This is critical — see the note below.

```bash
rustup target add x86_64-pc-windows-gnu
```

Verify:

```bash
rustup target list --installed
# must include: x86_64-pc-windows-gnu
```

> **Why GNU?**
> The Rust toolchain on Windows defaults to the MSVC ABI (`x86_64-pc-windows-msvc`). A static library compiled for MSVC **cannot** be linked by MinGW's g++. So cargo must cross-compile to `x86_64-pc-windows-gnu`, producing a `.a` that MinGW can link. If this target is missing, the CMake step will fail with linker errors.

---

## 3. Get the code

```bash
git clone <your-repo-url> MoYu
cd MoYu
```

The layout:

```
MoYu/
├── CMakeLists.txt          # build entry
├── src/                    # C++ interpreter
├── rust/
│   ├── fs_syscall/         # Rust: file system syscalls
│   ├── os_syscall/         # Rust: os syscalls
│   └── time_syscall/       # Rust: time syscalls
├── src/lib/                # MoYu standard library (.my modules)
├── docs/                   # documentation
└── test/                   # integration tests
```

### About the Rust libraries: source only, no prebuilt binaries

The Rust part of MoYu is distributed as **source code only**. We do **not** upload prebuilt binary artifacts (`.a` static libraries or `.dll` files) to the repository or release page.

- The repository contains `rust/fs_syscall/src/*.rs`, `rust/os_syscall/src/*.rs`, `rust/time_syscall/src/*.rs` — the Rust sources.
- The compiled static libraries (`libfs_syscall.a`, `libos_syscall.a`, `libtime_syscall.a`) live only in your local `target/` directory after building; they are not part of the repo (typically git-ignored).

This is a deliberate choice:

1. **Reproducibility** — every user builds from the same source, so the exact same Rust code produces the binary. No "trust the uploaded .a" problem.
2. **ABI portability** — the GNU `.a` must be built for `x86_64-pc-windows-gnu`. A prebuilt binary tied to one toolchain could silently break on another machine's MinGW. Building locally guarantees the ABI matches.
3. **Smaller repo** — static libs are tens of MB; source is a few KB.

The build step (section 4) automatically runs `cargo build` via CMake, so you never touch the Rust toolchain manually unless you modify the Rust code. The only prerequisite is the `x86_64-pc-windows-gnu` target from step 2.3.

---

## 4. Build

From the project root:

```bash
# 1. configure with the MinGW generator
cmake -G "MinGW Makefiles" -S . -B build

# 2. build everything
cmake --build build
```

The first build is slower because cargo compiles the three Rust libraries from scratch. Subsequent builds are incremental.

### What CMake does

1. Detects the Rust libraries (`libfs_syscall.a`, `libos_syscall.a`, `libtime_syscall.a`).
2. Runs `cargo build --release --target x86_64-pc-windows-gnu` for each, **only when** the Rust source changed.
3. Links them statically into `moyu.exe` along with the C++ interpreter.

### Produced binaries

In `build/`:

| binary | description |
|---|---|
| `moyu.exe` | the interpreter (REPL + single-file run) — this is what you distribute |
| `my_lexer_test.exe` | lexer unit test |
| `my_parser_test.exe` | parser unit test |
| `my_repl_test.exe` | REPL test entry |
| `my_json_test.exe` | json parser test |

`moyu.exe` is **self-contained** — no DLLs need to be shipped with it.

---

## 5. Run

```bash
./build/moyu.exe               # interactive REPL
./build/moyu.exe run x.my      # run a script file
./build/moyu.exe --version     # print version
./build/moyu.exe -v            # same as --version
```

### REPL on a Chinese Windows console

The default console code page is GBK (936), but MoYu sources and output are UTF-8. Chinese text typed in the REPL may be garbled. Fixes:

```bash
chcp 65001                      # switch the console to UTF-8
./build/moyu.exe
```

or in PowerShell:

```powershell
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
.\build\moyu.exe
```

Alternatively, write Chinese strings/paths in a `.my` file saved as UTF-8 — those work everywhere.

---

## 6. Smoke test

Create `hello.my`:

```my
import io;

let s : string = "hello";
io.println(s);
io.println(s.size());
```

Run it:

```bash
./build/moyu.exe run hello.my
# hello
# 5
```

---

## 7. If the build fails

### `undefined reference to 'recv'` / `WSASend` / `WSAGetLastError`

The Rust standard library depends on Windows system libraries that MinGW does not add automatically. CMake links `ws2_32 advapi32 userenv bcrypt ntdll` explicitly. If you still see these, you are probably building with the MSVC target — re-check step 2.3.

### `cannot open output file moyu.exe: Permission denied`

`moyu.exe` is running somewhere (a REPL or test process). Close it, or:

```bash
rm -f build/moyu.exe
cmake --build build
```

### `cargo build` runs every time even without changes

CMake tracks the Rust source files (`.rs`, `Cargo.toml`, `Cargo.lock`) as dependencies. If the mtime of `Cargo.toml` changed (e.g. by a git operation), cargo reruns. This is normal and harmless — cargo itself is incremental.

---

## 8. Tests

Integration tests live in `test/`:

```bash
./build/moyu.exe run ./test/test.my            # end-to-end sample
./build/moyu.exe run ./test/typecheck_ok.my    # correct code, expects 0 errors
./build/moyu.exe run ./test/typecheck_bad.my   # wrong code, expects errors
./build/moyu.exe run ./test/methods_test.my    # list/string/map methods
./build/moyu.exe run ./test/platform_test.my   # @platform filtering
```

---

## 9. Known limitations (beta)

- **Coroutines / `await`** are not yet implemented — the syntax is parsed but `await` evaluates synchronously (no suspension). Planned for a future version.
- **Type checker errors** are reported without line/column positions yet.
- **Custom modules** (user `import` of non-builtin `.my` files) load and run, but their exported types are not yet type-checked.
- **`@platform`** filtering happens at compile time — code for other platforms is removed before type checking, which is intentional.
