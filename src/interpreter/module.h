#ifndef __MODULE_H__
#define __MODULE_H__

#include <src/marco.h>
#include <src/type.h>
#include <src/object/object.hpp>

class ModuleLoader {
private:
    String baseDir_;            // 项目根目录：模块文件相对路径的基准（空 = 当前 cwd）
    Set<String> loading_;
    Map<String,SPtr<ModuleObject>> cache_;
    Map<String,String> pathMap_;
    std::function<SPtr<ModuleObject>(const String& path, const String& name)> fileLoader_;
public:
    ModuleLoader(const String& baseDir = "");
    ~ModuleLoader() = default;

    void setModulePath(const String& name , const String& path);
    void setFileLoader(std::function<SPtr<ModuleObject>(const String& path, const String& name)> fn) ;
    
    
    // 加载模块
    SPtr<Object> loadModule(const String& moduleName);


private:
    SPtr<ModuleObject> loadModuleImpl(const String& moduleName);
};



#endif