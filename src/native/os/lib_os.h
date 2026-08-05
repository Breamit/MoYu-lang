#ifndef __LIB_OS_H__
#define __LIB_OS_H__
#include <src/type.h>
#include <src/object/object.hpp>
#include <src/marco.h>

SPtr<Object> osPwd(const Array<SPtr<Object>> &args);

SPtr<Object> osGetEnv(const Array<SPtr<Object>> &args);

SPtr<Object> osSetEnv(const Array<SPtr<Object>> &args);

SPtr<Object> osExit(const Array<SPtr<Object>> &args);

SPtr<Object> osSleep(const Array<SPtr<Object>> &args);

SPtr<Object> osGetPid(const Array<SPtr<Object>> &args);

SPtr<Object> osExec(const Array<SPtr<Object>> &args);

SPtr<Object> osGetOS(const Array<SPtr<Object>> &args);

#endif
