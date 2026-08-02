#include "repl.h"
#include "sync.h"
#include <iostream>
#include <string>

// 同步 REPL 主循环
// 流程：读一行 → SyncRuntime.run 求值 → 打印结果 → 继续
// 环境跨行保持：整个 REPL 共享一个 SyncRuntime 实例，globalEnv 累积
int syncReplMain() {
    SyncRuntime rt;   // 只建一次，globalEnv 跨行累积

    std::string line;
    while(true) {
        // 打印提示符
        std::cout << "es> " << std::flush;

        // 读一行；EOF 或出错退出
        if(!std::getline(std::cin, line)) {
            break;
        }

        // 空行跳过
        if(line.empty()) {
            continue;
        }

        // 退出命令
        if(line == "exit" || line == "quit") {
            break;
        }

        // 求值并打印结果
        String res;
        rt.run(line, res);
        std::cout << res << std::endl;
    }

    return 0;
}
