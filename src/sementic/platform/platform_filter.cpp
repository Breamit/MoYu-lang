#include "platform_filter.h"
#include <cctype>

String PlatformFilter::currentOsName() {
    switch(GetCurrentOsType()) {
        case OsType::WINDOWS: return "windows";
        case OsType::LINUX:   return "linux";
        case OsType::MACOS:   return "macos";
        default:              return "unknown";
    }
}

void PlatformFilter::filter(Program* program) {
    if(program == nullptr) return;

    String cur = currentOsName();
    auto& stmts = program->statements;

    for(auto it = stmts.begin(); it != stmts.end();) {
        auto stmt = it->get();
        if(stmt->type() != NodeType::ANNOTATION_STATEMENT) {
            ++it;
            continue;
        }
        auto ann = static_cast<AnnotationStatement*>(stmt);

        // 找 annotations 里的 @platform
        bool hasPlatform = false;
        bool match = false;
        for(const auto& a : ann->annotations) {
            if(a->type() != NodeType::PLATFORM_ANNOTATION) continue;
            hasPlatform = true;
            auto p = static_cast<PlatformAnnotation*>(a.get());
            // 平台名比较，统一小写
            for(const auto& plat : p->platforms) {
                String name = plat->value;
                for(auto& c : name) c = std::tolower((unsigned char)c);
                if(name == cur) {
                    match = true;
                    break;
                }
            }
            break;   // 只查第一个 @platform
        }

        // 有 @platform 且不匹配 → 删整个节点
        if(hasPlatform && !match) {
            it = stmts.erase(it);
        } else {
            ++it;
        }
    }
}
