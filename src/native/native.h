#ifndef __NATIVE_H__
#define __NATIVE_H__

#include <src/type.h>
#include <src/marco.h>
#include <src/object/object.hpp>
#include <functional>

// 全局 native 方法注册表
// native 方法通过 path 映射到函数包装：path -> std::function
// 例：@native('vm.file.ino')  ->  path = "vm.file.ino"
//     调用时通过 path 查表，拿到函数执行
//
// 表是全局的，放 native 文件夹下，供 runtime 调用

class NativeRegistry {
private:
    Map<String, std::function<SPtr<Object>(const Array<SPtr<Object>>&)>> store_;
public:
    NativeRegistry();

    ~NativeRegistry() = default;

    // 注册 native 方法
    void set(const String& path, std::function<SPtr<Object>(const Array<SPtr<Object>>&)> func);

    // 按 path 查 native 方法，未注册返回 nullptr
    std::function<SPtr<Object>(const Array<SPtr<Object>>&)> get(const String& path) ;
    // 是否已注册
    bool has(const String& path) ;

    void regStdLib();
};

#endif
