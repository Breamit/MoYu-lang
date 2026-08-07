#include "lib_fs.h"
#include <limits>
#include <src/utils/json/json.h>

// Rust 侧导出，staticlib 静态链接进可执行文件
// char* 型：成功 -> 结果，失败 -> null；C++ 用完必须 fs_free
// c_int 型：1 / 0
extern "C" {
    char* fs_read_text(const char* path);
    char* fs_write_text(const char* path, const char* text);
    char* fs_append_text(const char* path, const char* text);
    int   fs_exists(const char* path);
    int   fs_is_directory(const char* path);
    int   fs_is_absolute(const char* path);
    char* fs_mkdir(const char* path);
    char* fs_rmdir(const char* path);
    char* fs_remove(const char* path);
    char* fs_create_time_of(const char* path);
    char* fs_modify_time_of(const char* path);
    char* fs_abspath_of(const char* path);
    u64   fs_size_of(const char* path);        
    char* fs_list_dir(const char* path);       
    void  fs_free(char* ptr);
}

namespace {
    // 取第 n 个参数并断言是 string，不满足返回 nullptr
    SPtr<Object> getStringArg(const Array<SPtr<Object>>& args, u32 idx, const String& fname) {
        if(args.size() <= idx || args[idx]->type() != ObjectType::STRING) {
            return std::make_shared<ErrorObject>(fname + ": arg " + std::to_string(idx) + " expect string");
        }
        return args[idx];
    }

    // 通用错误消息前缀
    String errPrefix(const String& fname) { return fname + ": failed"; }

    // 调 Rust char* 型函数：null -> ErrorObject，否则 StringObject
    SPtr<Object> callChar(char* (*fn)(const char*), const String& path, const String& fname) {
        char* res = fn(path.c_str());
        if(res == nullptr) {
            return std::make_shared<ErrorObject>(errPrefix(fname));
        }
        String s(res);
        fs_free(res);
        return std::make_shared<StringObject>(std::move(s));
    }
}

SPtr<Object> fsReadText(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.readText");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return callChar(fs_read_text, path, "fs.readText");
}

SPtr<Object> fsWriteText(const Array<SPtr<Object>>& args) {
    auto a0 = getStringArg(args, 0, "fs.writeText");
    if(a0->type() == ObjectType::ERROR) return a0;
    auto a1 = getStringArg(args, 1, "fs.writeText");
    if(a1->type() == ObjectType::ERROR) return a1;
    auto path = std::dynamic_pointer_cast<StringObject>(a0)->value;
    auto text = std::dynamic_pointer_cast<StringObject>(a1)->value;
    char* res = fs_write_text(path.c_str(), text.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("fs.writeText: failed");
    }
    fs_free(res);
    return None;
}

SPtr<Object> fsAppendText(const Array<SPtr<Object>>& args) {
    auto a0 = getStringArg(args, 0, "fs.appendText");
    if(a0->type() == ObjectType::ERROR) return a0;
    auto a1 = getStringArg(args, 1, "fs.appendText");
    if(a1->type() == ObjectType::ERROR) return a1;
    auto path = std::dynamic_pointer_cast<StringObject>(a0)->value;
    auto text = std::dynamic_pointer_cast<StringObject>(a1)->value;
    char* res = fs_append_text(path.c_str(), text.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("fs.appendText: failed");
    }
    fs_free(res);
    return None;
}

SPtr<Object> fsExists(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.exists");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return fs_exists(path.c_str()) ? True : False;
}

SPtr<Object> fsIsDirectory(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.isDirectory");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return fs_is_directory(path.c_str()) ? True : False;
}

SPtr<Object> fsIsAbsolute(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.isAbsolute");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return fs_is_absolute(path.c_str()) ? True : False;
}

SPtr<Object> fsMkdir(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.mkdir");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    char* res = fs_mkdir(path.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("fs.mkdir: failed");
    }
    fs_free(res);
    return None;
}

SPtr<Object> fsRmdir(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.rmdir");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    char* res = fs_rmdir(path.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("fs.rmdir: failed");
    }
    fs_free(res);
    return None;
}

SPtr<Object> fsRemove(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.remove");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    char* res = fs_remove(path.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("fs.remove: failed");
    }
    fs_free(res);
    return None;
}

SPtr<Object> fsCreateTimeOf(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.createTimeOf");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return callChar(fs_create_time_of, path, "fs.createTimeOf");
}

SPtr<Object> fsModifyTimeOf(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.modifyTimeOf");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return callChar(fs_modify_time_of, path, "fs.modifyTimeOf");
}

SPtr<Object> fsAbspathOf(const Array<SPtr<Object>>& args) {
    auto a = getStringArg(args, 0, "fs.abspathOf");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    return callChar(fs_abspath_of, path, "fs.abspathOf");
}

SPtr<Object> fsSizeOf(const Array<SPtr<Object>>& args)
{
    auto a = getStringArg(args, 0, "fs.sizeOf");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    u64 size = fs_size_of(path.c_str());
    if(size == std::numeric_limits<u64>::max()) {
        return std::make_shared<ErrorObject>("fs.sizeOf: failed");
    }
    return std::make_shared<U64Object>(size);
}

SPtr<Object> fsListDir(const Array<SPtr<Object>>& args)
{
    auto a = getStringArg(args, 0, "fs.listDir");
    if(a->type() == ObjectType::ERROR) return a;
    auto path = std::dynamic_pointer_cast<StringObject>(a)->value;
    char* res = fs_list_dir(path.c_str());
    if(res == nullptr) {
        return std::make_shared<ErrorObject>("fs.listDir: failed");
    }
    String all(res);
    fs_free(res);
    // Rust 返回 JSON 对象 {"名字":"file",...}，用项目 JSON 解析器还原成 MapObject
    JSON parser(all);
    auto obj = parser.parse();
    if(obj == nullptr || obj->type() == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("fs.listDir: bad json from rust");
    }
    return obj;
}
