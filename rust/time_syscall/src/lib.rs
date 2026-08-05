use std::ffi::CString;
use std::os::raw::c_char;
use std::ptr;
use std::time::{SystemTime, UNIX_EPOCH};

// 将 Rust 字符串转为 C 字符串（C++ 侧用完调 time_free 释放）
fn to_cstring(s: &str) -> *mut c_char {
    match CString::new(s) {
        Ok(cs) => cs.into_raw(),
        Err(_) => ptr::null_mut(),
    }
}

// unix 秒戳（u64，1970-01-01 起）
#[unsafe(no_mangle)]
pub extern "C" fn time_now() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_secs())
        .unwrap_or(0)
}

// unix 秒 → UTC 日历日期（Howard Hinnant 的 civil_from_days 算法）
fn civil_from_days(z: i64) -> (i64, i64, i64) {
    let z = z + 719468;
    let era = z.div_euclid(146097);
    let doe = z.rem_euclid(146097);
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let y = yoe + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = doy - (153 * mp + 2) / 5 + 1;
    let m = if mp < 10 { mp + 3 } else { mp - 9 };
    let y = if m <= 2 { y + 1 } else { y };
    (y, m, d)
}

// 当前时间格式化为 "YYYY-MM-DDTHH:MM:SS"（UTC）
fn format_now() -> String {
    let secs = time_now() as i64;
    let days = secs.div_euclid(86400);
    let tod = secs.rem_euclid(86400);
    let (y, m, d) = civil_from_days(days);
    let hh = tod / 3600;
    let mm = (tod % 3600) / 60;
    let ss = tod % 60;
    format!("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}", y, m, d, hh, mm, ss)
}

// 当前时间格式化字符串，失败 → null
#[unsafe(no_mangle)]
pub extern "C" fn time_nowf() -> *mut c_char {
    to_cstring(&format_now())
}

// 释放 Rust 分配的 C 字符串（配套 time_nowf）
#[unsafe(no_mangle)]
pub extern "C" fn time_free(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe { let _ = CString::from_raw(ptr); };
    }
}
