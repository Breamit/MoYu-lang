cmake_minimum_required(VERSION 3.10)
project(moyu_interpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 静态链接 MinGW 运行库（libgcc/libstdc++/winpthread），产出自包含 exe
# 避免分发 exe 时还要带上 libstdc++-6.dll 等 MinGW 运行时
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic")

# ============ 主程序：MoYu 解释器（REPL + 单文件运行） ============
add_executable(moyu
    src/main.cpp
    src/marco.h
    src/type.h
    src/lexer/token.h
    src/lexer/lexer.h
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/ast/ast.hpp
    src/ast/parser.h
    src/ast/parser.cpp
    src/object/object.hpp
    src/object/object_ops.h
    src/object/object_ops.cpp
    src/interpreter/env.hpp
    src/interpreter/module.h
    src/interpreter/module.cpp
    src/interpreter/sync.h
    src/interpreter/sync.cpp
    src/interpreter/repl.h
    src/interpreter/repl.cpp
    src/native/native.h
    src/native/native.cpp
    src/native/io/lib_stdio.h
    src/native/io/lib_stdio.cpp
    src/native/fs/lib_fs.h
    src/native/fs/lib_fs.cpp
    src/native/os/lib_os.h
    src/native/os/lib_os.cpp
    src/native/json/lib_json.h
    src/native/json/lib_json.cpp
    src/native/math/lib_math.h
    src/native/math/lib_math.cpp
    src/native/time/lib_time.h
    src/native/time/lib_time.cpp
    src/builtin/builtin.hpp
    src/builtin/builtin_type.cpp
    src/builtin/container_builtin.h
    src/builtin/container_builtin.cpp
    src/utils/json/json.h
    src/utils/json/json.cpp
    src/sementic/checker/typechecker.h
    src/sementic/checker/typechecker.cpp
    src/sementic/platform/platform_filter.h
    src/sementic/platform/platform_filter.cpp
)

target_include_directories(moyu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ============ 测试目标（默认不构建，需要时取消注释） ============
# add_executable(my_lexer_test
#     src/marco.h
#     src/type.h
#     src/lexer/token.h
#     src/lexer/lexer.h
#     src/lexer/token.cpp
#     src/lexer/lexer.cpp
#     src/test/tokenizer_test.cpp
# )
#
# add_executable(my_parser_test
#     src/marco.h
#     src/type.h
#     src/lexer/token.h
#     src/lexer/lexer.h
#     src/lexer/token.cpp
#     src/lexer/lexer.cpp
#     src/ast/ast.hpp
#     src/ast/parser.h
#     src/ast/parser.cpp
#     src/test/parser_test1.cpp
# )
#
# target_include_directories(my_lexer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(my_parser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# add_executable(my_repl_test
#     src/marco.h
#     src/type.h
#     src/lexer/token.h
#     src/lexer/lexer.h
#     src/lexer/token.cpp
#     src/lexer/lexer.cpp
#     src/ast/ast.hpp
#     src/ast/parser.h
#     src/ast/parser.cpp
#     src/object/object.hpp
#     src/object/object_ops.h
#     src/object/object_ops.cpp
#     src/interpreter/env.hpp
#     src/interpreter/module.h
#     src/interpreter/module.cpp
#     src/interpreter/sync.h
#     src/interpreter/sync.cpp
#     src/interpreter/repl.h
#     src/interpreter/repl.cpp
#     src/test/sync_repl.cpp
#     src/native/native.h
#     src/native/native.cpp
#     src/native/io/lib_stdio.h
#     src/native/io/lib_stdio.cpp
#     src/native/fs/lib_fs.h
#     src/native/fs/lib_fs.cpp
#     src/native/os/lib_os.h
#     src/native/os/lib_os.cpp
#     src/native/json/lib_json.h
#     src/native/json/lib_json.cpp
#     src/native/math/lib_math.h
#     src/native/math/lib_math.cpp
#     src/native/time/lib_time.h
#     src/native/time/lib_time.cpp
#     src/builtin/builtin.hpp
#     src/builtin/builtin_type.cpp
#     src/builtin/container_builtin.h
#     src/builtin/container_builtin.cpp
#     src/utils/json/json.h
#     src/utils/json/json.cpp
#     src/sementic/checker/typechecker.h
#     src/sementic/checker/typechecker.cpp
#     src/sementic/platform/platform_filter.h
#     src/sementic/platform/platform_filter.cpp
# )
# 
# target_include_directories(my_repl_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# add_executable(my_json_test
    # src/marco.h
    # src/type.h
    # src/object/object.hpp
    # src/utils/json/json.h
    # src/utils/json/json.cpp
    # src/test/json_test.cpp
# )

# target_include_directories(my_json_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============ 平台检测（Rust target 与系统库按平台区分） ============
if(WIN32)
    # Windows：Rust 交叉编译到 GNU target，供 MinGW 链接
    set(MOYU_RUST_TARGET x86_64-pc-windows-gnu)
    set(MOYU_SYS_LIBS ws2_32 advapi32 userenv bcrypt ntdll)
elseif(UNIX AND NOT APPLE)
    # Linux：本机 gnu target；pthread/dl/m 是 Rust std 的 Linux 依赖
    set(MOYU_RUST_TARGET x86_64-unknown-linux-gnu)
    set(MOYU_SYS_LIBS pthread dl m)
else()
    message(FATAL_ERROR "Unsupported platform: only Windows and Linux are supported")
endif()

# ============ Rust syscall：cargo 构建 libfs_syscall.a 并静态链接 ============
set(FS_MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/rust/fs_syscall/Cargo.toml)
set(FS_LOCK ${CMAKE_CURRENT_SOURCE_DIR}/rust/fs_syscall/Cargo.lock)
set(FS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/rust/fs_syscall/src/lib.rs)
set(FS_STATIC ${CMAKE_CURRENT_SOURCE_DIR}/rust/fs_syscall/target/${MOYU_RUST_TARGET}/release/libfs_syscall.a)

# cargo 构建 staticlib（GNU target，与 MinGW 链接器 ABI 兼容）
# DEPENDS 必须包含 lib.rs，否则改了 Rust 源码 CMake 检测不到、不会重编（比多编一遍更坑）
add_custom_command(
    OUTPUT ${FS_STATIC}
    COMMAND cargo build --release --target ${MOYU_RUST_TARGET} --manifest-path ${FS_MANIFEST}
    DEPENDS ${FS_MANIFEST} ${FS_LOCK} ${FS_SRC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust/fs_syscall
    COMMENT "Building libfs_syscall.a (Rust staticlib)"
)
add_custom_target(fs_syscall_build DEPENDS ${FS_STATIC})

# 静态链接 Rust 库；确保链接前已构建
# ws2_32/advapi32/userenv/bcrypt/ntdll 是 Rust std 在 Windows 上的系统依赖，MinGW 不自动带
target_link_libraries(moyu PRIVATE ${FS_STATIC} ${MOYU_SYS_LIBS})
# target_link_libraries(my_repl_test PRIVATE ${FS_STATIC} ${MOYU_SYS_LIBS})
add_dependencies(moyu fs_syscall_build)
# add_dependencies(my_repl_test fs_syscall_build)

# ============ Rust os syscall：cargo 构建 libos_syscall.a 并静态链接 ============
set(OS_MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/rust/os_syscall/Cargo.toml)
set(OS_LOCK ${CMAKE_CURRENT_SOURCE_DIR}/rust/os_syscall/Cargo.lock)
set(OS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/rust/os_syscall/src/lib.rs)
set(OS_STATIC ${CMAKE_CURRENT_SOURCE_DIR}/rust/os_syscall/target/${MOYU_RUST_TARGET}/release/libos_syscall.a)

add_custom_command(
    OUTPUT ${OS_STATIC}
    COMMAND cargo build --release --target ${MOYU_RUST_TARGET} --manifest-path ${OS_MANIFEST}
    DEPENDS ${OS_MANIFEST} ${OS_LOCK} ${OS_SRC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust/os_syscall
    COMMENT "Building libos_syscall.a (Rust staticlib)"
)
add_custom_target(os_syscall_build DEPENDS ${OS_STATIC})

target_link_libraries(moyu PRIVATE ${OS_STATIC} ${MOYU_SYS_LIBS})
# target_link_libraries(my_repl_test PRIVATE ${OS_STATIC} ${MOYU_SYS_LIBS})
add_dependencies(moyu os_syscall_build)
# add_dependencies(my_repl_test os_syscall_build)

# ============ Rust time syscall：cargo 构建 libtime_syscall.a并静态链接 ============
set(TIME_MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/rust/time_syscall/Cargo.toml)
set(TIME_LOCK ${CMAKE_CURRENT_SOURCE_DIR}/rust/time_syscall/Cargo.lock)
set(TIME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/rust/time_syscall/src/lib.rs)
set(TIME_STATIC ${CMAKE_CURRENT_SOURCE_DIR}/rust/time_syscall/target/${MOYU_RUST_TARGET}/release/libtime_syscall.a)

add_custom_command(
    OUTPUT ${TIME_STATIC}
    COMMAND cargo build --release --target ${MOYU_RUST_TARGET} --manifest-path ${TIME_MANIFEST}
    DEPENDS ${TIME_MANIFEST} ${TIME_LOCK} ${TIME_SRC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust/time_syscall
    COMMENT "Building libtime_syscall.a (Rust staticlib)"
)
add_custom_target(time_syscall_build DEPENDS ${TIME_STATIC})

target_link_libraries(moyu PRIVATE ${TIME_STATIC} ${MOYU_SYS_LIBS})
# target_link_libraries(my_repl_test PRIVATE ${TIME_STATIC} ${MOYU_SYS_LIBS})
add_dependencies(moyu time_syscall_build)
# add_dependencies(my_repl_test time_syscall_build)
