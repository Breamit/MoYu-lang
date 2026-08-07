#ifndef __CONTAINER_BUILTIN_H__
#define __CONTAINER_BUILTIN_H__

#include <src/type.h>
#include <src/object/object.hpp>

// ====================================================================
// 容器/内置类型成员方法（list / string / map 的 .size() .get() ...）
//
// 不被注册到全局 env —— 只由 evalMemberAccessExpression 分发调用。
// receiver 会被原地修改（push/set 等）；越界/缺键/未知方法返回 ErrorObject。
// ====================================================================

// 统一分发入口
SPtr<Object> callContainerMethod(SPtr<Object> receiver,
                                 const String& methodName,
                                 const Array<SPtr<Object>>& args);

// 三个类型各自的 dispatch
SPtr<Object> dispatchListMethod(SPtr<Object> receiver,
                                const String& methodName,
                                const Array<SPtr<Object>>& args);
SPtr<Object> dispatchStringMethod(SPtr<Object> receiver,
                                  const String& methodName,
                                  const Array<SPtr<Object>>& args);
SPtr<Object> dispatchMapMethod(SPtr<Object> receiver,
                               const String& methodName,
                               const Array<SPtr<Object>>& args);

// ==================== list 方法实现 ====================
SPtr<Object> listMethodOfSize(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> listMethodOfGet(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> listMethodOfSet(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> listMethodOfPush(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> listMethodOfPop(SPtr<Object> receiver, const Array<SPtr<Object>>& args);

// ==================== string 方法实现 ====================
SPtr<Object> stringMethodOfSize(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> stringMethodOfSubstring(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> stringMethodOfContains(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> stringMethodOfCount(SPtr<Object> receiver, const Array<SPtr<Object>>& args);

// ==================== map 方法实现 ====================
SPtr<Object> mapMethodOfSize(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> mapMethodOfGet(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> mapMethodOfSet(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> mapMethodOfContains(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> mapMethodOfKeys(SPtr<Object> receiver, const Array<SPtr<Object>>& args);
SPtr<Object> mapMethodOfValues(SPtr<Object> receiver, const Array<SPtr<Object>>& args);

#endif
