use std::ffi::{CStr, CString};
use std::fs;
use std::os::raw::{c_char, c_int};
use std::path::Path;
use std::ptr;
use std::time::UNIX_EPOCH;

// 将 Rust 字符串转为 C 字符串（C++ 侧用完调 fs_free 释放）
fn to_cstring(s: &str) -> *mut c_char {
    match CString::new(s) {
        Ok(cs) => cs.into_raw(),
        // 内容含 '\0'，C 字符串无法表示
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

// 将 SystemTime 转成 unix 秒字符串（如 "1722840000"）
fn time_to_string(m: fs::Metadata) -> Option<String> {
    let t = m.modified().ok()?.duration_since(UNIX_EPOCH).ok()?;
    Some(t.as_secs().to_string())
}

// 读文本文件：成功 -> 内容，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_read_text(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::read_to_string(path) {
        Ok(content) => to_cstring(&content),
        Err(_) => ptr::null_mut(),
    }
}

// 写文本文件：成功 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_write_text(path: *const c_char, text: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    let Some(text) = (unsafe { cstr_to_str(text) }) else {
        return ptr::null_mut();
    };
    match fs::write(path, text) {
        Ok(_) => to_cstring(""),
        Err(_) => ptr::null_mut(),
    }
}

// 追加文本：成功 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_append_text(path: *const c_char, text: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    let Some(text) = (unsafe { cstr_to_str(text) }) else {
        return ptr::null_mut();
    };
    use std::io::Write;
    match fs::OpenOptions::new().append(true).open(path).and_then(|mut f| f.write_all(text.as_bytes())) {
        Ok(_) => to_cstring(""),
        Err(_) => ptr::null_mut(),
    }
}

// 路径是否存在：1/0
#[unsafe(no_mangle)]
pub extern "C" fn fs_exists(path: *const c_char) -> c_int {
    match unsafe { cstr_to_str(path) } {
        Some(p) => Path::new(p).exists() as c_int,
        None => 0,
    }
}

// 是否目录：1/0
#[unsafe(no_mangle)]
pub extern "C" fn fs_is_directory(path: *const c_char) -> c_int {
    match unsafe { cstr_to_str(path) } {
        Some(p) => fs::metadata(p).map(|m| m.is_dir()).unwrap_or(false) as c_int,
        None => 0,
    }
}

// 是否绝对路径：1/0
#[unsafe(no_mangle)]
pub extern "C" fn fs_is_absolute(path: *const c_char) -> c_int {
    match unsafe { cstr_to_str(path) } {
        Some(p) => Path::new(p).is_absolute() as c_int,
        None => 0,
    }
}

// 创建目录（递归）：成功 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_mkdir(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::create_dir_all(path) {
        Ok(_) => to_cstring(""),
        Err(_) => ptr::null_mut(),
    }
}

// 删除目录：成功 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_rmdir(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::remove_dir_all(path) {
        Ok(_) => to_cstring(""),
        Err(_) => ptr::null_mut(),
    }
}

// 删除文件：成功 -> 空串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_remove(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::remove_file(path) {
        Ok(_) => to_cstring(""),
        Err(_) => ptr::null_mut(),
    }
}

// 创建时间：成功 -> unix 秒字符串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_create_time_of(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::metadata(path) {
        Ok(m) => match time_to_string(m) {
            Some(s) => to_cstring(&s),
            None => ptr::null_mut(),
        },
        Err(_) => ptr::null_mut(),
    }
}

// 修改时间：成功 -> unix 秒字符串，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_modify_time_of(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::metadata(path) {
        Ok(m) => match time_to_string(m) {
            Some(s) => to_cstring(&s),
            None => ptr::null_mut(),
        },
        Err(_) => ptr::null_mut(),
    }
}

// 绝对路径：成功 -> 规范化绝对路径，失败 -> null
#[unsafe(no_mangle)]
pub extern "C" fn fs_abspath_of(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    match fs::canonicalize(path) {
        Ok(p) => to_cstring(&p.to_string_lossy()),
        Err(_) => ptr::null_mut(),
    }
}

// 获取文件大小：成功返回字节数，失败返回 u64::MAX（哨兵，C++ 侧检查）
#[unsafe(no_mangle)]
pub extern "C" fn fs_size_of(path: *const c_char) -> u64 {
    match unsafe { cstr_to_str(path) } {
        Some(p) => match fs::metadata(p) {
            Ok(m) => m.len(),
            Err(_) => u64::MAX,
        },
        None => u64::MAX,
    }
}

// 列出目录下所有条目：返回 JSON 对象 {"名字":"file","名字2":"dir",...}，失败 -> null
// 键做 JSON 转义（文件名可能含引号/反斜杠），C++ 侧用项目 JSON 解析器还原
fn json_escape(s: &str) -> String {
    let mut out = String::new();
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            '\u{08}' => out.push_str("\\b"),
            '\u{0c}' => out.push_str("\\f"),
            c => out.push(c),
        }
    }
    out
}

#[unsafe(no_mangle)]
pub extern "C" fn fs_list_dir(path: *const c_char) -> *mut c_char {
    let Some(path) = (unsafe { cstr_to_str(path) }) else {
        return ptr::null_mut();
    };
    let mut items: Vec<String> = Vec::new();
    match fs::read_dir(path) {
        Ok(entries) => {
            for entry in entries.flatten() {
                let is_dir = entry.file_type().map(|t| t.is_dir()).unwrap_or(false);
                let kind = if is_dir { "dir" } else { "file" };
                let name = json_escape(&entry.file_name().to_string_lossy());
                items.push(format!("\"{}\":\"{}\"", name, kind));
            }
        }
        Err(_) => return ptr::null_mut(),
    }
    to_cstring(&format!("{{{}}}", items.join(",")))
}

// 释放 Rust 分配的 C 字符串
#[unsafe(no_mangle)]
pub extern "C" fn fs_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe { let _ = CString::from_raw(ptr); };
    }
}
