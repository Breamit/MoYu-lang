#ifndef __OBJECT_OPS_H__
#define __OBJECT_OPS_H__

#include <src/marco.h>
#include <src/type.h>
#include "./object.hpp"


// 这里主要是处理中缀表达式的运算的部分

// 需要处理的运算符号有：
// and or + - * / % & | ^ = != ==  >= <= > < 

// 对外暴露方法
SPtr<Object> objectOperators(SPtr<Object> left, SPtr<Object> right, const String& op);


// 不暴露方法：

// and
SPtr<Object> objectAnd(SPtr<Object> left, SPtr<Object> right);

// or
SPtr<Object> objectOr(SPtr<Object> left, SPtr<Object> right);

// +
SPtr<Object> objectPlus(SPtr<Object> left, SPtr<Object> right);

// -
SPtr<Object> objectMinus(SPtr<Object> left, SPtr<Object> right);

// * 
SPtr<Object> objectMulti(SPtr<Object> left, SPtr<Object> right);

// /
SPtr<Object> objectDiv(SPtr<Object> left, SPtr<Object> right);

// %
SPtr<Object> objectMod(SPtr<Object> left, SPtr<Object> right);

// & 
SPtr<Object> objectBitAnd(SPtr<Object> left, SPtr<Object> right);

// |
SPtr<Object> objectBitOr(SPtr<Object> left, SPtr<Object> right);

// ^
SPtr<Object> objectBitCaret(SPtr<Object> left, SPtr<Object> right);

// =
SPtr<Object> objectAssign(SPtr<Object> left, SPtr<Object> right);

// != 
SPtr<Object> objectNotEqual(SPtr<Object> left, SPtr<Object> right);

// == 
SPtr<Object> objectEqual(SPtr<Object> left, SPtr<Object> right);

// >= 
SPtr<Object> objectGreaterEqual(SPtr<Object> left, SPtr<Object> right);

// <= 
SPtr<Object> objectLessEqual(SPtr<Object> left, SPtr<Object> right);

// >
SPtr<Object> objectGreater(SPtr<Object> left, SPtr<Object> right);

// <
SPtr<Object> objectLess(SPtr<Object> left, SPtr<Object> right);

// 辅助函数

// 转化成i64类型
i64 toI64(SPtr<Object> num);

// 转化成u64类型
u64 toU64(SPtr<Object> num);

// 转化成f64类型
f64 toF64(SPtr<Object> num);

// 转化成bool类型（真值判断）
bool toBool(SPtr<Object> obj);

// 通过两个对象类型，返回一个需要被转化成的类型进行计算
ObjectType promote(ObjectType lt , ObjectType rt);

// 检查对象类型是否在数组中
bool objectIn(ObjectType ot , const Array<ObjectType>& ots);
#endif