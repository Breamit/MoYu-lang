#ifndef __BUILTIN_HPP__
#define __BUILTIN_HPP__
#include "builtin_type.h"
#include <src/runtime/sync/env.hpp>
void registerBuiltinFunctions(SPtr<Environment>& env) {
    env->set("len", std::make_shared<NativeObject>("builtin.len", builtinLen));
    env->set("keys", std::make_shared<NativeObject>("builtin.keys", builtinKeys));
    env->set("values", std::make_shared<NativeObject>("builtin.values", builtinValues));
    env->set("type", std::make_shared<NativeObject>("builtin.type", builtinType));
}
#endif