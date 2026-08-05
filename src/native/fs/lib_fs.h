#ifndef __LIB_FS_H__
#define __LIB_FS_H__
#include <src/type.h>
#include <src/marco.h>
#include <src/object/object.hpp>

// 文件系统模块：Rust syscall 桥（staticlib 静态链接）
// 约定：Rust 返回 char* 时 —— null = 失败；成功时 C++ 用完必须调 fs_free 释放
//       Rust 返回 c_int 时 —— 1 = true，0 = false

// 读文本文件
SPtr<Object> fsReadText(const Array<SPtr<Object>>& args);

// 写文本文件
SPtr<Object> fsWriteText(const Array<SPtr<Object>>& args);

// 追加文本
SPtr<Object> fsAppendText(const Array<SPtr<Object>>& args);

// 是否存在
SPtr<Object> fsExists(const Array<SPtr<Object>>& args);

// 是否目录
SPtr<Object> fsIsDirectory(const Array<SPtr<Object>>& args);

// 是否绝对路径
SPtr<Object> fsIsAbsolute(const Array<SPtr<Object>>& args);

// 创建目录
SPtr<Object> fsMkdir(const Array<SPtr<Object>>& args);

// 删除目录
SPtr<Object> fsRmdir(const Array<SPtr<Object>>& args);

// 删除文件
SPtr<Object> fsRemove(const Array<SPtr<Object>>& args);

// 创建时间
SPtr<Object> fsCreateTimeOf(const Array<SPtr<Object>>& args);

// 修改时间
SPtr<Object> fsModifyTimeOf(const Array<SPtr<Object>>& args);

// 绝对路径
SPtr<Object> fsAbspathOf(const Array<SPtr<Object>>& args);

// 文件大小（u64）
SPtr<Object> fsSizeOf(const Array<SPtr<Object>>& args);

// 列出目录下所有条目（name -> "file"/"dir"）
SPtr<Object> fsListDir(const Array<SPtr<Object>>& args);

#endif
