#include "native.h"
#include "io/lib_stdio.h"
NativeRegistry::NativeRegistry() {
    regStdLib();
}

void NativeRegistry::set(const String &path, std::function<SPtr<Object>(const Array<SPtr<Object>> &)> func) {
    store_[path] = std::move(func);
}

std::function<SPtr<Object>(const Array<SPtr<Object>> &)> NativeRegistry::get(const String &path) {
    auto it = store_.find(path);
    if (it != store_.end()) return it->second;
    return nullptr;
}

bool NativeRegistry::has(const String &path) {
    return store_.count(path) > 0;
}

void NativeRegistry::regStdLib() {
    // stdio
    set("io.print", ioPrint);
    set("io.println", ioPrintln);
    set("io.getStr", ioGetStr);
    set("io.getLine", ioGetLine);
    set("io.getFloat", ioGetFloat);
    set("io.getInt", ioGetInt);
}
