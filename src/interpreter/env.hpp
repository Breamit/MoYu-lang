#ifndef __ENV_HPP__
#define __ENV_HPP__

#include <src/type.h>
#include <src/marco.h>
#include <src/object/object.hpp>
#include <sstream>

// 栈作用域，Environment 是用来存储运行时的变量的
class Environment {
private:
    SPtr<Environment> outer_;
    Map<String,SPtr<Object>> store_;
public:
    Environment(): outer_(nullptr) {}
    explicit Environment(const SPtr<Environment>& parent): outer_(parent) {}
    ~Environment() = default;

    void set(const String& key , SPtr<Object> value) {
        store_[key] = std::move(value);
    }
    SPtr<Object> get(const String& key) {
        auto it = store_.find(key);
        if(it != store_.end()) {
            return it->second;
        }
        if(outer_ != nullptr) {
            // 该层没找到就向上查找
            return outer_->get(key);
        }
        std::stringstream ss;
        ss<<key<<" is not define or declare before";
        return std::make_shared<ErrorObject>(ss.str());
    }
};

// 类型注册，和变量隔离开
class TypeEnvironment {
private:
    Map<String,SPtr<Object>> store_;                       // 类型名 -> 类型定义对象
    Map<String,Map<String,SPtr<Object>>> methods_;         // 类型名 -> (方法名 -> 方法对象)
public:
    // 注册类型定义
    void set(const String& key, SPtr<Object> value) {
        store_[key] = std::move(value);
    }
    // 按类型名查类型定义，未注册返回 nullptr
    SPtr<Object> get(const String& key) {
        auto it = store_.find(key);
        if (it != store_.end()) return it->second;
        return nullptr;
    }
    // 注册方法到指定类型的方法表
    void setMethod(const String& structName, const String& methodName, SPtr<Object> fnObj) {
        methods_[structName][methodName] = std::move(fnObj);
    }
    // 按类型名+方法名查方法，不存在返回 nullptr
    SPtr<Object> getMethod(const String& structName, const String& methodName) {
        auto it = methods_.find(structName);
        if (it == methods_.end()) return nullptr;
        auto mit = it->second.find(methodName);
        if (mit == it->second.end()) return nullptr;
        return mit->second;
    }
};

#endif