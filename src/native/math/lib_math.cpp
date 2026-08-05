#include "lib_math.h"
#include <cmath>
#include <src/object/object_ops.h>

SPtr<Object> mathAbsf(const Array<SPtr<Object>> &args) {
    // 参数校验
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.absf: expect a float");
    }

    auto tp = args[0]->type();
    // 如果不是浮点数，报错
    if(!objectIn(tp,{ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("math.absf: expect a float");
    }

    // 获取高位浮点数
    f64 val;
    if(tp == ObjectType::F32) {
        auto var = static_cast<F32Object*>(args[0].get());
        val = var->value;
    } else {
        auto var = static_cast<F64Object*>(args[0].get());
        val = var->value;
    }

    if(val < 0) {
        return std::make_shared<ErrorObject>("math.absf: expect a non-negative float");
    }
    // 计算绝对值
    val = std::fabs(val);

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathAbs(const Array<SPtr<Object>> &args) {
    // 参数校验
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.abs: expect a integer");
    }

    auto tp = args[0]->type();
    // 如果不是整数，报错
    if(!objectIn(tp,{ObjectType::I32, ObjectType::I64,ObjectType::U32,ObjectType::U64})) {
        return std::make_shared<ErrorObject>("math.abs: expect a integer");
    }

    // 获取大整数
    i64 val;
    switch(tp) {
        case ObjectType::I32:
            val = static_cast<I32Object*>(args[0].get())->value;
            break;
        case ObjectType::I64:
            val = static_cast<I64Object*>(args[0].get())->value;
            break;
        case ObjectType::U32:
            val = static_cast<U32Object*>(args[0].get())->value;
            break;
        case ObjectType::U64:
            val = static_cast<U64Object*>(args[0].get())->value;
            break;
        default:
            return std::make_shared<ErrorObject>("math.abs: expect a integer");
            break;
    }
    if(val < 0) {
        return std::make_shared<ErrorObject>("math.abs: expect a non-negative integer");
    }

    // 计算绝对值
    val = std::abs(val);

    // 返回结果
    return std::make_shared<U64Object>(val);
}

SPtr<Object> mathSign(const Array<SPtr<Object>> &args) {
    // 参数校验
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.sign: expect a float");
    }

    auto tp = args[0]->type();
    // 如果不是数字，报错
    if(!objectIn(tp,{ObjectType::F32, ObjectType::F64,ObjectType::I32,ObjectType::I64,ObjectType::U32,ObjectType::U64})) {
        return std::make_shared<ErrorObject>("math.sign: expect a number");
    }
    auto sign = 0;
    switch(tp) {
        case ObjectType::F32: {
            auto val = static_cast<F32Object*>(args[0].get())->value;
            sign = (val > 0) ? 1 : (val < 0) ? -1 : 0;
            break;
        }
        case ObjectType::F64: {
            auto val = static_cast<F64Object*>(args[0].get())->value;
            sign = (val > 0) ? 1 : (val < 0) ? -1 : 0;
            break;
        }
        case ObjectType::I32: {
            auto val = static_cast<I32Object*>(args[0].get())->value;
            sign = (val > 0) ? 1 : (val < 0) ? -1 : 0;
            break;
        }
        case ObjectType::I64: {
            auto val = static_cast<I64Object*>(args[0].get())->value;
            sign = (val > 0) ? 1 : (val < 0) ? -1 : 0;
            break;
        }
        case ObjectType::U32: {
            auto val = static_cast<U32Object*>(args[0].get())->value;
            sign = (val > 0) ? 1 : (val < 0) ? -1 : 0;
            break;
        }
        case ObjectType::U64: {
            auto val = static_cast<U64Object*>(args[0].get())->value;
            sign = (val > 0) ? 1 : (val < 0) ? -1 : 0;
            break;
        }
        default:
            return std::make_shared<ErrorObject>("math.sign: expect a number");
            break;
    }

    return std::make_shared<I32Object>(sign);
}

SPtr<Object> mathSqrtf(const Array<SPtr<Object>> &args) {
    // 参数校验
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.absf: expect a float");
    }

    auto tp = args[0]->type();
    // 如果不是浮点数，报错
    if(!objectIn(tp,{ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("math.absf: expect a float");
    }

    // 获取高位浮点数
    f64 val;
    if(tp == ObjectType::F32) {
        auto var = static_cast<F32Object*>(args[0].get());
        val = var->value;
    } else {
        auto var = static_cast<F64Object*>(args[0].get());
        val = var->value;
    }

    if(val < 0) {
        return std::make_shared<ErrorObject>("math.sqrtf: expect a non-negative float");
    }
    // 计算平方根
    val = std::sqrtf(val);

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathSqrt(const Array<SPtr<Object>> &args) {
    // 参数校验
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.absf: expect a float");
    }

    auto tp = args[0]->type();
    // 如果不是整数，报错
    if(!objectIn(tp,{ObjectType::I32, ObjectType::I64,ObjectType::U32,ObjectType::U64})) {
        return std::make_shared<ErrorObject>("math.sqrt: expect a integer");
    }

    // 获取高位浮点数
    u64 val;
    if(tp == ObjectType::F32) {
        auto var = static_cast<F32Object*>(args[0].get());
        val = var->value;
    } else {
        auto var = static_cast<F64Object*>(args[0].get());
        val = var->value;
    }

    if(val < 0) {
        return std::make_shared<ErrorObject>("math.sqrt: expect a non-negative integer");
    }
    // 计算平方根
    val = std::sqrt(val);

    // 返回结果
    return std::make_shared<U64Object>(val);
}

SPtr<Object> mathPow(const Array<SPtr<Object>> &args) {
    if(args.size() != 2) {
        return std::make_shared<ErrorObject>("math.pow: expect two arguments");
    }

    f64 base = toF64(args[0]);
    f64 exp = toF64(args[1]);
    f64 val = std::pow(base, exp);

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathExp(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.exp: expect a float");
    }

    f64 val = std::exp(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathLn(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.ln: expect a float");
    }

    f64 val = std::log(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathLg(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.lg: expect a float");
    }

    f64 val = std::log10(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathSin(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.sin: expect a float");
    }

    f64 val = std::sin(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathCos(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.cos: expect a float");
    }

    f64 val = std::cos(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathTan(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.tan: expect a float");
    }

    f64 val = std::tan(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathCeil(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.ceil: expect a float");
    }

    f64 val = std::ceil(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathFloor(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.floor: expect a float");
    }

    f64 val = std::floor(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathRound(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.round: expect a float");
    }

    f64 val = std::round(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathTrunc(const Array<SPtr<Object>> &args) {
    if(args.size() != 1) {
        return std::make_shared<ErrorObject>("math.trunc: expect a float");
    }

    f64 val = std::trunc(toF64(args[0]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathMin(const Array<SPtr<Object>> &args) {
    if(args.size() != 2) {
        return std::make_shared<ErrorObject>("math.min: expect two arguments");
    }

    f64 val = std::min(toF64(args[0]), toF64(args[1]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathMax(const Array<SPtr<Object>> &args) {
    if(args.size() != 2) {
        return std::make_shared<ErrorObject>("math.max: expect two arguments");
    }

    f64 val = std::max(toF64(args[0]), toF64(args[1]));

    // 返回结果
    return std::make_shared<F64Object>(val);
}

SPtr<Object> mathClamp(const Array<SPtr<Object>> &args) {
    if(args.size() != 3) {
        return std::make_shared<ErrorObject>("math.clamp: expect three arguments");
    }
    auto val = toF64(args[0]);
    auto left = toF64(args[1]);
    auto right = toF64(args[2]);
    if(val < left) {
        return std::make_shared<F64Object>(left);
    } else if(val > right) {
        return std::make_shared<F64Object>(right);
    } else {
        return std::make_shared<F64Object>(val);
    }
}
