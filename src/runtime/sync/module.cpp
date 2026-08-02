#include "module.h"

ModuleLoader::ModuleLoader() {
    // io
    setModulePath("io", "src/lib/io.my");
}

void ModuleLoader::setModulePath(const String &name, const String &path) {
    pathMap_[name] = path;
}

void ModuleLoader::setFileLoader(std::function<SPtr<ModuleObject>(const String &path, const String &name)> fn) {
    fileLoader_ = fn;
}

SPtr<Object> ModuleLoader::loadModule(const String &moduleName)
{
    if(loading_.find(moduleName) != loading_.end()) {
        // 循环引用报错
        return std::make_shared<ErrorObject>("cycle quote");
    }

    // 缓存命中就直接返回，不重复加载
    auto it = cache_.find(moduleName);
    if(it != cache_.end()) {   
        return it->second;
    }

    // 标记正在加载
    loading_.insert(moduleName);
    SPtr<ModuleObject> mod = loadModuleImpl(moduleName);
    if(mod == nullptr) {
        // 加载失败，清除标记
        loading_.erase(moduleName);
        return std::make_shared<ErrorObject>("module not found: " + moduleName);
    }

    loading_.erase(moduleName);
    // 加入缓存
    cache_[moduleName] = mod;
    return mod;
}

SPtr<ModuleObject> ModuleLoader::loadModuleImpl(const String &moduleName) {
    String path;
    auto pit = pathMap_.find(moduleName);

    if(pit != pathMap_.end()) {
        path = pit->second;
    } else {
        path = "src/lib/" + moduleName + ".my";
    }

    if(fileLoader_) {
        return fileLoader_(path, moduleName);
    } else {
        return nullptr;
    }
}
