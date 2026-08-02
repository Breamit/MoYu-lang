#include <iostream>
#include <src/type.h>
#include <filesystem>
#include <fstream>
#include <src/runtime/sync/sync.h>
#include <src/runtime/sync/repl.h>
#include <sstream>

int main(i32 argc, char *argv[]) {
    namespace fs = std::filesystem;
    // 第一个是程序名字，我们只接受2/3个参数
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    if(argc == 1) {
        // 无参数 → 进入交互式 REPL
        return syncReplMain();
    } else if(argc == 2) {
        String p1 = argv[1];
        if(p1 == "--version" || p1 == "-v") {
            std::cout<<"MoYu 0.1 beta"<<std::endl;
        } else if(p1 == "--help" || p1 == "-h") {
            // 有一个具体文档等一下给出来
        } else {
            std::cerr<<"unknown option: "<<p1<<std::endl;
            return 1;
        }
    } else if(argc == 3) {
        String p1 = argv[1];
        String p2 = argv[2];
        if(p1 == "run") {
            // 运行文件
            if(!fs::exists(p2)) {
                std::cerr<<"file not found: "<<p2<<std::endl;
                return 1;
            }
            if(fs::is_regular_file(p2)) {
                // 运行文件
                std::ifstream file(p2);
                if(!file.is_open()) {
                    std::cerr<<"file open failed: "<<p2<<std::endl;
                    return 1;
                }
                std::stringstream ss;
                ss << file.rdbuf();
                String source = ss.str();
                file.close();
                // 运行文件，返回退出码（0 成功 / 非 0 出错）
                SyncRuntime runtime;
                String res;
                return runtime.run(source, res);
            } else {
                // waiting for realize
                return 1;
            }
        } else {
            std::cerr<<"unknown option: "<<p1<<std::endl;
            return 1;
        }
    } else {
        std::cerr<<"too many arguments"<<std::endl;
        return 1;
    }


    return 0;
}