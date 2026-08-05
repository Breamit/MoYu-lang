#include "lib_os.h"

// Rust 侧导出（rust/os_syscall/src/lib.rs），staticlib 静态链接进可执行文件
// char* 型：成功 → 结果（空串也算成功），失败 → null；C++ 用完必须 os_free
// u64 / c_int 型：直接返回值
extern "C" {
    char* os_pwd();
    char* os_get_env(const char* key);
    char* os_set_env(const char* key, const char* value);
    void  os_exit(int code);
    void  os_sleep(u64 ms);
    u64   os_get_pid();
    int   os_exec(const char* cmd);
    char* os_get_os();
    void  os_free(char* ptr);
}

namespace {
    // 调 Rust char* 型函数：null → ErrorObject，否则 StringObject（并释放）
    SPtr<Object> osCallChar(char* (*fn)(), const String& fname) {
        char* res = fn();
        if(res == nullptr) {
            return std::make_shared<ErrorObject>(fname + ": failed");
        }
        String s(res);
        os_free(res);
        return std::make_shared<StringObject>(std::move(s));
    }

    // 取第 0 个参数并断言是 string，不满足返回 nullptr
    SPtr<Object> osGetStringArg(const Array<SPtr<Object>>& args, const String& fname) {
        if(args.size() < 1 || args[0]->type() != ObjectType::STRING) {
            return std::make_shared<ErrorObject>(fname + ": arg 0 expect string");
        }
        return args[0];
    }
}

SPtr<Object> osPwd(const Array<SPtr<Object>> &args)
{
    (void)args;
    return osCallChar(os_pwd, "os.pwd");
}

SPtr<Object> osGetEnv(const Array<SPtr<Object>> &args)
{
    auto a = osGetStringArg(args, "os.getEnv");
    if(a->type() == ObjectType::ERROR) return a;
    auto key = std::dynamic_pointer_cast<StringObject>(a)->value;
    char* res = os_get_env(key.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("os.getEnv: failed");
    }
    String s(res);
    os_free(res);
    return std::make_shared<StringObject>(std::move(s));
}

SPtr<Object> osSetEnv(const Array<SPtr<Object>> &args)
{
    if(args.size() < 2 || args[0]->type() != ObjectType::STRING || args[1]->type() != ObjectType::STRING) {
        return std::make_shared<ErrorObject>("os.setEnv: arg 0/1 expect string");
    }
    auto key = std::dynamic_pointer_cast<StringObject>(args[0])->value;
    auto val = std::dynamic_pointer_cast<StringObject>(args[1])->value;
    char* res = os_set_env(key.c_str(), val.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("os.setEnv: failed");
    }
    os_free(res);
    return None;
}

SPtr<Object> osExit(const Array<SPtr<Object>> &args)
{
    int code = 0;
    if(args.size() >= 1 && args[0]->type() == ObjectType::I32) {
        code = static_cast<I32Object*>(args[0].get())->value;
    }
    os_exit(code);
    return None;    // 不会到这
}

SPtr<Object> osSleep(const Array<SPtr<Object>> &args)
{
    u64 ms = 0;
    if(args.size() >= 1) {
        switch(args[0]->type()) {
            case ObjectType::I32: ms = static_cast<I32Object*>(args[0].get())->value; break;
            case ObjectType::U64: ms = static_cast<U64Object*>(args[0].get())->value; break;
            default: break;
        }
    }
    os_sleep(ms);
    return None;
}

SPtr<Object> osGetPid(const Array<SPtr<Object>> &args)
{
    (void)args;
    return std::make_shared<U64Object>(os_get_pid());
}

SPtr<Object> osExec(const Array<SPtr<Object>> &args)
{
    auto a = osGetStringArg(args, "os.exec");
    if(a->type() == ObjectType::ERROR) return a;
    auto cmd = std::dynamic_pointer_cast<StringObject>(a)->value;
    int code = os_exec(cmd.c_str());
    return std::make_shared<I32Object>(code);
}

SPtr<Object> osGetOS(const Array<SPtr<Object>> &args)
{
    (void)args;
    return osCallChar(os_get_os, "os.getOS");
}
