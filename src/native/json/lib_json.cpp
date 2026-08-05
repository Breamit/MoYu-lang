#include "lib_json.h"
#include <src/utils/json/json.h>


SPtr<Object> jsonParse(const Array<SPtr<Object>> &args) {
    // 要求接受一个字符串类型
    if(args.size() != 1 ||(args.size() > 0 && args[0]->type() != ObjectType::STRING)) {
        return std::make_shared<ErrorObject>("json.parse: expect a string");
    }
    
    // 得到字符串对象
    auto var = static_cast<StringObject*>(args[0].get());
    if(var == nullptr) {
        return std::make_shared<ErrorObject>("json.parse: expect a string");
    }

    String str = var->value;

    // 创建Json解析对象
    JSON json(str);

    auto res = json.parse();

    if(res == nullptr) {
        return std::make_shared<ErrorObject>("json.parse: parse error");
    }

    return res;

}

SPtr<Object> jsonStringify(const Array<SPtr<Object>> &args) {
    // 要求一个Map<String,any>的对象
    if(args.size() != 1 ||(args.size() > 0 && args[0]->type() != ObjectType::MAP)) {
        return std::make_shared<ErrorObject>("json.stringify: expect a map");
    }
    if(args[0] == nullptr) {
        return std::make_shared<ErrorObject>("json.stringify: expect a map");
    }
    // 创建json解析对象
    JSON json("");
    auto str = json.stringify(args[0]);
    return std::make_shared<StringObject>(str);
}
