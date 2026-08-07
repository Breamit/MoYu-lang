#ifndef __SYNC_REPL_H__
#define __SYNC_REPL_H__
#include <src/type.h>

// 同步 REPL 入口：单线程递归求值 eval，用于命令行交互
// 跨行保持同一 SyncRuntime 实例（globalEnv 累积）
// projectRoot：项目根；空 = 当前 cwd
// 返回进程退出码
int syncReplMain(const String& projectRoot = "");

#endif
