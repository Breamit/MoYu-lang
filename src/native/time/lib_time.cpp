#include "lib_time.h"

// Rust 侧导出（rust/time_syscall/src/lib.rs），staticlib 静态链接进可执行文件
extern "C" {
    u64   time_now();
    char* time_nowf();   // 成功 → 格式化时间，失败 → null；用完 time_free
    void  time_free(char* ptr);
}

SPtr<Object> timeNow(const Array<SPtr<Object>> &args) {
    (void)args;
    return std::make_shared<U64Object>(time_now());
}

SPtr<Object> timeNowf(const Array<SPtr<Object>> &args) {
    (void)args;
    char* res = time_nowf();
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("time.nowf: failed");
    }
    String s(res);
    time_free(res);
    return std::make_shared<StringObject>(std::move(s));
}
