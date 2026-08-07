use std::env;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;
use std::time::Duration;

// 将 Rust 字符串转为 C 字符串（C++ 侧用完调 os_free 释放）
fn to_cstring(s: &str) -> *mut c_char {
    match CString::new(s) {
        Ok(cs) => cs.into_raw(),
        Err(_) => ptr::null_mut(),
    }
}

// 从 C 字符串取 &str，非法输入返回 None
unsafe fn cstr_to_str<'a>(p: *const c_char) -> Option<&'a str> {
    if p.is_null() {
        return None;
    }
    unsafe { CStr::from_ptr(p).to_str().ok() }
}

// 获取当前工作目录：成功 -> 路径，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn os_pwd() -> *mut c_char {
    match env::current_dir() {
        Ok(p) => to_cstring(&p.to_string_lossy()),
        Err(_) => ptr::null_mut(),
    }
}

// 获取环境变量：成功 -> 值，未设置 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn os_get_env(key: *const c_char) -> *mut c_char {
    let Some(key) = (unsafe { cstr_to_str(key) }) else {
        return ptr::null_mut();
    };
    match env::var(key) {
        Ok(v) => to_cstring(&v),
        Err(_) => to_cstring(""),
    }
}

// 设置环境变量：成功 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn os_set_env(key: *const c_char, value: *const c_char) -> *mut c_char {
    let Some(key) = (unsafe { cstr_to_str(key) }) else {
        return ptr::null_mut();
    };
    let Some(value) = (unsafe { cstr_to_str(value) }) else {
        return ptr::null_mut();
    };
    // set_var 直接设置，无返回值；成功恒返回空串
    // Rust 2024 中 set_var 标记为 unsafe（与 get_var 的并发语义有关）
    unsafe { env::set_var(key, value) };
    to_cstring("")
}

// 退出进程：不返回
#[unsafe(no_mangle)]
pub extern "C" fn os_exit(code: c_int) {
    std::process::exit(code);
}

// 睡眠：单位毫秒，不返回
#[unsafe(no_mangle)]
pub extern "C" fn os_sleep(ms: u64) {
    std::thread::sleep(Duration::from_millis(ms));
}

// 获取当前进程 ID
#[unsafe(no_mangle)]
pub extern "C" fn os_get_pid() -> u64 {
    std::process::id() as u64
}

// 执行命令：成功返回退出码，失败返回 -1
#[unsafe(no_mangle)]
pub extern "C" fn os_exec(cmd: *const c_char) -> c_int {
    let Some(cmd) = (unsafe { cstr_to_str(cmd) }) else {
        return -1;
    };
    match std::process::Command::new("cmd").args(["/C", cmd]).status() {
        Ok(status) => status.code().unwrap_or(-1),
        Err(_) => -1,
    }
}

// 获取操作系统类型：成功 -> "WINDOWS"/"LINUX"/"UNKNOWN"，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn os_get_os() -> *mut c_char {
    #[cfg(target_os = "windows")]
    return to_cstring("WINDOWS");
    #[cfg(target_os = "linux")]
    return to_cstring("LINUX");
    #[cfg(not(any(target_os = "windows", target_os = "linux")))]
    return to_cstring("UNKNOWN");
}

// 释放 Rust 分配的 C 字符串（配套 os_pwd/os_get_env/os_set_env/os_get_os）
#[unsafe(no_mangle)]
pub extern "C" fn os_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe { let _ = CString::from_raw(ptr); };
    }
}
