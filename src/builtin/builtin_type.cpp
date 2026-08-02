#include "builtin_type.h"

SPtr<Object> builtinLen(const Array<SPtr<Object>> &args) {
    // 必须是 Array , Map , String这些类型
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("len() takes exactly 1 argument");
    }
    
    // 确认类型
    if(args[0] != nullptr) {
        switch(args[0]->type()) {
            case ObjectType::LIST:{
                auto arg = static_cast<ListObject*>(args[0].get());
                return std::make_shared<I32Object>(arg->value.size());
            }
            case ObjectType::STRING:{
                auto arg = static_cast<StringObject*>(args[0].get());
                return std::make_shared<I32Object>(arg->value.size());
            }
            case ObjectType::MAP:{
                auto arg = static_cast<MapObject*>(args[0].get());
                return std::make_shared<I32Object>(arg->value.size());
            }
            default:
                return std::make_shared<ErrorObject>("len() takes List , Map , String as argument");
        }
    } else {
        return std::make_shared<ErrorObject>("arguments is null");
    }

}

SPtr<Object> builtinKeys(const Array<SPtr<Object>> &args) {
    // 必须是 Map类型
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("keys() takes exactly 1 argument");
    }

    // 确定类型
    if(args[0] != nullptr && args[0]->type() == ObjectType::MAP) {
        auto lists = std::make_shared<ListObject>();
        auto mp = static_cast<MapObject*>(args[0].get());
        for(auto& [k,_] : mp->value) {
            lists->value.push_back(k);
        }
        return lists;
    } else {
        return std::make_shared<ErrorObject>("keys() takes Map as argument");
    }

}

SPtr<Object> builtinValues(const Array<SPtr<Object>> &args) {
    // 必须是 Map类型
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("keys() takes exactly 1 argument");
    }

    // 确定类型
    if(args[0] != nullptr && args[0]->type() == ObjectType::MAP) {
        auto lists = std::make_shared<ListObject>();
        auto mp = static_cast<MapObject*>(args[0].get());
        for(auto& [_,v] : mp->value) {
            lists->value.push_back(v);
        }
        return lists;
    } else {
        return std::make_shared<ErrorObject>("keys() takes Map as argument");
    }
}

SPtr<Object> builtinType(const Array<SPtr<Object>> &args) {
    // 必须是 any类型
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("type() takes exactly 1 argument");
    }
    if(args[0] == nullptr ) {
        return std::make_shared<StringObject>("none");
    } else  {
        switch(args[0]->type()) {
            case ObjectType::BOOL:
            case ObjectType::I32:
            case ObjectType::U32:
            case ObjectType::I64:
            case ObjectType::U64:
            case ObjectType::F32:
            case ObjectType::F64:
            case ObjectType::STRING:
            case ObjectType::MAP:
            case ObjectType::LIST:
            case ObjectType::NONE:
            case ObjectType::NIL:
            case ObjectType::FUNCTION:
            case ObjectType::CONSTRUCTOR:
            case ObjectType::NATIVE:
            case ObjectType::STRUCT:
            case ObjectType::ANY:
            case ObjectType::RETURN:
            case ObjectType::ERROR:
            case ObjectType::STRUCT_DECL:
            case ObjectType::MODULE:
                return std::make_shared<StringObject>(args[0]->typeInspect());
            default:
                return std::make_shared<ErrorObject>("type() can't used in key words");
        }
    }
}
