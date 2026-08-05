#include <iostream>
#include <src/type.h>
#include <filesystem>
#include <fstream>
#include <src/interpreter/sync.h>
#include <src/interpreter/repl.h>
#include <sstream>

int main(i32 argc, char *argv[]) {
    namespace fs = std::filesystem;
    // 第一个是程序名字，我们只接受2/3个参数
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    // 项目根（标准库 src/lib 的基准）：优先用 exe 所在目录的父目录
    // （build/moyu.exe → 项目根），这样从任意目录启动都能找到标准库
    // exe 被单独拷走（无 src/lib）时 fallback 到当前 cwd
    auto projRoot = fs::absolute(fs::path(argv[0]).parent_path()).parent_path();
    if(!fs::exists(projRoot / "src" / "lib")) {
        projRoot = fs::current_path();
    }

    if(argc == 1) {
        // 无参数 → 进入交互式 REPL
        return syncReplMain(projRoot.string());
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
                // 运行文件：脚本里的相对路径以脚本所在目录为基准
                // 先把 cwd 切到脚本目录（fs.readText("./x.txt") 相对脚本位置）
                // 模块文件 src/lib/ 仍以项目根（原 cwd）为基准，传给 runtime
                auto scriptAbs = fs::absolute(p2);
                fs::current_path(scriptAbs.parent_path());

                std::ifstream file(scriptAbs);   // 用绝对路径，chdir 后依然可读
                if(!file.is_open()) {
                    std::cerr<<"file open failed: "<<scriptAbs<<std::endl;
                    return 1;
                }
                std::stringstream ss;
                ss << file.rdbuf();
                String source = ss.str();
                file.close();
                // 运行文件，返回退出码（0 成功 / 非 0 出错）
                SyncRuntime runtime(projRoot.string());
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