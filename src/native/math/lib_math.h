#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

#include <src/type.h>
#include <src/object/object.hpp>
#include <src/marco.h>

// 浮点绝对值
SPtr<Object> mathAbsf(const Array<SPtr<Object>> &args);

// 整数绝对值
SPtr<Object> mathAbs(const Array<SPtr<Object>> &args);

// 符号函数
SPtr<Object> mathSign(const Array<SPtr<Object>> &args);

// 浮点平方根
SPtr<Object> mathSqrtf(const Array<SPtr<Object>> &args);

// 整数平方根
SPtr<Object> mathSqrt(const Array<SPtr<Object>> &args);

// 底-幂
SPtr<Object> mathPow(const Array<SPtr<Object>> &args);

// e 指数
SPtr<Object> mathExp(const Array<SPtr<Object>> &args);

// 自然对数底数 e
SPtr<Object> mathLn(const Array<SPtr<Object>> &args);

// 对数底数 10
SPtr<Object> mathLg(const Array<SPtr<Object>> &args);

// 三角函数sin
SPtr<Object> mathSin(const Array<SPtr<Object>> &args);

// 三角函数cos
SPtr<Object> mathCos(const Array<SPtr<Object>> &args);

// 三角函数tan
SPtr<Object> mathTan(const Array<SPtr<Object>> &args);

// 向上取整
SPtr<Object> mathCeil(const Array<SPtr<Object>> &args);

// 向下取整
SPtr<Object> mathFloor(const Array<SPtr<Object>> &args);

// 四舍五入
SPtr<Object> mathRound(const Array<SPtr<Object>> &args);

// 截断小数
SPtr<Object> mathTrunc(const Array<SPtr<Object>> &args);

// 最小值
SPtr<Object> mathMin(const Array<SPtr<Object>> &args);

// 最大值
SPtr<Object> mathMax(const Array<SPtr<Object>> &args);

// 限制范围 [min, max]
SPtr<Object> mathClamp(const Array<SPtr<Object>> &args);

#endif
