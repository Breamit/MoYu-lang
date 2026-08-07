#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__
#include <src/type.h>
#include <src/marco.h>
#include <sstream>
#include <src/ast/ast.hpp>
#include <functional>
enum class ObjectType {
    BOOL = 0,
    I32,
    U32,
    I64,
    U64,
    F32,
    F64,            // bool到f64的顺序绝对不可以修改
    STRING,
    MAP,
    LIST,
    ANY,
    NONE,
    NIL,        // 用 NIL 代替 NULL
    FUNCTION,   // declared function
    CONSTRUCTOR,// 构造器
    NATIVE,     // syscall
    STRUCT,     // 结构体定义对象
    RETURN,     // 返回值
    ERROR,      // error
    STRUCT_DECL,// 结构体定义
    BREAK,      // break 退出当前循环
    CONTINUE,   // continue 继续下一个循环
    MODULE,     // 模块对象，用于存储模块级的变量、方法以及对象
};



class Object;
class Environment;      // 前向声明：FunctionObject 持有 SPtr<Environment>，完整定义在 env.hpp
class I32Object;
class U32Object;
class I64Object;
class U64Object;
class F32Object;
class F64Object;
class StringObject;
class MapObject;
class ListObject;
class BoolObject;
class AnyObject;
class NilObject;
class NoneObject;
class FunctionObject;
class ConstructorObject;
class NativeObject;
class StructObject;
class ReturnObject;
class ErrorObject;
class StructDeclObject;
class BreakObject;
class ContinueObject;
class ModuleObject;


// 基类
interface Object {
public:
    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
    virtual String inspect() const = 0;
    virtual String typeInspect() const = 0;
};


// i32
class I32Object : impl Object {
public:
    i32 value;
    I32Object() = default;
    explicit I32Object(i32 v) : value(v) {}
    ObjectType type() const {
        return ObjectType::I32;
    }
    String inspect() const {
        return std::to_string(value);
    }
    String typeInspect() const {
        return "i32";
    }
    ~I32Object() = default;
};

// u32
class U32Object : impl Object {
public:
    u32 value;
    U32Object() = default;
    explicit U32Object(u32 v) : value(v) {}
    ObjectType type() const {
        return ObjectType::U32;
    }
    String inspect() const {
        return std::to_string(value);
    }
    String typeInspect() const {
        return "u32";
    }
    ~U32Object() = default;
};

// i64
class I64Object : impl Object {
public:
    i64 value;
    I64Object() = default;
    explicit I64Object(i64 v) : value(v) {}
    ObjectType type() const {
        return ObjectType::I64;
    }
    String inspect() const {
        return std::to_string(value);
    }
    String typeInspect() const {
        return "i64";
    }
    ~I64Object() = default;
};

// u64
class U64Object : impl Object {
public:
    u64 value;
    U64Object() = default;
    explicit U64Object(u64 v) : value(v) {}
    ObjectType type() const {
        return ObjectType::U64;
    }
    String inspect() const {
        return std::to_string(value);
    }
    String typeInspect() const {
        return "u64";
    }
    ~U64Object() = default;
};

// f32
class F32Object : impl Object {
public:
    f32 value;
    F32Object() = default;
    explicit F32Object(f32 v) : value(v) {}
    ObjectType type() const {
        return ObjectType::F32;
    }
    String inspect() const {
        return std::to_string(value);
    }
    String typeInspect() const {
        return "f32";
    }
    ~F32Object() = default;
};

// f64
class F64Object : impl Object {
public:
    f64 value;
    F64Object() = default;
    explicit F64Object(f64 v) : value(v) {}
    ObjectType type() const {
        return ObjectType::F64;
    }
    String inspect() const {
        return std::to_string(value);
    }
    String typeInspect() const {
        return "f64";
    }
    ~F64Object() = default;
};

// string
class StringObject : impl Object {
public:
    String value;
    StringObject() = default;
    explicit StringObject(String v) : value(std::move(v)) {}
    ObjectType type() const {
        return ObjectType::STRING;
    }
    String inspect() const {
        return value;
    }
    String typeInspect() const {
        return "string";
    }
    ~StringObject() = default;
};

// bool
class BoolObject : impl Object {
public:
    bool value;
    BoolObject() = default;
    explicit BoolObject(bool v) : value(v) {}
    ObjectType type() const {
        return ObjectType::BOOL;
    }
    String inspect() const {
        return value ? "true" : "false";
    }
    String typeInspect() const {
        return "bool";
    }
    ~BoolObject() = default;
};

// hashCode 函数
struct ObjectHash {
    size_t operator() (const std::shared_ptr<Object>& obj) const {
        switch(obj->type()) {
            case ObjectType::STRING: {
                auto s = std::dynamic_pointer_cast<StringObject>(obj);
                return std::hash<String>()(s->value);
            }
            case ObjectType::I32: {
                auto i = std::dynamic_pointer_cast<I32Object>(obj);
                return std::hash<i64>()(i->value);
            }
            case ObjectType::U32: {
                auto i = std::dynamic_pointer_cast<U32Object>(obj);
                return std::hash<u64>()(i->value);
            }
            case ObjectType::I64: {
                auto i = std::dynamic_pointer_cast<I64Object>(obj);
                return std::hash<u64>()(i->value);
            }
            case ObjectType::U64: {
                auto i = std::dynamic_pointer_cast<U64Object>(obj);
                return std::hash<u64>()(i->value);
            }
            case ObjectType::F32: {
                auto i = std::dynamic_pointer_cast<F32Object>(obj);
                return std::hash<f32>()(i->value);
            }
            case ObjectType::F64: {
                auto i = std::dynamic_pointer_cast<F64Object>(obj);
                return std::hash<i64>()(i->value);
            }
            case ObjectType::BOOL: {
                auto b = std::dynamic_pointer_cast<BoolObject>(obj);
                return std::hash<bool>()(b->value);
            }
            default:
                return 0;
        }
    }
};

// 哈希比较器
struct ObjectEqual {
    bool operator()(const std::shared_ptr<Object>& a , const std::shared_ptr<Object>& b) const {
        if(a->type() != b->type()) {
            return false;
        }        
        switch(a->type()) {
            // 只有 string 和 数字 可以做键
            case ObjectType::STRING:
                return std::dynamic_pointer_cast<StringObject>(a)->value == std::dynamic_pointer_cast<StringObject>(b)->value;
            case ObjectType::I32:
                return std::dynamic_pointer_cast<I32Object>(a)->value == std::dynamic_pointer_cast<I32Object>(b)->value;
            case ObjectType::U32:
                return std::dynamic_pointer_cast<U32Object>(a)->value == std::dynamic_pointer_cast<U32Object>(b)->value;
            case ObjectType::I64:
                return std::dynamic_pointer_cast<I64Object>(a)->value == std::dynamic_pointer_cast<I64Object>(b)->value;
            case ObjectType::U64:
                return std::dynamic_pointer_cast<U64Object>(a)->value == std::dynamic_pointer_cast<U64Object>(b)->value;    
            case ObjectType::F32:
                return std::dynamic_pointer_cast<F32Object>(a)->value == std::dynamic_pointer_cast<F32Object>(b)->value;
            case ObjectType::F64:
                return std::dynamic_pointer_cast<F64Object>(a)->value == std::dynamic_pointer_cast<F64Object>(b)->value;
            case ObjectType::BOOL:
                return std::dynamic_pointer_cast<BoolObject>(a)->value == std::dynamic_pointer_cast<BoolObject>(b)->value;
            default:
                // 指针比较兜底
                return a==b;
        }   
    }
};

// map
class MapObject : impl Object {
public:
    Map<SPtr<Object>, SPtr<Object>, ObjectHash, ObjectEqual> value;
    ObjectType type() const {
        return ObjectType::MAP;
    }
    String inspect() const {
        std::stringstream ss;
        ss << "{";
        for (auto& [key, value] : value) {
            ss << key->inspect() << ":" << value->inspect() << ", ";
        }
        ss << "}";
        return ss.str();
    }
    String typeInspect() const {
        return "map";
    }
    ~MapObject() = default;
};

// list
class ListObject : impl Object {
public:
    Array<SPtr<Object>> value;
    ListObject() = default;
    explicit ListObject(Array<SPtr<Object>> v) : value(std::move(v)) {}
    ObjectType type() const {
        return ObjectType::LIST;
    }
    String inspect() const {
        std::stringstream ss;
        ss << "[";
        for (auto& item : value) {
            ss << item->inspect() << ", ";
        }
        ss << "]";
        return ss.str();
    }
    String typeInspect() const {
        return "list";
    }
    ~ListObject() = default;
};

// any
class AnyObject : impl Object {
public:
    UPtr<Object> value;
    ObjectType type() const {
        return ObjectType::ANY;
    }
    String inspect() const {
        if(value == nullptr) {
            return "null";
        }
        return value->inspect();
    }
    String typeInspect() const {
        return value->typeInspect();
    }
    ~AnyObject() = default;
};

// null
class NilObject : impl Object {
public:
    ObjectType type() const {
        return ObjectType::NIL;
    }
    String inspect() const {
        return "null";
    }
    String typeInspect() const {
        return "nil";
    }
    ~NilObject() = default;
};

// none
class NoneObject : impl Object {
public:
    ObjectType type() const {
        return ObjectType::NONE;
    }
    String inspect() const {
        return "none";
    }
    String typeInspect() const {
        return "none";
    }
    ~NoneObject() = default;
};

// function
class FunctionObject : impl Object {
public:
    Array<String> params;               // 参数名列表（不拥有 AST，只存名字）
    SPtr<BlockStatement> body;          // 函数体（共享引用，不拥有所有权）
    SPtr<Environment> env;              // 闭包环境
    bool isAsync = false;               // @async 标记：V2.0 并发调度识别异步函数
    FunctionObject(Array<String> p, SPtr<BlockStatement> b, SPtr<Environment> e) : params(std::move(p)), body(std::move(b)), env(std::move(e)) {}
    ObjectType type() const {
        return ObjectType::FUNCTION;
    }
    String inspect() const {
        return "function(...):.. {...}";
    }
    String typeInspect() const {
        return "function";
    }
    ~FunctionObject() = default;
};

// constructor
class ConstructorObject : impl Object {
public:
    // structName
    String name;
    std::function<SPtr<Object>(const Array<SPtr<Object>>&)> func;

    explicit ConstructorObject(std::function<SPtr<Object>(const Array<SPtr<Object>>&)> f) : func(std::move(f)){}
    ObjectType type() const {
        return ObjectType::CONSTRUCTOR;
    }
    String inspect() const {
        return "Constructor Function";
    }
    String typeInspect() const {
        return "function";
    }
    ~ConstructorObject() = default;
};

// native
class NativeObject : impl Object {
public:
    String path;
    std::function<SPtr<Object>(const Array<SPtr<Object>>&)> func;
    bool isAsync = false;               // @async 标记：native 方法也可能异步
    NativeObject(String p, std::function<SPtr<Object>(const Array<SPtr<Object>>&)> f) : path(std::move(p)), func(std::move(f)){}
    ObjectType type() const {
        return ObjectType::NATIVE;
    }
    String inspect() const {
        return "Native Function";
    }
    String typeInspect() const {
        return "function";
    }
    ~NativeObject() = default;
};

// struct 实例
class StructObject : impl Object {
public:
    String structName;                          // 指向类型定义的名字（typeEnv 查表用）
    Array<Pair<String, SPtr<Object>>> fields;   // 字段名 -> 字段值
    ObjectType type() const {
        return ObjectType::STRUCT;
    }
    String inspect() const {
        std::stringstream ss;
        ss << structName << "{";
        for (size_t i = 0; i < fields.size(); i++) {
            if (i > 0) ss << ",";
            ss << fields[i].first << ":";
            if (fields[i].second) ss << fields[i].second->inspect();
            else ss << "null";
        }
        ss << "}";
        return ss.str();
    }
    String typeInspect() const {
        return structName;
    }
    ~StructObject() = default;
};

// return
class ReturnObject : impl Object {
public:
    SPtr<Object> value;
    ReturnObject() = default;
    explicit ReturnObject(SPtr<Object> v) : value(std::move(v)) {}
    ObjectType type() const {
        return ObjectType::RETURN;
    }
    String inspect() const {
        return value->inspect();
    }
    String typeInspect() const {
        return value->typeInspect();
    }
    ~ReturnObject() = default;
};

// error
class ErrorObject : impl Object {
public:
    String message;
    ObjectType type() const {
        return ObjectType::ERROR;
    }
    String inspect() const {
        return message;
    }
    ErrorObject(String msg) : message(std::move(msg)) {}
    String typeInspect() const {
        return "error";
    }
    ~ErrorObject() = default;
};

// 结构体声明
class StructDeclObject : impl Object {
public:
    String structName;                              // 类型名
    Array<Pair<String, String>> fieldsDecl;         // 字段名 -> 字段类型名（类型检查交给中间件）
    Map<String, SPtr<FunctionObject>> methods;      // 方法名 -> 方法（@method 注入）
    ObjectType type() const {
        return ObjectType::STRUCT_DECL;
    }
    String inspect() const {
        return "struct " + structName + "{...}";
    }
    String typeInspect() const {
        return structName;
    }
    ~StructDeclObject() = default;
};

// break
class BreakObject : impl Object {
public:
    ObjectType type() const {
        return ObjectType::BREAK;
    }
    String inspect() const {
        return "break";
    }
    String typeInspect() const {
        return "break";
    }
    ~BreakObject() = default;
};

// continue
class ContinueObject : impl Object {
public:
    ObjectType type() const {
        return ObjectType::CONTINUE;
    }
    String inspect() const {
        return "continue";
    }
    String typeInspect() const {
        return "continue";
    }
    ~ContinueObject() = default;
};

// module
class ModuleObject : impl Object {
public:
    String moduleName;
    Map<String, SPtr<Object>> members;
    ObjectType type() const {
        return ObjectType::MODULE;
    }
    String inspect() const {
        return "module " + moduleName;
    }
    String typeInspect() const {
        return moduleName;
    }
    ~ModuleObject() = default;
};


// 全局共享常量对象单例（inline 保证全程序唯一实例）
// 共享即不可变：这些对象仅允许读取，禁止任何地方修改其 value
inline SPtr<BoolObject> True  = std::make_shared<BoolObject>(true);
inline SPtr<BoolObject> False = std::make_shared<BoolObject>(false);
inline SPtr<NilObject>  Null  = std::make_shared<NilObject>();
inline SPtr<NoneObject> None  = std::make_shared<NoneObject>();


#endif