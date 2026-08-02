#include "object_ops.h"

// 检查对象类型是否在类型数组中
bool objectIn(ObjectType ot , const Array<ObjectType>& ots) {
    for(const auto& i : ots) {
        if(i == ot) {
            return true;
        }
    }
    return false;
}

SPtr<Object> objectOperators(SPtr<Object> left, SPtr<Object> right, const String& op) {
    // dispatch
    static Map<String,u32> dispatch = {
        {"and", 0},     {"or", 1},
        {"+", 2},       {"-", 3},
        {"*", 4},       {"/", 5},
        {"%", 6},       {"&", 7}, 
        {"|", 8},       {"^", 9},
        {"=", 10},      {"!=", 11},
        {"==", 12},     {">=", 13},
        {"<=", 14},     {">", 15},
        {"<", 16},
    };
    auto it = dispatch.find(op);
    if(it == dispatch.end()) {
        return std::make_shared<ErrorObject>("operator not found");
    }
    u32 id = it->second;
    
    switch(id) {
        case 0:
            return objectAnd(left, right);
        case 1:
            return objectOr(left, right);
        case 2:
            return objectPlus(left, right);
        case 3:
            return objectMinus(left, right);
        case 4:
            return objectMulti(left, right);
        case 5:
            return objectDiv(left, right);
        case 6:
            return objectMod(left, right);  
        case 7:
            return objectBitAnd(left, right);
        case 8:
            return objectBitOr(left, right);
        case 9:
            return objectBitCaret(left, right); 
        case 10:
            return objectAssign(left, right);
        case 11:
            return objectNotEqual(left, right);
        case 12:
            return objectEqual(left, right);
        case 13:
            return objectGreaterEqual(left, right);
        case 14:
            return objectLessEqual(left, right);
        case 15:
            return objectGreater(left, right);
        case 16:
            return objectLess(left, right);
        default:
            return std::make_shared<ErrorObject>("this operator is not supported for this type");
        }
    
}

SPtr<Object> objectAnd(SPtr<Object> left, SPtr<Object> right) {
    // 逻辑与：仅支持 bool 类型
    if(left->type() != ObjectType::BOOL || right->type() != ObjectType::BOOL) {
        return std::make_shared<ErrorObject>("and operator is not supported for this type");
    }
    bool lv = static_cast<BoolObject*>(left.get())->value;
    bool rv = static_cast<BoolObject*>(right.get())->value;
    return std::make_shared<BoolObject>(lv && rv);
}

SPtr<Object> objectOr(SPtr<Object> left, SPtr<Object> right) {
    // 逻辑或：仅支持 bool 类型
    if(left->type() != ObjectType::BOOL || right->type() != ObjectType::BOOL) {
        return std::make_shared<ErrorObject>("or operator is not supported for this type");
    }
    bool lv = static_cast<BoolObject*>(left.get())->value;
    bool rv = static_cast<BoolObject*>(right.get())->value;
    return std::make_shared<BoolObject>(lv || rv);
}

SPtr<Object> objectPlus(SPtr<Object> left, SPtr<Object> right) {   
    // 支持 i32 , i64 , u32 , u64 , bool , f64, f32 之间的转换计算
    // 类型相同，直接进行加法操作
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::BOOL, ObjectType::F32, ObjectType::F64,ObjectType::STRING})) {
        return std::make_shared<ErrorObject>("+ operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::BOOL, ObjectType::F32, ObjectType::F64,ObjectType::STRING})) {
        return std::make_shared<ErrorObject>("+ operator is not supported for this type");
    }



    auto targetType = promote(lt, rt);

    // 两个bool类型不可以直接进行加法操作
    if(lt == ObjectType::BOOL && rt == ObjectType::BOOL) {
        return std::make_shared<ErrorObject>("+ operator is not supported for two bool type");
    }

    // 防止类型转换失败
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("+ operator is not supported for this type");
    }
    // 进行类型提升后计算
    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<I32Object>(toI64(left) + toI64(right));
        case ObjectType::U32:
            return std::make_shared<U32Object>(toU64(left) + toU64(right));
        case ObjectType::I64:
            return std::make_shared<I64Object>(toI64(left) + toI64(right));
        case ObjectType::U64:
            return std::make_shared<U64Object>(toU64(left) + toU64(right));
        case ObjectType::F32:
            return std::make_shared<F32Object>(toF64(left) + toF64(right));
        case ObjectType::F64:
            return std::make_shared<F64Object>(toF64(left) + toF64(right));
        case ObjectType::STRING: {
            // 左操作数必须是string才可以
            if(lt != ObjectType::STRING) {
                return std::make_shared<ErrorObject>("+ operator is not supported for this type,left operand must be string");
            } else {
                return std::make_shared<StringObject>(toString(left) + toString(right));
            }
        }
            
        default:
            return std::make_shared<ErrorObject>("+ operator is not supported for this type");
    }    
}

SPtr<Object> objectMinus(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32 , i64 , u32 , u64 , f64, f32 之间的转换计算
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("- operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("- operator is not supported for this type");
    }

    auto targetType = promote(lt, rt);


    // 防止类型转换失败
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("- operator is not supported for this type");
    }
    // 进行类型提升后计算
    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<I32Object>(toI64(left) - toI64(right));
        case ObjectType::U32:
            return std::make_shared<U32Object>(toU64(left) - toU64(right));
        case ObjectType::I64:
            return std::make_shared<I64Object>(toI64(left) - toI64(right));
        case ObjectType::U64:
            return std::make_shared<U64Object>(toU64(left) - toU64(right));
        case ObjectType::F32:
            return std::make_shared<F32Object>(toF64(left) - toF64(right));
        case ObjectType::F64:
            return std::make_shared<F64Object>(toF64(left) - toF64(right));
        default:
            return std::make_shared<ErrorObject>("- operator is not supported for this type");
    }    
}

SPtr<Object> objectMulti(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32 , i64 , u32 , u64  , f64, f32 之间的转换计算
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64,ObjectType::STRING})) {
        return std::make_shared<ErrorObject>("* operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("* operator is not supported for this type");
    }

    auto targetType = promote(lt, rt);

    // 防止类型转换失败
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("* operator is not supported for this type");
    }
    // 进行类型提升后计算
    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<I32Object>(toI64(left) * toI64(right));
        case ObjectType::U32:
            return std::make_shared<U32Object>(toU64(left) * toU64(right));
        case ObjectType::I64:
            return std::make_shared<I64Object>(toI64(left) * toI64(right));
        case ObjectType::U64:
            return std::make_shared<U64Object>(toU64(left) * toU64(right));
        case ObjectType::F32:
            return std::make_shared<F32Object>(toF64(left) * toF64(right));
        case ObjectType::F64:
            return std::make_shared<F64Object>(toF64(left) * toF64(right));
        case ObjectType::STRING: {
            // 左操作数为string，右操作数必须大于零
            if(lt != ObjectType::STRING) {
                return std::make_shared<ErrorObject>("* operator is not supported for this type,left operand must be string");
            } else {
                auto fr = toU64(right);
                if(fr < 0) {
                    return std::make_shared<ErrorObject>("* operator is not supported for this type,right operand must be positive");
                } else if(fr == 0) {
                    return std::make_shared<StringObject>("");
                } else {
                    std::stringstream ss;
                    for(u64 i = 0; i < fr; i++) {
                        ss << toString(left);
                    }
                    return std::make_shared<StringObject>(ss.str());
                }
            }
        }
        default:
            return std::make_shared<ErrorObject>("* operator is not supported for this type");
    }    
}

SPtr<Object> objectDiv(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32 , i64 , u32 , u64  , f64, f32 之间的转换计算
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("/ operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("/ operator is not supported for this type");
    }

    auto targetType = promote(lt, rt);

    if(toI64(right) == 0 || toF64(right) == 0.0) {
        // 后期接入异常处理函数，实现自定义异常处理
        return std::make_shared<ErrorObject>("/ operator is not supported for zero");
    }
    
    // 防止类型转换失败
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("/ operator is not supported for this type");
    }
    // 进行类型提升后计算
    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<I32Object>(toI64(left) / toI64(right));
        case ObjectType::U32:
            return std::make_shared<U32Object>(toU64(left) / toU64(right));
        case ObjectType::I64:
            return std::make_shared<I64Object>(toI64(left) / toI64(right));
        case ObjectType::U64:
            return std::make_shared<U64Object>(toU64(left) / toU64(right));
        case ObjectType::F32:
            return std::make_shared<F32Object>(toF64(left) / toF64(right));
        case ObjectType::F64:
            return std::make_shared<F64Object>(toF64(left) / toF64(right));
        default:
            return std::make_shared<ErrorObject>("/ operator is not supported for this type");
    }    
}

SPtr<Object> objectMod(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32 , i64 , u32 , u64  , f64, f32 之间的转换计算
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("mod operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32, ObjectType::I64, ObjectType::U32, ObjectType::U64, ObjectType::F32, ObjectType::F64})) {
        return std::make_shared<ErrorObject>("mod operator is not supported for this type");
    }

    auto targetType = promote(lt, rt);

    // 防止类型转换失败
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("mod operator is not supported for this type");
    }
    // 进行类型提升后计算
    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<I32Object>(toI64(left) % toI64(right));
        case ObjectType::U32:
            return std::make_shared<U32Object>(toU64(left) % toU64(right));
        case ObjectType::I64:
            return std::make_shared<I64Object>(toI64(left) % toI64(right));
        case ObjectType::U64:
            return std::make_shared<U64Object>(toU64(left) % toU64(right));
        case ObjectType::F32:  
        case ObjectType::F64:
            return std::make_shared<ErrorObject>("mod operator is not supported for this floating");
        default:
            return std::make_shared<ErrorObject>("mod operator is not supported for this type");
    }    
}

SPtr<Object> objectBitAnd(SPtr<Object> left, SPtr<Object> right) {
    if(left->type() != right->type()) {
        return std::make_shared<ErrorObject>("& operator is not supported for same type");
    }
    auto type = left->type();
    // 允许 i32,u32,i64,f64四种运算符操作
    switch(type) {
        case ObjectType::I32: {
            auto lv = static_cast<I32Object*>(left.get());
            auto rv = static_cast<I32Object*>(right.get());
            return std::make_shared<I32Object>(lv->value & rv->value);
}
        case ObjectType::U32: {
            auto lv = static_cast<U32Object*>(left.get());
            auto rv = static_cast<U32Object*>(right.get());
            return std::make_shared<U32Object>(lv->value & rv->value);
}
        case ObjectType::I64: {
            auto lv = static_cast<I64Object*>(left.get());
            auto rv = static_cast<I64Object*>(right.get());
            return std::make_shared<I64Object>(lv->value & rv->value);
        }
        case ObjectType::U64: {
            auto lv = static_cast<U64Object*>(left.get());
            auto rv = static_cast<U64Object*>(right.get());
            return std::make_shared<U64Object>(lv->value & rv->value);
}
        default:
            return std::make_shared<ErrorObject>("& operator is not supported for this type");
    }
}

SPtr<Object> objectBitOr(SPtr<Object> left, SPtr<Object> right) {
   if(left->type() != right->type()) {
        return std::make_shared<ErrorObject>("| operator is not supported for same type");
    }
    auto type = left->type();
    // 允许 i32,u32,i64,f64四种运算符操作
    switch(type) {
        case ObjectType::I32: {
            auto lv = static_cast<I32Object*>(left.get());
            auto rv = static_cast<I32Object*>(right.get());
            return std::make_shared<I32Object>(lv->value | rv->value);
}
        case ObjectType::U32: {
            auto lv = static_cast<U32Object*>(left.get());
            auto rv = static_cast<U32Object*>(right.get());
            return std::make_shared<U32Object>(lv->value | rv->value);
}
        case ObjectType::I64: {
            auto lv = static_cast<I64Object*>(left.get());
            auto rv = static_cast<I64Object*>(right.get());
            return std::make_shared<I64Object>(lv->value | rv->value);
        }
        case ObjectType::U64: {
            auto lv = static_cast<U64Object*>(left.get());
            auto rv = static_cast<U64Object*>(right.get());
            return std::make_shared<U64Object>(lv->value | rv->value);
}
        default:
            return std::make_shared<ErrorObject>("| operator is not supported for this type");
    }
}

SPtr<Object> objectBitCaret(SPtr<Object> left, SPtr<Object> right) {
    if(left->type() != right->type()) {
        return std::make_shared<ErrorObject>("~ operator is not supported for same type");
    }
    auto type = left->type();
    // 允许 i32,u32,i64,u64四种运算符操作
    switch(type) {
        case ObjectType::I32: {
            auto lv = static_cast<I32Object*>(left.get());
            auto rv = static_cast<I32Object*>(right.get());
            return std::make_shared<I32Object>(lv->value ^ rv->value);
}
        case ObjectType::U32: {
            auto lv = static_cast<U32Object*>(left.get());
            auto rv = static_cast<U32Object*>(right.get());
            return std::make_shared<U32Object>(lv->value ^ rv->value);
}
        case ObjectType::I64: {
            auto lv = static_cast<I64Object*>(left.get());
            auto rv = static_cast<I64Object*>(right.get());
            return std::make_shared<I64Object>(lv->value ^ rv->value);
        }
        case ObjectType::U64: {
            auto lv = static_cast<U64Object*>(left.get());
            auto rv = static_cast<U64Object*>(right.get());
            return std::make_shared<U64Object>(lv->value ^ rv->value);
}
        default:
            return std::make_shared<ErrorObject>("~ operator is not supported for this type");
    }
}

SPtr<Object> objectAssign(SPtr<Object> left, SPtr<Object> right) {
    if(left->type() == right->type()) {
        if(objectIn(left->type(),{ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64,ObjectType::BOOL,ObjectType::STRING})) {
            // 基础数据类型可以进行赋值
            switch(left->type()) {
                case ObjectType::I32: {
                    auto lv = static_cast<I32Object*>(left.get());
                    auto rv = static_cast<I32Object*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::U32: {
                    auto lv = static_cast<U32Object*>(left.get());
                    auto rv = static_cast<U32Object*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::I64: {
                    auto lv = static_cast<I64Object*>(left.get());
                    auto rv = static_cast<I64Object*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::U64: {
                    auto lv = static_cast<U64Object*>(left.get());
                    auto rv = static_cast<U64Object*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::F32: {
                    auto lv = static_cast<F32Object*>(left.get());
                    auto rv = static_cast<F32Object*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::F64: {
                    auto lv = static_cast<F64Object*>(left.get());
                    auto rv = static_cast<F64Object*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::BOOL: {
                    auto lv = static_cast<BoolObject*>(left.get());
                    auto rv = static_cast<BoolObject*>(right.get());
                    lv->value = rv->value;
                    return left;
}                case ObjectType::STRING: {
                    auto lv = static_cast<StringObject*>(left.get());
                    auto rv = static_cast<StringObject*>(right.get());
                    lv->value = rv->value;
                    return left;
}                default:
                    return std::make_shared<ErrorObject>("= operator is not supported for this type");
            }
        } else {
            // 直接移动
            left = std::move(right);
            return left;
        }
    } else {
        // 基础数据类型可以进行类型提升赋值，这里使用拷贝而不是移动
        // 左操作数支持 i32,u32,i64,u64,f32,f64
        // 右操作数支持 i32,u32,i64,u64,f32,f64,bool
        if(!objectIn(left->type(),{ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64,ObjectType::NIL})) {
            return std::make_shared<ErrorObject>("= operator is not supported for this type");
        }
        if(!objectIn(right->type(),{ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64,ObjectType::BOOL})) {
            return std::make_shared<ErrorObject>("= operator is not supported for this type");
        }

        // 赋值取决于左边的类型
        switch(left->type()) {
            case ObjectType::I32: {
                auto lv = static_cast<I32Object*>(left.get());
                auto rv = toI64(right);
                lv->value = rv;
                return left;
            }
            case ObjectType::U32: {
                auto lv = static_cast<U32Object*>(left.get());
                auto rv = toU64(right);
                lv->value = rv;
                return left;
            }
            case ObjectType::I64: {
                auto lv = static_cast<I64Object*>(left.get());
                auto rv = toI64(right);
                lv->value = rv;
                return left;
            }
            case ObjectType::U64: {
                auto lv = static_cast<U64Object*>(left.get());
                auto rv = toU64(right);
                lv->value = rv;
                return left;
            }
            case ObjectType::F32: {
                auto lv = static_cast<F32Object*>(left.get());
                auto rv = toF64(right);
                lv->value = rv;
                return left;
            }
            case ObjectType::F64: {
                auto lv = static_cast<F64Object*>(left.get());
                auto rv = toF64(right);
                lv->value = rv;
                return left;
            }
            case ObjectType::NIL: {
                left = std::move(right);
                return left;
            }
            default:
                return std::make_shared<ErrorObject>("= operator is not supported for this type");
        }
    }
}

SPtr<Object> objectNotEqual(SPtr<Object> left, SPtr<Object> right) {
    // objectEquel肯定返回BoolObject / ErrorObject
    auto res = objectEqual(left,right);
    if(res->type() == ObjectType::BOOL) {
        auto resv = static_cast<BoolObject*>(res.get());
        return std::make_shared<BoolObject>(!resv->value);
    } else {
        return res;
    }
}

SPtr<Object> objectEqual(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32, u32, i64, u64, f32, f64, bool, string 之间的比较
    auto lt = left->type();
    auto rt = right->type();

    // 同类型直接比较，无精度损失
    if(lt == rt) {
        switch(lt) {
            case ObjectType::I32:
                return std::make_shared<BoolObject>(static_cast<I32Object*>(left.get())->value == static_cast<I32Object*>(right.get())->value);
            case ObjectType::U32:
                return std::make_shared<BoolObject>(static_cast<U32Object*>(left.get())->value == static_cast<U32Object*>(right.get())->value);
            case ObjectType::I64:
                return std::make_shared<BoolObject>(static_cast<I64Object*>(left.get())->value == static_cast<I64Object*>(right.get())->value);
            case ObjectType::U64:
                return std::make_shared<BoolObject>(static_cast<U64Object*>(left.get())->value == static_cast<U64Object*>(right.get())->value);
            case ObjectType::F32:
                return std::make_shared<BoolObject>(static_cast<F32Object*>(left.get())->value == static_cast<F32Object*>(right.get())->value);
            case ObjectType::F64:
                return std::make_shared<BoolObject>(static_cast<F64Object*>(left.get())->value == static_cast<F64Object*>(right.get())->value);
            case ObjectType::BOOL:
                return std::make_shared<BoolObject>(static_cast<BoolObject*>(left.get())->value == static_cast<BoolObject*>(right.get())->value);
            case ObjectType::STRING:
                return std::make_shared<BoolObject>(static_cast<StringObject*>(left.get())->value == static_cast<StringObject*>(right.get())->value);
            default:
                return std::make_shared<ErrorObject>("== operator is not supported for this type");
        }
    }

    // 类型不同：提升到公共类型后比较
    auto targetType = promote(lt, rt);
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("== operator is not supported for this type");
    }
    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<BoolObject>(toI64(left) == toI64(right));
        case ObjectType::U32:
            return std::make_shared<BoolObject>(toU64(left) == toU64(right));
        case ObjectType::I64:
            return std::make_shared<BoolObject>(toI64(left) == toI64(right));
        case ObjectType::U64:
            return std::make_shared<BoolObject>(toU64(left) == toU64(right));
        case ObjectType::F32:
            return std::make_shared<BoolObject>(toF64(left) == toF64(right));
        case ObjectType::F64:
            return std::make_shared<BoolObject>(toF64(left) == toF64(right));
        default:
            return std::make_shared<ErrorObject>("== operator is not supported for this type");
    }
}

SPtr<Object> objectGreaterEqual(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32, u32, i64, u64, f32, f64 之间的比较
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>(">= operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>(">= operator is not supported for this type");
    }

    // 类型提升
    auto targetType = promote(lt, rt);
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>(">= operator is not supported for this type");
    }

    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<BoolObject>(toI64(left) >= toI64(right));
        case ObjectType::U32:
            return std::make_shared<BoolObject>(toU64(left) >= toU64(right));
        case ObjectType::I64:
            return std::make_shared<BoolObject>(toI64(left) >= toI64(right));
        case ObjectType::U64:
            return std::make_shared<BoolObject>(toU64(left) >= toU64(right));
        case ObjectType::F32:
            return std::make_shared<BoolObject>(toF64(left) >= toF64(right));
        case ObjectType::F64:
            return std::make_shared<BoolObject>(toF64(left) >= toF64(right));
        default:
            return std::make_shared<ErrorObject>(">= operator is not supported for this type");
    }
}

SPtr<Object> objectLessEqual(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32, u32, i64, u64, f32, f64 之间的比较
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>("<= operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>("<= operator is not supported for this type");
    }

    // 类型提升
    auto targetType = promote(lt, rt);
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("<= operator is not supported for this type");
    }

    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<BoolObject>(toI64(left) <= toI64(right));
        case ObjectType::U32:
            return std::make_shared<BoolObject>(toU64(left) <= toU64(right));
        case ObjectType::I64:
            return std::make_shared<BoolObject>(toI64(left) <= toI64(right));
        case ObjectType::U64:
            return std::make_shared<BoolObject>(toU64(left) <= toU64(right));
        case ObjectType::F32:
            return std::make_shared<BoolObject>(toF64(left) <= toF64(right));
        case ObjectType::F64:
            return std::make_shared<BoolObject>(toF64(left) <= toF64(right));
        default:
            return std::make_shared<ErrorObject>("<= operator is not supported for this type");
    }
}

SPtr<Object> objectGreater(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32, u32, i64, u64, f32, f64 之间的比较
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>("> operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>("> operator is not supported for this type");
    }

    // 类型提升
    auto targetType = promote(lt, rt);
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("> operator is not supported for this type");
    }

    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<BoolObject>(toI64(left) > toI64(right));
        case ObjectType::U32:
            return std::make_shared<BoolObject>(toU64(left) > toU64(right));
        case ObjectType::I64:
            return std::make_shared<BoolObject>(toI64(left) > toI64(right));
        case ObjectType::U64:
            return std::make_shared<BoolObject>(toU64(left) > toU64(right));
        case ObjectType::F32:
            return std::make_shared<BoolObject>(toF64(left) > toF64(right));
        case ObjectType::F64:
            return std::make_shared<BoolObject>(toF64(left) > toF64(right));
        default:
            return std::make_shared<ErrorObject>("> operator is not supported for this type");
    }
}

SPtr<Object> objectLess(SPtr<Object> left, SPtr<Object> right) {
    // 支持 i32, u32, i64, u64, f32, f64 之间的比较
    auto lt = left->type();
    auto rt = right->type();

    // 进行类型检查
    if(!objectIn(lt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>("< operator is not supported for this type");
    }
    if(!objectIn(rt, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
        return std::make_shared<ErrorObject>("< operator is not supported for this type");
    }

    // 类型提升
    auto targetType = promote(lt, rt);
    if(targetType == ObjectType::ERROR) {
        return std::make_shared<ErrorObject>("< operator is not supported for this type");
    }

    switch(targetType) {
        case ObjectType::I32:
            return std::make_shared<BoolObject>(toI64(left) < toI64(right));
        case ObjectType::U32:
            return std::make_shared<BoolObject>(toU64(left) < toU64(right));
        case ObjectType::I64:
            return std::make_shared<BoolObject>(toI64(left) < toI64(right));
        case ObjectType::U64:
            return std::make_shared<BoolObject>(toU64(left) < toU64(right));
        case ObjectType::F32:
            return std::make_shared<BoolObject>(toF64(left) < toF64(right));
        case ObjectType::F64:
            return std::make_shared<BoolObject>(toF64(left) < toF64(right));
        default:
            return std::make_shared<ErrorObject>("< operator is not supported for this type");
    }
}

i64 toI64(SPtr<Object> num) {
    // 将任意数值类型统一提取为 i64（bool 视为 0/1）
    switch(num->type()) {
        case ObjectType::I32:
            return static_cast<i64>(static_cast<I32Object*>(num.get())->value);
        case ObjectType::U32:
            return static_cast<i64>(static_cast<U32Object*>(num.get())->value);
        case ObjectType::I64:
            return static_cast<I64Object*>(num.get())->value;
        case ObjectType::U64:
            return static_cast<i64>(static_cast<U64Object*>(num.get())->value);
        case ObjectType::F32:
            return static_cast<i64>(static_cast<F32Object*>(num.get())->value);
        case ObjectType::F64:
            return static_cast<i64>(static_cast<F64Object*>(num.get())->value);
        case ObjectType::BOOL:
            return static_cast<BoolObject*>(num.get())->value ? 1 : 0;
        default:
            return 0;
    }
}

u64 toU64(SPtr<Object> num) {
    // 将任意数值类型统一提取为 u64（bool 视为 0/1）
    switch(num->type()) {
        case ObjectType::I32:
            return static_cast<u64>(static_cast<I32Object*>(num.get())->value);
        case ObjectType::U32:
            return static_cast<u64>(static_cast<U32Object*>(num.get())->value);
        case ObjectType::I64:
            return static_cast<u64>(static_cast<I64Object*>(num.get())->value);
        case ObjectType::U64:
            return static_cast<U64Object*>(num.get())->value;
        case ObjectType::F32:
            return static_cast<u64>(static_cast<F32Object*>(num.get())->value);
        case ObjectType::F64:
            return static_cast<u64>(static_cast<F64Object*>(num.get())->value);
        case ObjectType::BOOL:
            return static_cast<BoolObject*>(num.get())->value ? 1 : 0;
        default:
            return 0;
    }
}

f64 toF64(SPtr<Object> num) {
    // 将任意数值类型统一提取为 f64（bool 视为 0.0/1.0）
    switch(num->type()) {
        case ObjectType::I32:
            return static_cast<f64>(static_cast<I32Object*>(num.get())->value);
        case ObjectType::U32:
            return static_cast<f64>(static_cast<U32Object*>(num.get())->value);
        case ObjectType::I64:
            return static_cast<f64>(static_cast<I64Object*>(num.get())->value);
        case ObjectType::U64:
            return static_cast<f64>(static_cast<U64Object*>(num.get())->value);
        case ObjectType::F32:
            return static_cast<f64>(static_cast<F32Object*>(num.get())->value);
        case ObjectType::F64:
            return static_cast<F64Object*>(num.get())->value;
        case ObjectType::BOOL:
            return static_cast<BoolObject*>(num.get())->value ? 1.0 : 0.0;
        default:
            return 0.0;
    }
}

bool toBool(SPtr<Object> obj) {
    // 真值判断：数值非0为true，bool取自身，string非空为true，其余为false
    switch(obj->type()) {
        case ObjectType::I32:
            return static_cast<I32Object*>(obj.get())->value != 0;
        case ObjectType::U32:
            return static_cast<U32Object*>(obj.get())->value != 0;
        case ObjectType::I64:
            return static_cast<I64Object*>(obj.get())->value != 0;
        case ObjectType::U64:
            return static_cast<U64Object*>(obj.get())->value != 0;
        case ObjectType::F32:
            return static_cast<F32Object*>(obj.get())->value != 0.0f;
        case ObjectType::F64:
            return static_cast<F64Object*>(obj.get())->value != 0.0;
        case ObjectType::BOOL:
            return static_cast<BoolObject*>(obj.get())->value;
        case ObjectType::STRING:
            return !static_cast<StringObject*>(obj.get())->value.empty();
        default:
            return false;
    }
}

String toString(SPtr<Object> obj) {
    return String(obj->inspect());
}

ObjectType promote(ObjectType lt, ObjectType rt) {
    // 类型提升规则（C 语言 usual arithmetic conversions 的简化版）
    // 1. 有 STRING，提升到 STRING
    // 2. 只要有 F64，提升到 F64
    // 3. 有 F32，提升到 F32
    // 4. 有 U64，提升到 U64
    // 5. 有 I64，提升到 I64
    // 6. 有 U32，提升到 U32
    // 7. 否则 I32
    // 注意：i64 与 u64 混算会按 u64 提升（和 C 一致，负数会包错）

    // 先检查是否都是数值类型（含 bool），bool 视为 I32 参与提升
    auto toInt = [](ObjectType t) -> ObjectType {
        if(t == ObjectType::BOOL) return ObjectType::I32;
        return t;
    };
    ObjectType a = toInt(lt);
    ObjectType b = toInt(rt);

    if(!objectIn(a, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64,ObjectType::STRING})) {
        return ObjectType::ERROR;
    }
    if(!objectIn(b, {ObjectType::I32,ObjectType::U32,ObjectType::I64,ObjectType::U64,ObjectType::F32,ObjectType::F64,ObjectType::STRING})) {
        return ObjectType::ERROR;
    }
    if(a == ObjectType::STRING || b == ObjectType::STRING) return ObjectType::STRING;
    if(a == ObjectType::F64 || b == ObjectType::F64) return ObjectType::F64;
    if(a == ObjectType::F32 || b == ObjectType::F32) return ObjectType::F32;
    if(a == ObjectType::U64 || b == ObjectType::U64) return ObjectType::U64;
    if(a == ObjectType::I64 || b == ObjectType::I64) return ObjectType::I64;
    if(a == ObjectType::U32 || b == ObjectType::U32) return ObjectType::U32;
    return ObjectType::I32;
}
