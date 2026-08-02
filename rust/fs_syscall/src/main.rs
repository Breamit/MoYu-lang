
use std::ffi::{CString,CStr};
use std::os::raw::c_char;
use std::fs;

// 将字符串转换为C字符串
fn cstring(s : &str) -> *mut c_char {
    CString::new(s).unwrap().into_raw()
}

fn extract(json: &str , key: &str)->String {
    let needle = format!("\"{}\":\"",key);
    if let Some(start) = json.find(&needle) {
        let rest = &json[start+needle.len()..];
        if let Some(end) = rest.find('"') {
            return rest[..end].to_string();
        }
    }
    String::new()
}

fn escape(s: &str) -> String {
    s.replace('\\', "\\\\")
     .replace('"', "\\\"")
     .replace('\n', "\\n")
}

// 统一入口：接收请求 JSON，返回结果 JSON
// 请求: {"op":"readText","path":"/a/b.txt"}
// 返回: {"ok":true,"data":"文件内容"}  或  {"ok":false,"error":"..."}
#[no_mangle]
pub extern "C" fn fs_call(request: *const c_char) -> *mut c_char {
    if request.is_null() {
        return cstring("{\"ok\":false,\"error\":\"null request\"}");
    }
    let req = unsafe { CStr::from_ptr(request) };
    let req_str = req.to_string_lossy().to_string();

    // 按 op 分发
    if req_str.contains("\"readText\"") {
        let path = extract(&req_str, "path");
        match fs::read_to_string(&path) {
            Ok(content) => cstring(&format!("{{\"ok\":true,\"data\":\"{}\"}}", escape(&content))),
            Err(e) => cstring(&format!("{{\"ok\":false,\"error\":\"{}\"}}", e)),
        }
    } else {
        cstring("{\"ok\":false,\"error\":\"unknown op\"}")
    }
}

// 释放 C++ 传回的字符串（Rust 分配，Rust 释放）
#[no_mangle]
pub extern "C" fn fs_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe { CString::from_raw(ptr) };
    }
}