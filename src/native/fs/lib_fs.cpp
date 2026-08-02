#include "lib_fs.h"
#include <sstream>

// 从结果 JSON 提取 data 字段（{"ok":true,"data":"..."}）
static String extractJsonStr(const String& json, const String& key) {
    String needle = "\"" + key + "\":\"";
    size_t start = json.find(needle);
    if(start == String::npos) return "";
    start += needle.size();
    size_t end = json.find('"', start);
    if(end == String::npos) return "";
    return json.substr(start, end - start);
}

// fs.readText(path) —— 参数校验 + 转发给 Rust
SPtr<Object> fsReadText(const Array<SPtr<Object>>& args) {
    // ========== 1. 参数校验（C++ 层职责） ==========
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("fs.readText expects 1 arg");
    }
    if(args[0]->type() != ObjectType::STRING) {
        return std::make_shared<ErrorObject>("fs.readText needs a string path");
    }
    String path = static_cast<StringObject*>(args[0].get())->value;

    // ========== 2. 构造请求 JSON ==========
    // 将来用 std::filesystem 解析出绝对路径再拼进去
    String req = "{\"op\":\"readText\",\"path\":\"" + path + "\"}";

    // ========== 3. 转发给 Rust ==========
    char* res = fs_call(req.c_str());
    String result = res ? res : "";
    fs_free(res);   // 释放 Rust 分配的内存

    // ========== 4. 解析结果 → Object ==========
    if(result.find("\"ok\":true") != String::npos) {
        // 成功：提取 data → StringObject
        String content = extractJsonStr(result, "data");
        return std::make_shared<StringObject>(content);
    }
    // 失败：提取 error → ErrorObject
    String err = extractJsonStr(result, "error");
    return std::make_shared<ErrorObject>("fs.readText: " + err);
}