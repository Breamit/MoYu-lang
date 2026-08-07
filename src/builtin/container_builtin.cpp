#include "container_builtin.h"
#include <functional>

using MethodFn = std::function<SPtr<Object>(SPtr<Object>, const Array<SPtr<Object>>&)>;

namespace {
    // 从数字对象取 i64（i32/i64/u32/u64；其他类型返回 -1）
    i64 toIndex(SPtr<Object> obj) {
        switch(obj->type()) {
            case ObjectType::I32: return static_cast<I32Object*>(obj.get())->value;
            case ObjectType::U32: return static_cast<U32Object*>(obj.get())->value;
            case ObjectType::I64: return static_cast<I64Object*>(obj.get())->value;
            case ObjectType::U64: return static_cast<U64Object*>(obj.get())->value;
            default: return -1;
        }
    }
}

// ====================================================================
// list 方法
// ====================================================================
SPtr<Object> listMethodOfSize(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    (void)args;
    auto lst = static_cast<ListObject*>(receiver.get());
    return std::make_shared<U64Object>(lst->value.size());
}

SPtr<Object> listMethodOfGet(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 1) {
        return std::make_shared<ErrorObject>("list.get needs 1 arg");
    }
    auto lst = static_cast<ListObject*>(receiver.get());
    i64 idx = toIndex(args[0]);
    if(idx < 0 || (u64)idx >= lst->value.size()) {
        return std::make_shared<ErrorObject>("list.get index out of range");
    }
    return lst->value.at(idx);
}

SPtr<Object> listMethodOfSet(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 2) {
        return std::make_shared<ErrorObject>("list.set needs 2 args");
    }
    auto lst = static_cast<ListObject*>(receiver.get());
    i64 idx = toIndex(args[0]);
    if(idx < 0 || (u64)idx >= lst->value.size()) {
        return std::make_shared<ErrorObject>("list.set index out of range");
    }
    lst->value.at(idx) = args[1];
    return None;
}

SPtr<Object> listMethodOfPush(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 1) {
        return std::make_shared<ErrorObject>("list.push needs 1 arg");
    }
    auto lst = static_cast<ListObject*>(receiver.get());
    lst->value.push_back(args[0]);
    return None;
}

SPtr<Object> listMethodOfPop(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    (void)args;
    auto lst = static_cast<ListObject*>(receiver.get());
    if(lst->value.empty()) {
        return std::make_shared<ErrorObject>("list.pop on empty list");
    }
    auto v = lst->value.back();
    lst->value.pop_back();
    return v;
}

SPtr<Object> dispatchListMethod(SPtr<Object> receiver,
                                const String& methodName,
                                const Array<SPtr<Object>>& args) {
    // 方法表
    static const Map<String, MethodFn> table = {
        {"size",   listMethodOfSize},
        {"get",    listMethodOfGet},
        {"set",    listMethodOfSet},
        {"push",   listMethodOfPush},
        {"append", listMethodOfPush},   // append = push 别名
        {"pop",    listMethodOfPop},
    };
    auto it = table.find(methodName);
    if(it == table.end()) {
        return std::make_shared<ErrorObject>("list has no method " + methodName);
    }
    return it->second(receiver, args);
}

// ====================================================================
// string 方法
// ====================================================================
SPtr<Object> stringMethodOfSize(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    (void)args;
    auto str = static_cast<StringObject*>(receiver.get());
    return std::make_shared<U64Object>(str->value.size());
}

SPtr<Object> stringMethodOfSubstring(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 2) {
        return std::make_shared<ErrorObject>("string.substring needs 2 args");
    }
    auto str = static_cast<StringObject*>(receiver.get());
    i64 a = toIndex(args[0]);
    i64 b = toIndex(args[1]);
    if(a < 0 || b < a || (u64)b > str->value.size()) {
        return std::make_shared<ErrorObject>("string.substring out of range");
    }
    return std::make_shared<StringObject>(str->value.substr(a, b - a));
}

SPtr<Object> stringMethodOfContains(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 1 || args[0]->type() != ObjectType::STRING) {
        return std::make_shared<ErrorObject>("string.contains needs a string arg");
    }
    auto str = static_cast<StringObject*>(receiver.get());
    auto sub = static_cast<StringObject*>(args[0].get())->value;
    return str->value.find(sub) != String::npos ? True : False;
}

SPtr<Object> stringMethodOfCount(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 1 || args[0]->type() != ObjectType::STRING) {
        return std::make_shared<ErrorObject>("string.count needs a string arg");
    }
    auto str = static_cast<StringObject*>(receiver.get());
    auto sub = static_cast<StringObject*>(args[0].get())->value;
    if(sub.empty()) return std::make_shared<U64Object>(0);
    u64 cnt = 0;
    size_t pos = 0;
    while((pos = str->value.find(sub, pos)) != String::npos) {
        cnt++;
        pos += sub.size();
    }
    return std::make_shared<U64Object>(cnt);
}

SPtr<Object> dispatchStringMethod(SPtr<Object> receiver,
                                  const String& methodName,
                                  const Array<SPtr<Object>>& args) {
    static const Map<String, MethodFn> table = {
        {"size",      stringMethodOfSize},
        {"substring", stringMethodOfSubstring},
        {"contains",  stringMethodOfContains},
        {"count",     stringMethodOfCount},
    };
    auto it = table.find(methodName);
    if(it == table.end()) {
        return std::make_shared<ErrorObject>("string has no method " + methodName);
    }
    return it->second(receiver, args);
}

// ====================================================================
// map 方法
// ====================================================================
SPtr<Object> mapMethodOfSize(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    (void)args;
    auto mp = static_cast<MapObject*>(receiver.get());
    return std::make_shared<U64Object>(mp->value.size());
}

SPtr<Object> mapMethodOfGet(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 1) {
        return std::make_shared<ErrorObject>("map.get needs 1 arg");
    }
    auto mp = static_cast<MapObject*>(receiver.get());
    auto it = mp->value.find(args[0]);
    if(it == mp->value.end()) {
        return std::make_shared<ErrorObject>("map.get key not found");
    }
    return it->second;
}

SPtr<Object> mapMethodOfSet(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 2) {
        return std::make_shared<ErrorObject>("map.set needs 2 args");
    }
    auto mp = static_cast<MapObject*>(receiver.get());
    mp->value[args[0]] = args[1];
    return None;
}

SPtr<Object> mapMethodOfContains(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    if(args.size() < 1) {
        return std::make_shared<ErrorObject>("map.contains needs 1 arg");
    }
    auto mp = static_cast<MapObject*>(receiver.get());
    return mp->value.count(args[0]) > 0 ? True : False;
}

SPtr<Object> mapMethodOfKeys(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    (void)args;
    auto mp = static_cast<MapObject*>(receiver.get());
    auto keys = std::make_shared<ListObject>();
    for(const auto& [k, v] : mp->value) {
        keys->value.push_back(k);
    }
    return keys;
}

SPtr<Object> mapMethodOfValues(SPtr<Object> receiver, const Array<SPtr<Object>>& args) {
    (void)args;
    auto mp = static_cast<MapObject*>(receiver.get());
    auto vals = std::make_shared<ListObject>();
    for(const auto& [k, v] : mp->value) {
        vals->value.push_back(v);
    }
    return vals;
}

SPtr<Object> dispatchMapMethod(SPtr<Object> receiver,
                               const String& methodName,
                               const Array<SPtr<Object>>& args) {
    static const Map<String, MethodFn> table = {
        {"size",     mapMethodOfSize},
        {"get",      mapMethodOfGet},
        {"set",      mapMethodOfSet},
        {"contains", mapMethodOfContains},
        {"has",      mapMethodOfContains},   // has = contains 别名
        {"keys",     mapMethodOfKeys},
        {"values",   mapMethodOfValues},
    };
    auto it = table.find(methodName);
    if(it == table.end()) {
        return std::make_shared<ErrorObject>("map has no method " + methodName);
    }
    return it->second(receiver, args);
}

// ====================================================================
// 统一入口
// ====================================================================
SPtr<Object> callContainerMethod(SPtr<Object> receiver,
                                 const String& methodName,
                                 const Array<SPtr<Object>>& args) {
    if(receiver == nullptr) {
        return std::make_shared<ErrorObject>("container method on null");
    }
    switch(receiver->type()) {
        case ObjectType::LIST:   return dispatchListMethod(receiver, methodName, args);
        case ObjectType::STRING: return dispatchStringMethod(receiver, methodName, args);
        case ObjectType::MAP:    return dispatchMapMethod(receiver, methodName, args);
        default:
            return std::make_shared<ErrorObject>("container method on non-container type");
    }
}
