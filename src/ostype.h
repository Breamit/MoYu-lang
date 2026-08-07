#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

// 获取操作系统类型，供platform注解使用

enum class OsType {
    WINDOWS = 0,
    LINUX,
    MACOS,
    UNKNOW
};

inline OsType GetCurrentOsType() {
#if defined(_WIN32)
    return OsType::WINDOWS;
#elif defined(__linux__)
    return OsType::LINUX;
#elif defined(__APPLE__) && defined(__MACH__)
    return OsType::MACOS;
#else
    return OsType::UNKNOW;
#endif
}

#endif