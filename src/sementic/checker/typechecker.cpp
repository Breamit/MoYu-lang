#include "typechecker.h"


namespace {
    // 深拷贝 TypeLiteral
    UPtr<TypeLiteral> cloneType(const TypeLiteral* src) {
        if(src == nullptr) return nullptr;
        auto out = std::make_unique<TypeLiteral>();
        out->token = src->token;
        out->name = src->name;
        for(const auto& p : src->typeParams) {
            out->typeParams.push_back(cloneType(p.get()));
        }
        return out;
    }

    // UPtr -> SPtr 转移
    SPtr<TypeLiteral> toShared(UPtr<TypeLiteral> p) {
        return p ? SPtr<TypeLiteral>(std::move(p)) : nullptr;
    }

    // 比较两个 TypeLiteral 是否结构上完全相同
    bool typeEquals(const TypeLiteral* a, const TypeLiteral* b) {
        if(!a || !b) return false;
        if(a->name != b->name) return false;
        if(a->typeParams.size() != b->typeParams.size()) return false;
        for(size_t i = 0; i < a->typeParams.size(); i++) {
            if(!typeEquals(a->typeParams[i].get(), b->typeParams[i].get())) return false;
        }
        return true;
    }

    // 从 Identifier / TypeLiteral 节点提取名字
    String nodeName(Expression* node) {
        if(node == nullptr) return "";
        if(node->type() == NodeType::IDENTIFIER) {
            return static_cast<Identifier*>(node)->value;
        }
        if(node->type() == NodeType::TYPE_LITERAL) {
            return static_cast<TypeLiteral*>(node)->name;
        }
        return "";
    }
}

Array<ErrorInfo> TypeChecker::check(Program* program) {
    if(program == nullptr) return errors_;


    pushScope();
    for(const auto& stmt : program->statements) {
        checkStatement(stmt.get());
    }
    popScope();
    return errors_;
}

void TypeChecker::setFileLoader(FileLoader fn) {
    fileLoader_ = std::move(fn);
}


void TypeChecker::error(const String& msg) {
    Token tok;
    ErrorInfo ei(tok);
    ei.setMsg(msg, ErrorType::ASTBUILD);
    errors_.push_back(ei);
}

void TypeChecker::pushScope() {
    scopes_.push(Scope{});
}

void TypeChecker::popScope() {
    if(!scopes_.empty()) scopes_.pop();
}

void TypeChecker::declareVar(const String& name, SPtr<TypeLiteral> type, bool explicitSrc) {
    if(scopes_.empty()) pushScope();
    auto& vars = scopes_.top().vars;
    if(vars.count(name) > 0) {
        error("variable '" + name + "' already declared in this scope");
        return;
    }
    TypeInfo info;
    info.type = std::move(type);
    info.sourceExplicit = explicitSrc;
    info.checked = true;
    vars[name] = std::move(info);
}

SPtr<TypeLiteral> TypeChecker::lookupVar(const String& name) {
    if(scopes_.empty()) return nullptr;
    auto scopeCopy = scopes_;
    while(!scopeCopy.empty()) {
        auto& vars = scopeCopy.top().vars;
        auto it = vars.find(name);
        if(it != vars.end()) return it->second.type;
        scopeCopy.pop();
    }
    return nullptr;
}

bool TypeChecker::isBuiltinName(const String& name) {
    static const Set<String> builtins = {
        "i32","u32","i64","u64","f32","f64",
        "string","bool","any","void","bytes","nil","list","map"
    };
    return builtins.count(name) > 0;
}

bool TypeChecker::isAssignable(const SPtr<TypeLiteral>& target, const SPtr<TypeLiteral>& source) {
    if(!target || !source) return false;
    // any 通配一切：any 可接收任何类型，也可赋给任何类型
    if(target->name == "any" || source->name == "any") return true;
    // 相同类型
    if(typeEquals(target.get(), source.get())) return true;
    // 内置数字间允许隐式转换
    static const Set<String> nums = {"i32","u32","i64","u64","f32","f64"};
    if(nums.count(target->name) && nums.count(source->name)) return true;
    // 裸 list / map兼容任何同名的 list<T> / map<K,V>
    if(target->name == "list" && source->name == "list" &&
       target->typeParams.empty()) return true;
    if(target->name == "map" && source->name == "map" &&
       target->typeParams.empty()) return true;
    // list<T1> vs list<T2>：元素递归
    if(target->name == "list" && source->name == "list" &&
       target->typeParams.size() == 1 && source->typeParams.size() == 1) {
        return isAssignable(toShared(cloneType(target->typeParams[0].get())),
                            toShared(cloneType(source->typeParams[0].get())));
    }
    // map<K1,V1> vs map<K2,V2>：键值递归
    if(target->name == "map" && source->name == "map" &&
       target->typeParams.size() == 2 && source->typeParams.size() == 2) {
        return isAssignable(toShared(cloneType(target->typeParams[0].get())),
                            toShared(cloneType(source->typeParams[0].get()))) &&
               isAssignable(toShared(cloneType(target->typeParams[1].get())),
                            toShared(cloneType(source->typeParams[1].get())));
    }
    return false;
}

SPtr<TypeLiteral> TypeChecker::deriveExpr(Expression* node) {
    if(node == nullptr) return nullptr;
    switch(node->type()) {
        case NodeType::INTEGER_LITERAL:
            return builtinType("i64");
        case NodeType::FLOAT_LITERAL:
            return builtinType("f64");
        case NodeType::STRING_LITERAL:
            return builtinType("string");
        case NodeType::BOOL_LITERAL:
            return builtinType("bool");
        case NodeType::NULL_LITERAL:
            return builtinType("nil");
        case NodeType::ARRAY_LITERAL:
            return deriveArray(static_cast<ArrayLiteral*>(node));
        case NodeType::MAP_LITERAL:
            return deriveMap(static_cast<MapLiteral*>(node));
        case NodeType::IDENTIFIER:
            return deriveIdentifier(static_cast<Identifier*>(node));
        case NodeType::PREFIX_EXPRESSION: {
            auto pe = static_cast<PrefixExpression*>(node);
            auto t = deriveExpr(pe->right.get());
            if(pe->op == "not" || pe->op == "!") return builtinType("bool");
            return t;
        }
        case NodeType::INFIX_EXPRESSION:
            return deriveInfix(static_cast<InfixExpression*>(node));
        case NodeType::CALL_EXPRESSION:
            return deriveCall(static_cast<CallExpression*>(node));
        case NodeType::INDEX_EXPRESSION:
            return deriveIndex(static_cast<IndexExpression*>(node));
        case NodeType::MEMBER_ACCESS_EXPRESSION:
            return deriveMember(static_cast<MemberAccessExpression*>(node));
        default:
            return nullptr;
    }
}

SPtr<TypeLiteral> TypeChecker::deriveArray(ArrayLiteral* node) {
    if(node->elements.empty()) return nullptr;   // 空数组类型不定
    SPtr<TypeLiteral> elem = nullptr;
    for(const auto& e : node->elements) {
        auto et = deriveExpr(e.get());
        if(elem == nullptr) { elem = et; continue; }
        if(!typeEquals(elem.get(), et.get())) { elem = builtinType("any"); break; }
    }
    if(elem == nullptr) return nullptr;
    auto listT = builtinType("list");
    listT->typeParams.push_back(cloneType(elem.get()));
    return listT;
}

SPtr<TypeLiteral> TypeChecker::deriveMap(MapLiteral* node) {
    if(node->pairs.empty()) return nullptr;
    auto k = deriveExpr(node->pairs[0].first.get());
    auto v = deriveExpr(node->pairs[0].second.get());
    auto anyT = builtinType("any");
    auto mapT = builtinType("map");
    mapT->typeParams.push_back(k ? cloneType(k.get()) : cloneType(anyT.get()));
    mapT->typeParams.push_back(v ? cloneType(v.get()) : cloneType(anyT.get()));
    return mapT;
}

SPtr<TypeLiteral> TypeChecker::deriveIdentifier(Identifier* node) {
    auto t = lookupVar(node->value);
    if(t == nullptr) {
        error("variable '" + node->value + "' is not defined");
    }
    return t;
}

SPtr<TypeLiteral> TypeChecker::deriveInfix(InfixExpression* node) {
    auto lt = deriveExpr(node->left.get());
    auto rt = deriveExpr(node->right.get());

    // 赋值在 checkAssign 处理，这里跳过
    if(node->op == "=") return rt;

    // 比较 / 逻辑运算 -> bool
    if(node->op == "==" || node->op == "!=" || node->op == "<" ||
       node->op == ">" || node->op == "<=" || node->op == ">=" ||
       node->op == "and" || node->op == "or") {
        if(lt && rt && !isAssignable(lt, rt) && !isAssignable(rt, lt) &&
           lt->name != "any" && rt->name != "any") {
            error("cannot compare '" + lt->name + "' with '" + rt->name + "'");
        }
        node->typeInfo = std::make_shared<TypeInfo>();
        node->typeInfo->type = builtinType("bool");
        node->typeInfo->checked = true;
        return node->typeInfo->type;
    }

    // 算术 -> 操作数类型
    if(lt && rt) {
        if(!isAssignable(lt, rt) && !isAssignable(rt, lt) &&
           lt->name != "any" && rt->name != "any") {
            error("operator '" + node->op + "' not supported between '" +
                  lt->name + "' and '" + rt->name + "'");
        }
        auto res = (lt->name == "any") ? rt : lt;
        node->typeInfo = std::make_shared<TypeInfo>();
        node->typeInfo->type = cloneType(res.get());
        node->typeInfo->checked = true;
        return node->typeInfo->type;
    }
    return nullptr;
}

SPtr<TypeLiteral> TypeChecker::deriveCall(CallExpression* node) {
    String fname = nodeName(node->func.get());
    if(fname.empty()) {
        error("function name must be an identifier");
        return nullptr;
    }

    // 模块函数 io.getInt()
    if(node->func->type() == NodeType::MEMBER_ACCESS_EXPRESSION) {
        auto ma = static_cast<MemberAccessExpression*>(node->func.get());
        String modName = nodeName(ma->object.get());
        String memName = nodeName(ma->called.get());
        auto mit = modules_.find(modName);
        if(mit == modules_.end()) {
            error("module '" + modName + "' is not imported");
            return nullptr;
        }
        auto cit = mit->second.find(memName);
        if(cit == mit->second.end()) {
            // 内置模块（表为空）宽松：成员存在性交给运行时，不报编译错
            if(mit->second.empty()) return nullptr;
            error("module '" + modName + "' has no member '" + memName + "'");
            return nullptr;
        }
        node->typeInfo = std::make_shared<TypeInfo>();
        node->typeInfo->type = cloneType(cit->second.get());
        node->typeInfo->checked = true;
        return node->typeInfo->type;
    }

    // 普通函数
    auto fit = funcs_.find(fname);
    if(fit == funcs_.end()) {
        error("function '" + fname + "' is not defined");
        return nullptr;
    }
    const auto& sig = fit->second;

    // struct 构造（Demo({...})）：返回类型是 struct 名，参数个数宽松
    if(structs_.count(sig.ret->name) > 0) {
        node->typeInfo = std::make_shared<TypeInfo>();
        node->typeInfo->type = cloneType(sig.ret.get());
        node->typeInfo->checked = true;
        return node->typeInfo->type;
    }

    if(sig.params.size() != node->arguments.size()) {
        error("function '" + fname + "' expects " +
              std::to_string(sig.params.size()) + " args, got " +
              std::to_string(node->arguments.size()));
        return nullptr;
    }
    for(size_t i = 0; i < node->arguments.size(); i++) {
        auto at = deriveExpr(node->arguments[i].get());
        if(at && !isAssignable(sig.params[i], at)) {
            error("arg " + std::to_string(i) + " of '" + fname +
                  "' expects '" + sig.params[i]->name + "', got '" + at->name + "'");
        }
    }
    node->typeInfo = std::make_shared<TypeInfo>();
    node->typeInfo->type = cloneType(sig.ret.get());
    node->typeInfo->checked = true;
    return node->typeInfo->type;
}

SPtr<TypeLiteral> TypeChecker::deriveIndex(IndexExpression* node) {
    auto lt = deriveExpr(node->left.get());
    if(lt == nullptr) return nullptr;
    if(lt->name == "list" && lt->typeParams.size() == 1) {
        return cloneType(lt->typeParams[0].get());
    }
    if(lt->name == "string") return builtinType("string");
    if(lt->name == "any") return nullptr;
    error("cannot index a value of type '" + lt->name + "'");
    return nullptr;
}

SPtr<TypeLiteral> TypeChecker::deriveMember(MemberAccessExpression* node) {
    auto obj = nodeName(node->object.get());
    auto mem = nodeName(node->called.get());

    // 方法调用 self.func(args) / obj.method(args)：
    // AST 是 MemberAccess(object, called=CallExpression(func=Identifier,...))
    if(node->called->type() == NodeType::CALL_EXPRESSION) {
        auto call = static_cast<CallExpression*>(node->called.get());
        auto methodName = nodeName(call->func.get());
        // 求值实参（检查它们，但不强制签名）
        for(const auto& arg : call->arguments) {
            deriveExpr(arg.get());
        }
        // 返回类型：查字段或方法表——这里宽松返回 any
        node->typeInfo = std::make_shared<TypeInfo>();
        node->typeInfo->type = builtinType("any");
        node->typeInfo->checked = true;
        (void)methodName;
        return node->typeInfo->type;
    }

    // obj 是模块名
    auto mit = modules_.find(obj);
    if(mit != modules_.end()) {
        auto cit = mit->second.find(mem);
        if(cit != mit->second.end()) return cloneType(cit->second.get());
        if(mit->second.empty()) return nullptr;   // 内置模块宽松
        error("module '" + obj + "' has no member '" + mem + "'");
        return nullptr;
    }

    // struct 字段访问 p.x：obj 可能是变量名或类型名
    // 若是变量 -> 先查它的类型名，再用类型名查 structs_
    String typeName = obj;
    if(node->object->type() == NodeType::IDENTIFIER) {
        auto vt = lookupVar(obj);
        if(vt != nullptr) typeName = vt->name;
    }
    auto sit = structs_.find(typeName);
    if(sit != structs_.end()) {
        for(const auto& [fname, ftype] : sit->second) {
            if(fname == mem) return cloneType(ftype.get());
        }
        error("struct '" + typeName + "' has no field '" + mem + "'");
        return nullptr;
    }

    error("member access on unknown type '" + obj + "'");
    return nullptr;
}

void TypeChecker::checkStatement(Statement* stmt) {
    if(stmt == nullptr) return;
    switch(stmt->type()) {
        case NodeType::LET_STATEMENT:
            checkLetStatement(static_cast<LetStatement*>(stmt));
            break;
        case NodeType::CONST_STATEMENT:
            checkConstStatement(static_cast<ConstStatement*>(stmt));
            break;
        case NodeType::FUNCTION_DECLARE_STATEMENT:
            checkFunctionDeclareStatement(static_cast<FunctionDeclareStatement*>(stmt));
            break;
        case NodeType::RETURN_STATEMENT:
            checkReturnStatement(static_cast<ReturnStatement*>(stmt));
            break;
        case NodeType::IF_STATEMENT:
            checkIfStatement(static_cast<IfStatement*>(stmt));
            break;
        case NodeType::WHILE_STATEMENT:
            checkWhileStatement(static_cast<WhileStatement*>(stmt));
            break;
        case NodeType::FOR_STATEMENT:
            checkForStatement(static_cast<ForStatement*>(stmt));
            break;
        case NodeType::SWITCH_STATEMENT:
            checkSwitchStatement(static_cast<SwitchStatement*>(stmt));
            break;
        case NodeType::BLOCK_STATEMENT:
            checkBlockStatement(static_cast<BlockStatement*>(stmt));
            break;
        case NodeType::EXPRESSION_STATEMENT:
            checkExpressionStatement(static_cast<ExpressionStatement*>(stmt));
            break;
        case NodeType::IMPORT_STATEMENT:
            checkImportStatement(static_cast<ImportStatement*>(stmt));
            break;
        case NodeType::STRUCT_DECLARE_STATEMENT: {
            // 登记 struct 类型：structs_ 字段表 + funcs_ 构造签名
            auto sd = static_cast<StructDeclareStatement*>(stmt);
            Array<Pair<String, SPtr<TypeLiteral>>> fields;
            for(const auto& [fname, ftype] : sd->fields) {
                if(fname && ftype) {
                    fields.push_back({static_cast<Identifier*>(fname.get())->value,
                                      toShared(cloneType(ftype.get()))});
                }
            }
            structs_[sd->name->value] = std::move(fields);
            // 构造签名：参数是 map<string, 字段类型...>
            FuncSig ctor;
            ctor.ret = builtinType(sd->name->value);
            funcs_[sd->name->value] = std::move(ctor);
            break;
        }
        case NodeType::ANNOTATION_STATEMENT: {
            auto ann = static_cast<AnnotationStatement*>(stmt);
            if(ann->target) checkStatement(ann->target.get());
            break;
        }
        default:
            break;
    }
}

void TypeChecker::checkLetStatement(LetStatement* node) {
    auto derived = deriveExpr(node->value.get());
    SPtr<TypeLiteral> finalType;
    bool explicitSrc = false;
    if(node->varType != nullptr) {
        finalType = toShared(cloneType(node->varType.get()));
        explicitSrc = true;
        if(!isBuiltinName(finalType->name) && structs_.count(finalType->name) == 0) {
            error("unknown type '" + finalType->name + "'");
        }
        if(derived && !isAssignable(finalType, derived)) {
            error("cannot assign '" + derived->name +
                  "' to variable of type '" + finalType->name + "'");
        }
    } else {
        finalType = derived;
    }
    node->typeInfo = std::make_shared<TypeInfo>();
    node->typeInfo->type = finalType;
    node->typeInfo->sourceExplicit = explicitSrc;
    node->typeInfo->checked = true;
    declareVar(node->name->value, finalType, explicitSrc);
}

void TypeChecker::checkConstStatement(ConstStatement* node) {
    auto derived = deriveExpr(node->value.get());
    SPtr<TypeLiteral> finalType;
    bool explicitSrc = false;
    if(node->varType != nullptr) {
        finalType = toShared(cloneType(node->varType.get()));
        explicitSrc = true;
        if(derived && !isAssignable(finalType, derived)) {
            error("cannot assign '" + derived->name +
                  "' to const of type '" + finalType->name + "'");
        }
    } else {
        finalType = derived;
    }
    node->typeInfo = std::make_shared<TypeInfo>();
    node->typeInfo->type = finalType;
    node->typeInfo->sourceExplicit = explicitSrc;
    node->typeInfo->checked = true;
    declareVar(node->name->value, finalType, explicitSrc);
}

void TypeChecker::checkFunctionDeclareStatement(FunctionDeclareStatement* node) {
    if(funcs_.count(node->name->value) > 0) {
        error("function '" + node->name->value + "' already declared");
        return;
    }
    FuncSig sig;
    for(const auto& [pname, ptype] : node->params) {
        if(ptype) sig.params.push_back(toShared(cloneType(ptype.get())));
        else sig.params.push_back(builtinType("any"));
    }
    sig.ret = node->returnType ? toShared(cloneType(node->returnType.get())) : builtinType("void");
    funcs_[node->name->value] = sig;

    if(node->body) {
        pushScope();
        for(size_t i = 0; i < node->params.size(); i++) {
            auto pn = nodeName(node->params[i].first.get());
            auto pt = sig.params[i];
            declareVar(pn, pt, true);
        }
        checkBlockStatement(node->body.get());
        popScope();
    }
    node->typeInfo = std::make_shared<TypeInfo>();
    node->typeInfo->type = sig.ret;
    node->typeInfo->checked = true;
}

void TypeChecker::checkReturnStatement(ReturnStatement* node) {
    // 返回值类型检查：当前函数返回类型记录在 funcs_，但需要知道当前在哪个函数。
    // 简化：先推导返回值类型，函数级返回检查留给后续增强。
    if(node->value) deriveExpr(node->value.get());
}

void TypeChecker::checkIfStatement(IfStatement* node) {
    if(node->condition) {
        auto ct = deriveExpr(node->condition.get());
        if(ct && ct->name != "bool" && ct->name != "any") {
            error("if condition must be bool, got '" + ct->name + "'");
        }
    }
    if(node->consequence) checkBlockStatement(node->consequence.get());
    for(const auto& [cond, body] : node->elifClauses) {
        if(cond) {
            auto ct = deriveExpr(cond.get());
            if(ct && ct->name != "bool" && ct->name != "any") {
                error("elif condition must be bool");
            }
        }
        if(body) checkBlockStatement(body.get());
    }
    if(node->alternative) checkBlockStatement(node->alternative.get());
}

void TypeChecker::checkWhileStatement(WhileStatement* node) {
    if(node->condition) {
        auto ct = deriveExpr(node->condition.get());
        if(ct && ct->name != "bool" && ct->name != "any") {
            error("while condition must be bool");
        }
    }
    if(node->body) checkBlockStatement(node->body.get());
}

void TypeChecker::checkForStatement(ForStatement* node) {
    pushScope();
    if(node->init) checkStatement(node->init.get());
    if(node->condition) {
        auto ct = deriveExpr(node->condition.get());
        if(ct && ct->name != "bool" && ct->name != "any") {
            error("for condition must be bool");
        }
    }
    if(node->update) deriveExpr(node->update.get());
    if(node->body) checkBlockStatement(node->body.get());
    popScope();
}

void TypeChecker::checkSwitchStatement(SwitchStatement* node) {
    auto st = node->condition ? deriveExpr(node->condition.get()) : nullptr;
    for(const auto& [c, body] : node->cases) {
        if(c) {
            auto ct = deriveExpr(c.get());
            if(st && ct && !isAssignable(st, ct) && !isAssignable(ct, st) && st->name != "any") {
                error("case type '" + ct->name + "' does not match switch type '" + st->name + "'");
            }
        }
        if(body) checkBlockStatement(body.get());
    }
    if(node->defaultCase) checkBlockStatement(node->defaultCase.get());
}

void TypeChecker::checkBlockStatement(BlockStatement* node) {
    pushScope();
    for(const auto& s : node->statements) {
        checkStatement(s.get());
    }
    popScope();
}

void TypeChecker::checkExpressionStatement(ExpressionStatement* node) {
    if(node->expression) {
        auto expr = node->expression.get();
        if(expr->type() == NodeType::INFIX_EXPRESSION) {
            auto inf = static_cast<InfixExpression*>(expr);
            if(inf->op == "=") { checkAssign(inf); return; }
        }
        deriveExpr(expr);
    }
}

void TypeChecker::checkAssign(InfixExpression* node) {
    if(node->left->type() != NodeType::IDENTIFIER) return;
    auto varName = static_cast<Identifier*>(node->left.get())->value;
    auto varType = lookupVar(varName);
    if(varType == nullptr) {
        error("assignment to undeclared variable '" + varName + "'");
        return;
    }
    auto rt = deriveExpr(node->right.get());
    if(rt && varType && !isAssignable(varType, rt)) {
        error("cannot assign '" + rt->name +
              "' to variable '" + varName + "' of type '" + varType->name + "'");
    }
}

void TypeChecker::checkImportStatement(ImportStatement* node) {
    auto modName = node->name->value;
    if(modules_.count(modName) > 0) return;
    // 内置模块：宽松，成员存在性交给运行时
    static const Set<String> stdlib = {"io","fs","json","math","os","time"};
    if(stdlib.count(modName) > 0) {
        modules_[modName] = {};
        return;
    }
    // 自定义模块：FileLoader 加载 + 递归检查
    if(fileLoader_) {
        checkModule(modName, "");
    }
}

void TypeChecker::checkModule(const String& moduleName, const String& path) {
    if(checkingModules_.count(moduleName) > 0) {
        error("cycle import of module '" + moduleName + "'");
        return;
    }
    checkingModules_.insert(moduleName);
    if(fileLoader_) {
        auto modProgram = fileLoader_(moduleName, path);
        if(modProgram != nullptr) {
            pushScope();
            for(const auto& stmt : modProgram->statements) {
                checkStatement(stmt.get());
            }
            popScope();
        }
    }
    checkingModules_.erase(moduleName);
}

SPtr<TypeLiteral> TypeChecker::builtinType(const String& name) {
    auto tl = std::make_shared<TypeLiteral>();
    tl->name = name;
    return tl;
}
