#include "native.h"
#include "io/lib_stdio.h"
#include "fs/lib_fs.h"
#include "json/lib_json.h"
#include "math/lib_math.h"
#include "os/lib_os.h"
#include "time/lib_time.h"
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
    set("std.io.print", ioPrint);
    set("std.io.println", ioPrintln);
    set("std.io.getStr", ioGetStr);
    set("std.io.getLine", ioGetLine);
    set("std.io.getFloat", ioGetFloat);
    set("std.io.getInt", ioGetInt);

    // fs
    set("std.fs.readText", fsReadText);
    set("std.fs.writeText", fsWriteText);
    set("std.fs.appendText", fsAppendText);
    set("std.fs.exists", fsExists);
    set("std.fs.isDirectory", fsIsDirectory);
    set("std.fs.isAbsolute", fsIsAbsolute);
    set("std.fs.mkdir", fsMkdir);
    set("std.fs.rmdir", fsRmdir);
    set("std.fs.remove", fsRemove);
    set("std.fs.createTimeOf", fsCreateTimeOf);
    set("std.fs.modifyTimeOf", fsModifyTimeOf);
    set("std.fs.abspathOf", fsAbspathOf);
    set("std.fs.sizeOf", fsSizeOf);
    set("std.fs.listDir", fsListDir);

    // json
    set("std.json.parse", jsonParse);
    set("std.json.stringify", jsonStringify);

    // math
    set("std.math.absf", mathAbsf);
    set("std.math.abs", mathAbs);
    set("std.math.sign", mathSign);
    set("std.math.sqrtf", mathSqrtf);
    set("std.math.sqrt", mathSqrt);
    set("std.math.pow", mathPow);
    set("std.math.exp", mathExp);
    set("std.math.ln", mathLn);
    set("std.math.lg", mathLg);
    set("std.math.sin", mathSin);
    set("std.math.cos", mathCos);
    set("std.math.tan", mathTan);
    set("std.math.ceil", mathCeil);
    set("std.math.floor", mathFloor);
    set("std.math.round", mathRound);
    set("std.math.trunc", mathTrunc);
    set("std.math.min", mathMin);
    set("std.math.max", mathMax);
    set("std.math.clamp", mathClamp);

    // os
    set("std.os.pwd", osPwd);
    set("std.os.getEnv", osGetEnv);
    set("std.os.setEnv", osSetEnv);
    set("std.os.exit", osExit);
    set("std.os.sleep", osSleep);
    set("std.os.getPid", osGetPid);
    set("std.os.exec", osExec);
    set("std.os.getOS", osGetOS);

    // time
    set("std.time.now", timeNow);
    set("std.time.nowf", timeNowf);
}
