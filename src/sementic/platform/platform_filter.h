#ifndef __PLATFORM_FILTER_H__
#define __PLATFORM_FILTER_H__

#include <src/marco.h>
#include <src/type.h>
#include <src/ast/ast.hpp>
#include <src/ostype.h>

// ====================================================================
// 平台过滤 pass —— 先于 typechecker 运行
//
// 处理 @platform(Windows, Linux) 注解：若当前 OS 不在声明的平台列表里，
// 则删除整个 AnnotationStatement（连同 target 函数/struct）。
// 匹配时原样保留（eval 对 @platform 本就忽略）。
//
// 纯编译期预处理，不改语义、不影响运行。
// ====================================================================

class PlatformFilter {
public:
    // 入口：遍历 program->statements，删除不匹配当前 OS 的注解节点
    static void filter(Program* program);

    // 当前 OS 名（小写）："windows" / "linux" / "macos" / "unknown"
    static String currentOsName();
};

#endif
