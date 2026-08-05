#include "sync.h"
#include <src/ast/parser.h>
#include <src/object/object_ops.h>
#include <iostream>
#include "module.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <src/builtin/builtin.hpp>

SyncRuntime::SyncRuntime(const String& projectRoot) : moduleLoader_(projectRoot) {
    globalEnv = std::make_shared<Environment>();

    // 注册内置函数
    registerBuiltinFunctions(globalEnv);
    
    exitCode = 0;
    // 装载全局程序
    // 模块文件加载回调：读文件 → parse → eval → 收集 export
    moduleLoader_.setFileLoader([this](const String& path, const String& name) -> SPtr<ModuleObject> {
        auto res = loadModuleFile(path, name);
        // loadModuleFile 返回 Object；出错或非模块返回 nullptr
        if(res != nullptr && res->type() == ObjectType::MODULE) {
            return std::static_pointer_cast<ModuleObject>(res);
        }
        return nullptr;
    });


}

u32 SyncRuntime::run(const String &content,String& res) {
    if(content.empty())return 0;
    UPtr<Lexer> l = std::make_unique<Lexer>(content);

    if(!l->getErrors().empty()) {
        for(const auto& err : l->getErrors()) {
            err.println(std::cout);
        }
        return 1;
    }
    Parser p(std::move(l));
    if(!p.getErrors().empty()) {
        for(const auto& err : p.getErrors()) {
            err.println(std::cout);
        }
        return 1;
    }
    auto node = p.parseProgram();
    // 执行优化
    executeMiddleware(node.get());
    auto result = eval(node.get(),globalEnv);

    programPool_.push_back(std::move(node));   // 缓存已解析的程序，保活 AST
    if(result == nullptr) {
        res = "fatal eval";
        return exitCode;
    }
    if(result->type() == ObjectType::ERROR) {
        std::cout<<result->inspect()<<std::endl;
        return exitCode;
    }
    return exitCode;
}

SPtr<Object> SyncRuntime::eval(const Node* node, SPtr<Environment> env) {
    NodeType tp = node->type();

    // 根据type枚举量进行switch选择，这个比纯if-elif-else快
    switch(tp) {
        // 程序对象
        case NodeType::PROGRAM: {
            // 转换
            auto program = dynamic_cast<const Program*>(node);
            if(!program) {
                exitCode = 1;   // 证明是错误退出，而不是真的返回nullptr
                return nullptr;
            }
            auto res = evalStatements(program->statements,env);
            return res;
        } case NodeType::EXPRESSION_STATEMENT: {
            auto exprStmt = dynamic_cast<const ExpressionStatement*>(node);
            if(!node) {
                exitCode = 1;
                return nullptr;
            }
            auto val = eval(exprStmt->expression.get(),env);
            if(val != nullptr && val->type() == ObjectType::ERROR) {
                return val;
            }
            return val;
        } case NodeType::STRUCT_DECLARE_STATEMENT: {
            // 结构体声明
            auto structDecl = dynamic_cast<const StructDeclareStatement*>(node);
            if(!structDecl) {
                exitCode = 1;
                return nullptr;
            }
            return evalStructDeclareStatement(structDecl, env);
        } case NodeType::MEMBER_ACCESS_EXPRESSION: {
            // 成员访问（字段 / 方法调用）
            auto member = dynamic_cast<const MemberAccessExpression*>(node);
            if(!member) {
                exitCode = 1;
                return nullptr;
            }
            return evalMemberAccessExpression(member, env);
        } case NodeType::INTEGER_LITERAL: {
            auto lit = dynamic_cast<const IntegerLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalInteger(lit);
        } case NodeType::FLOAT_LITERAL: {
            auto lit = dynamic_cast<const FloatLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalFloating(lit);
        } case NodeType::STRING_LITERAL: {
            auto lit = dynamic_cast<const StringLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalString(lit);
        } case NodeType::BOOL_LITERAL: {
            auto lit = dynamic_cast<const BoolLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalBool(lit);
        } case NodeType::NULL_LITERAL: {
            auto lit = dynamic_cast<const NullLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalNull(lit);
        } case NodeType::FUNCTION_LITERAL: {
            auto lit = dynamic_cast<const FunctionLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalFunctionLiteral(lit, env);
        } case NodeType::IDENTIFIER: {
            auto ident = dynamic_cast<const Identifier*>(node);
            if(!ident) { exitCode = 1; return nullptr; }
            return evalIdentifier(ident, env);
        } case NodeType::PREFIX_EXPRESSION: {
            auto expr = dynamic_cast<const PrefixExpression*>(node);
            if(!expr) { exitCode = 1; return nullptr; }
            return evalPrefixExpression(expr, env);
        } case NodeType::INFIX_EXPRESSION: {
            auto expr = dynamic_cast<const InfixExpression*>(node);
            if(!expr) { exitCode = 1; return nullptr; }
            return evalInfixExpression(expr, env);
        } case NodeType::BLOCK_STATEMENT: {
            auto block = dynamic_cast<const BlockStatement*>(node);
            if(!block) { exitCode = 1; return nullptr; }
            return evalBlockStatement(block, env);
        } case NodeType::LET_STATEMENT: {
            auto stmt = dynamic_cast<const LetStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalLetStatement(stmt, env);
        } case NodeType::CONST_STATEMENT: {
            auto stmt = dynamic_cast<const ConstStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalConstStatement(stmt, env);
        } case NodeType::RETURN_STATEMENT: {
            auto stmt = dynamic_cast<const ReturnStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalReturnStatement(stmt, env);
        } case NodeType::IF_STATEMENT: {
            auto stmt = dynamic_cast<const IfStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalIfStatement(stmt, env);
        } case NodeType::WHILE_STATEMENT: {
            auto stmt = dynamic_cast<const WhileStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalWhileStatement(stmt, env);
        } case NodeType::FOR_STATEMENT: {
            auto stmt = dynamic_cast<const ForStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalForStatement(stmt, env);
        } case NodeType::BREAK_STATEMENT: {
            auto stmt = dynamic_cast<const BreakStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalBreakStatement(stmt, env);
        } case NodeType::CONTINUE_STATEMENT: {
            auto stmt = dynamic_cast<const ContinueStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalContinueStatement(stmt, env);
        } case NodeType::SWITCH_STATEMENT: {
            auto stmt = dynamic_cast<const SwitchStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalSwitchStatement(stmt, env);
        } case NodeType::FUNCTION_DECLARE_STATEMENT: {
            auto stmt = dynamic_cast<const FunctionDeclareStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalFunctionDeclareStatement(stmt, env);
        } case NodeType::CALL_EXPRESSION: {
            auto expr = dynamic_cast<const CallExpression*>(node);
            if(!expr) { exitCode = 1; return nullptr; }
            return evalCallExpression(expr, env);
        } case NodeType::ARRAY_LITERAL: {
            auto lit = dynamic_cast<const ArrayLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalArrayLiteral(lit, env);
        } case NodeType::MAP_LITERAL: {
            auto lit = dynamic_cast<const MapLiteral*>(node);
            if(!lit) { exitCode = 1; return nullptr; }
            return evalMapLiteral(lit, env);
        } case NodeType::INDEX_EXPRESSION: {
            auto expr = dynamic_cast<const IndexExpression*>(node);
            if(!expr) { exitCode = 1; return nullptr; }
            return evalIndexExpression(expr, env);
        } case NodeType::IMPORT_STATEMENT: {
            auto stmt = dynamic_cast<const ImportStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalImportStatement(stmt, env);
        } case NodeType::EXPORT_STATEMENT: {
            auto stmt = dynamic_cast<const ExportStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalExportStatement(stmt, env);
        } case NodeType::ANNOTATION_STATEMENT: {
            auto stmt = dynamic_cast<const AnnotationStatement*>(node);
            if(!stmt) { exitCode = 1; return nullptr; }
            return evalAnnotationStatement(stmt, env);
        } case NodeType::AWAIT_EXPRESSION: {
            auto expr = dynamic_cast<const AwaitExpression*>(node);
            if(!expr) { exitCode = 1; return nullptr; }
            return evalAwaitExpression(expr, env);
        }

        default:
            return std::make_shared<ErrorObject>("unknow statement type");
    }

}

SPtr<Object> SyncRuntime::evalStatements(const Array<UPtr<Statement>> &stmts, SPtr<Environment> env) {
    SPtr<Object> res = std::make_shared<NilObject>();
    for(const auto& stmt : stmts) {
        res = eval(stmt.get(),env);
        // 遇到错误或者需要返回值的就返回
        if(res != nullptr && res->type() == ObjectType::RETURN) {
            return res;
        }
        if(res != nullptr && res->type() == ObjectType::ERROR) {
            return res;
        }
        if(res != nullptr && res->type() == ObjectType::BREAK) {
            return res;
        }
        if(res != nullptr && res->type() == ObjectType::CONTINUE) {
            return res;
        }
    }
    return res;
}

SPtr<Object> SyncRuntime::evalInteger(const IntegerLiteral *node) {
    // 现在的node是整数字面量
    i64 value;

    try {
        value = std::stoll(node->value);
    } catch(const std::invalid_argument& err ) {
        return std::make_shared<ErrorObject>("integer literal contains invalid character");
    } catch (const std::out_of_range& err) {
        return std::make_shared<ErrorObject>("integer literal out of ranges");
    }

    return std::make_unique<I64Object>(value);
}

SPtr<Object> SyncRuntime::evalFloating(const FloatLiteral *node) {
    // 现在的node是浮点字面量
    f64 value;

    try {
        value = std::stod(node->value);
    } catch(const std::invalid_argument& err ) {
        return std::make_shared<ErrorObject>("floating literal contains invalid character");
    } catch (const std::out_of_range& err) {
        return std::make_shared<ErrorObject>("floating literal out of ranges");
    }
    return std::make_shared<F64Object>(value);
}

SPtr<Object> SyncRuntime::evalString(const StringLiteral *node) {
    // 处理转义字符 TODO

    return std::make_shared<StringObject>(node->value);
}

SPtr<Object> SyncRuntime::evalBool(const BoolLiteral *node) {
    if(node->value) {
        // true
        return True;
    } else {
        return False;
    }
}

SPtr<Object> SyncRuntime::evalNull(const NullLiteral *node) {
    return Null;
}

SPtr<Object> SyncRuntime::evalFunctionLiteral(const FunctionLiteral *node, SPtr<Environment> env) {
    Array<String> paramNames;
    for(const auto& [paramName,paramType] : node->params) {
        (void)paramType;
        paramNames.push_back(static_cast<const Identifier*>(paramName.get())->value);
    }
    auto val = std::make_shared<FunctionObject>(
        std::move(paramNames),
        SPtr<BlockStatement>(const_cast<BlockStatement*>(node->body.get()),[](BlockStatement*){}),
        env
    );

    return val;
}

SPtr<Object> SyncRuntime::evalIdentifier(const Identifier *node, SPtr<Environment> env) {
    return env->get(node->value);
}

SPtr<Object> SyncRuntime::evalPrefixExpression(const PrefixExpression *node, SPtr<Environment> env) {
    String op = node->op;
    // + - ~
    SPtr<Object> right = eval(node->right.get(),env);
    if(op == "+") {
        // 这个i32,u32,i64,u64,f32,f64不报错，但是也不计算
        switch(right->type()) {
            case ObjectType::I32:
            case ObjectType::U32:
            case ObjectType::I64:
            case ObjectType::U64:
            case ObjectType::F32:
            case ObjectType::F64:
                return right;
            default:
                return std::make_shared<ErrorObject>("+ prefix operator type is not supported");
        }
    } else if(op == "-") {
        // 这个只有 i32, i64 , f32, f64 才支持
        switch(right->type()) {
            case ObjectType::I32: {
                auto val = static_cast<I32Object*>(right.get())->value;
                return std::make_shared<I32Object>(-val);
            }
            case ObjectType::I64: {
                auto val = static_cast<I64Object*>(right.get())->value;
                return std::make_shared<I64Object>(-val);
            }
            case ObjectType::F32: {
                auto val = static_cast<F32Object*>(right.get())->value;
                return std::make_shared<F32Object>(-val);
            }
            case ObjectType::F64: {
                auto val = static_cast<F64Object*>(right.get())->value;
                return std::make_shared<F64Object>(-val);
            }
            default:
                return std::make_shared<ErrorObject>("- prefix operator type is not supported");
        }
    } else if(op == "~") {
        // 这个只有i32,i64,u32,u64 才支持
        switch(right->type()) {
            case ObjectType::I32: {
                auto val = static_cast<I32Object*>(right.get())->value;
                return std::make_shared<I32Object>(~val);
            }
            case ObjectType::I64: {
                auto val = static_cast<I64Object*>(right.get())->value;
                return std::make_shared<I64Object>(~val);
            }
            case ObjectType::U32: {
                auto val = static_cast<U32Object*>(right.get())->value;
                return std::make_shared<U32Object>(~val);
            }
            case ObjectType::U64: {
                auto val = static_cast<U64Object*>(right.get())->value;
                return std::make_shared<U64Object>(~val);
            }
            default:
                return std::make_shared<ErrorObject>("~ prefix operator type is not supported");
        }
    } else {
        return std::make_shared<ErrorObject>("unknow prefix operator");
    }
}

SPtr<Object> SyncRuntime::evalInfixExpression(const InfixExpression *node, SPtr<Environment> env) {
    String op = node->op;
    
    if(op == "=") {
        return evalAssign(node,env);
    }
    auto left = eval(node->left.get(),env);
    auto right = eval(node->right.get(),env);
    return objectOperators(left,right,op);
}

SPtr<Object> SyncRuntime::evalAssign(const InfixExpression *node, SPtr<Environment> env) {
    // 1. 先求值右操作数
      auto right = eval(node->right.get(), env);
      if(right != nullptr && right->type() == ObjectType::ERROR) {
          return right;
      }

      // 2. 按左操作数节点类型分发写回
      auto leftNode = node->left.get();
      switch(leftNode->type()) {
          // x = 10 → 变量已在环境里：保持原地改值（写透外层作用域）
          case NodeType::IDENTIFIER: {
              auto name = static_cast<const Identifier*>(leftNode)->value;
              auto cur = env->get(name);
              if(cur != nullptr && cur->type() == ObjectType::ERROR) {
                  return cur;    // 变量不存在
              }
              return objectAssign(cur, right);
          }

          // p.y = 12 → 写回 struct 实例的字段
          case NodeType::MEMBER_ACCESS_EXPRESSION: {
              auto mem = static_cast<const MemberAccessExpression*>(leftNode);
              // 求值 object 部分（p）
              auto obj = eval(mem->object.get(), env);
              if(obj != nullptr && obj->type() == ObjectType::ERROR) {
                  return obj;
              }
              if(obj->type() != ObjectType::STRUCT) {
                  return std::make_shared<ErrorObject>("field assignment needs a struct instance");
              }
              if(mem->called->type() != NodeType::IDENTIFIER) {
                  return std::make_shared<ErrorObject>("field name must be an identifier");
              }
              auto inst = static_cast<StructObject*>(obj.get());
              String fieldName = static_cast<const Identifier*>(mem->called.get())->value;
              // 字段已存在 → 替换值；不存在 → 追加（容忍构造时缺省的字段）
              for(auto& [name, val] : inst->fields) {
                  if(name == fieldName) {
                      val = right;
                      return right;
                  }
              }
              inst->fields.push_back({fieldName, right});
              return right;
          }

          // list[i] = v / map[k] = v → 写回容器（顺带修好 list/map 赋值）
          case NodeType::INDEX_EXPRESSION: {
              auto idx = static_cast<const IndexExpression*>(leftNode);
              auto container = eval(idx->left.get(), env);
              if(container != nullptr && container->type() == ObjectType::ERROR) {
                  return container;
              }
              auto index = eval(idx->index.get(), env);
              if(index != nullptr && index->type() == ObjectType::ERROR) {
                  return index;
              }
              switch(container->type()) {
                  case ObjectType::LIST: {
                      auto arr = static_cast<ListObject*>(container.get());
                      auto i = toU64(index);
                      if(i >= arr->value.size()) {
                          return std::make_shared<ErrorObject>("Index out of range");
                      }
                      arr->value[i] = right;
                      return right;
                  }
                  case ObjectType::MAP: {
                      auto hash = static_cast<MapObject*>(container.get());
                      hash->value[index] = right;
                      return right;
                  }
                  default:
                      return std::make_shared<ErrorObject>("index assignment needs a list or map");
              }
          }

          default:
              return std::make_shared<ErrorObject>("= operator is not supported for this expression");
      }
}

SPtr<Object> SyncRuntime::evalBlockStatement(const BlockStatement *node, SPtr<Environment> env) {
    SPtr<Object> res;
    auto env_ = std::make_shared<Environment>(env); // 创建子作用域
    for(const auto& stmt : node->statements) {
        res = eval(stmt.get(),env_);
        if(res != nullptr && res->type() == ObjectType::ERROR) {
            return res;
        }
        if(res != nullptr && res->type() == ObjectType::RETURN) {
            return res;
        }
    }
    return res;
}

SPtr<Object> SyncRuntime::evalExpressionStatement(const ExpressionStatement *node, SPtr<Environment> env) {
    auto res = eval(node->expression.get(),env);
    if(res != nullptr && res->type() == ObjectType::ERROR) {
        return res;
    }
    return res;
}

SPtr<Object> SyncRuntime::evalLetStatement(const LetStatement *node, SPtr<Environment> env) {
    auto val = eval(node->value.get(),env);
    // varType交给ASTchecker解决
    if(val->type() == ObjectType::ERROR) {
        return val;
    }
    env->set(node->name->value,val);
    return Null;
}

SPtr<Object> SyncRuntime::evalConstStatement(const ConstStatement *node, SPtr<Environment> env) {
    // const    交给ASTchecker解决
    auto val = eval(node->value.get(),env);
    // varType  交给ASTchecker解决
    if(val->type() == ObjectType::ERROR) {
        return val;
    }
    env->set(node->name->value,val);
    return Null;
}

SPtr<Object> SyncRuntime::evalReturnStatement(const ReturnStatement *node, SPtr<Environment> env) {
    SPtr<Object> res;
    if(node->value != nullptr) {
        res = eval(node->value.get(),env);
    } else {
        res = Null;
    }
    return std::make_shared<ReturnObject>(std::move(res));
}

SPtr<Object> SyncRuntime::evalIfStatement(const IfStatement *node, SPtr<Environment> env) {
    auto fcond = eval(node->condition.get(),env);
    if(fcond != nullptr && fcond->type() == ObjectType::ERROR) {
        return fcond;
    }
    if(toBool(fcond) == true) {
        // 执行consequence
        // 创建新的 env 执行consequence
        auto env_ = std::make_shared<Environment>(env);
        return evalStatements(node->consequence->statements,env_);
    } else {
        // else if
        if(node->elifClauses.size() <= 0) {
            // 执行 alternative
            // 创建新的 env 执行 alternative
            if(node->alternative == nullptr) {
                return None;
            }
            auto env_ = std::make_shared<Environment>(env);
            return evalStatements(node->alternative->statements,env_);
        } else {
            for(const auto& [c,s] : node->elifClauses) {
                auto cond = eval(c.get(),env);
                if(toBool(cond) == true) {
                    // 创建新的 env 执行 elif
                    auto env_ = std::make_shared<Environment>(env);
                    return evalStatements(s->statements,env_);
                }
            }
            // 执行 alternative
            // 创建新的 env 执行 alternative
            if(node->alternative == nullptr) {
                return None;
            }
            auto env_ = std::make_shared<Environment>(env);
            return evalStatements(node->alternative->statements,env_);
        }
    }
}

SPtr<Object> SyncRuntime::evalWhileStatement(const WhileStatement *node, SPtr<Environment> env) {
    while(true) {
        auto cond = eval(node->condition.get(),env);
        // 先检查cond 再执行 body
        if(toBool(cond) == false) {
            return None;
        } else {
            // 创建一个新的env，执行body
            auto env_ = std::make_shared<Environment>(env);
            auto res = evalStatements(node->body->statements,env_);
            if(res != nullptr && res->type() == ObjectType::CONTINUE) {
                continue;
            }
            if(res != nullptr && res->type() == ObjectType::ERROR) {
                return res;
            }
            if(res != nullptr && res->type() == ObjectType::RETURN) {
                return res;
            }
            if(res != nullptr && res->type() == ObjectType::BREAK) {
                return res;
            }
            // 继续执行while循环
        }
    }
    return None;
}

SPtr<Object> SyncRuntime::evalForStatement(const ForStatement *node, SPtr<Environment> env) {
    // 只检查一次 init 
    // 后续循环先检查 cond 再执行body ， 最后执行 update
    auto ini = eval(node->init.get(),env);
    if(ini != nullptr && ini->type() == ObjectType::ERROR) {
        return ini;
    }
    
    while(true) {
        auto cond = eval(node->condition.get(),env);
        if(toBool(cond) == false) {
            break;
        } else {
            // 创建一个新的env，执行body
            auto env_ = std::make_shared<Environment>(env);
            auto res = evalStatements(node->body->statements,env_);
            if(res != nullptr && res->type() == ObjectType::ERROR) {
                return res;
            }
            if(res != nullptr && res->type() == ObjectType::RETURN) {
                return res;
            }
            if(res != nullptr && res->type() == ObjectType::BREAK) {
                return res;
            }
            // 继续执行for循环
        }
        // 执行 update
        auto up = eval(node->update.get(),env);
        if(up != nullptr && up->type() == ObjectType::ERROR) {
            return up;
        }
    }
    return Null;

}

SPtr<Object> SyncRuntime::evalBreakStatement(const BreakStatement *node, SPtr<Environment> env) {
    return std::make_shared<BreakObject>();
}

SPtr<Object> SyncRuntime::evalContinueStatement(const ContinueStatement *node, SPtr<Environment> env) {
    return std::make_shared<ContinueObject>();
}

SPtr<Object> SyncRuntime::evalSwitchStatement(const SwitchStatement *node, SPtr<Environment> env) {
    // 先获取cond的结果
    auto cond = eval(node->condition.get(),env);
    if(cond != nullptr && cond->type() == ObjectType::ERROR) {
        return cond;
    }
    // 获取匹配到的结果
    i32 idx = -1;
    for(i32 i = 0;i < node->cases.size();i++) {
        auto caseVal = eval(node->cases[i].first.get(),env);
        if(caseVal != nullptr && caseVal->type() == ObjectType::ERROR) {
            return caseVal;
        }
        auto eq = objectEqual(cond,caseVal);
        if(eq->type() == ObjectType::BOOL && toBool(eq) == true) {
            idx = i;
            break;
        } else {
            continue;
        }
    }

    // 没匹配到case
    if(idx == -1) {
        return Null;
    }

    // 执行匹配到case的body
    for(int i = idx;i<node->cases.size();i++) {
        // 创建新的env
        SPtr<Environment> env_ = std::make_shared<Environment>(env);
        auto res = evalStatements(node->cases[i].second->statements,env_);
        if(res != nullptr && res->type() == ObjectType::ERROR) {
            return res;
        }
        if(res != nullptr && res->type() == ObjectType::RETURN) {
            return static_cast<ReturnObject*>(res.get())->value;
        }
        if(res != nullptr && res->type() == ObjectType::BREAK) {
            return None;
        }
        // 否则执行下一个case
    }


    if(node->defaultCase == nullptr) {
        return None;
    }
    // 执行 defaultCase
    // 创建新的env
    SPtr<Environment> env_ = std::make_shared<Environment>(env);
    auto res = eval(node->defaultCase.get(),env_);
    if(res != nullptr && res->type() == ObjectType::ERROR) {
        return res;
    }
    if(res != nullptr && res->type() == ObjectType::RETURN) {
        return static_cast<ReturnObject*>(res.get())->value;
    }
    // 否则返回null
    return res; 
}

SPtr<Object> SyncRuntime::evalFunctionDeclareStatement(const FunctionDeclareStatement *node, SPtr<Environment> env) {
    // 将 function declare作为值存入环境
    // 参数列表：只取参数名，不拥有 AST 所有权
    Array<String> paramNames;
    for(const auto& [paramName, paramType] : node->params) {
        (void)paramType;
        if(paramName) {
            paramNames.push_back(static_cast<const Identifier*>(paramName.get())->value);
        }
    }
    auto val = std::make_shared<FunctionObject>(
        std::move(paramNames),
        std::shared_ptr<BlockStatement>(const_cast<BlockStatement*>(node->body.get()), [](BlockStatement*){}),
        env);
    env->set(node->name->value,val);

    return Null;
}

SPtr<Object> SyncRuntime::evalCallExpression(const CallExpression *node, SPtr<Environment> env) {
    // 求值被调用的对象
    auto callee = eval(node->func.get(),env);
    if(callee != nullptr && callee->type() == ObjectType::ERROR) {
        return callee;
    }
    
    // 获取参数列表
    Array<SPtr<Object>> args;
    for(const auto& argNode : node->arguments) {
        auto argVal = eval(argNode.get(),env);
        if(argVal != nullptr && argVal->type() == ObjectType::ERROR) {
            return argVal;
        }
        args.push_back(argVal);
    }

    // 按照类型分发
    switch(callee->type()) {
        case ObjectType::FUNCTION: {
            auto fn = static_cast<FunctionObject*>(callee.get());
            // 参数个数检查
            if(fn->params.size() != args.size()) {
                return std::make_shared<ErrorObject>("Call function with wrong number of arguments");
            }

            // 创建环境
            auto callEnv = std::make_shared<Environment>(fn->env);

            // 绑定参数
            for(i32 i = 0;i < fn->params.size();i++) {
                callEnv->set(fn->params[i],args[i]);
            }

            // 执行函数体
            auto res = evalStatements(fn->body->statements,callEnv);

            if(res != nullptr && res->type() == ObjectType::ERROR) {
                return res;
            }
            // 遇到 return：拆掉 ReturnObject 壳，取回真实值
            if(res != nullptr && res->type() == ObjectType::RETURN) {
                return static_cast<ReturnObject*>(res.get())->value;
            }
            return res;
        }
            
        case ObjectType::CONSTRUCTOR: {
            // 结构体构造器：Person({"x":10,"y":10})
            auto ctor = static_cast<ConstructorObject*>(callee.get());
            // 调转换回调：map → 实例
            return ctor->func(args);
        }
        case ObjectType::NATIVE: {
            auto native = static_cast<NativeObject*>(callee.get());
            
            return native->func(args);
        }
        default:
            return std::make_shared<ErrorObject>("not found the function");
    }
}

SPtr<Object> SyncRuntime::evalArrayLiteral(const ArrayLiteral *node, SPtr<Environment> env) {
    // 创建数组对象
    Array<SPtr<Object>> elems;
    for(const auto& elem : node->elements) {
        auto val = eval(elem.get(),env);
        if(val != nullptr && val->type() == ObjectType::ERROR) {
            return val;
        }
        elems.push_back(val);
    }

    // 返回数组对象
    return std::make_shared<ListObject>(std::move(elems));
}

SPtr<Object> SyncRuntime::evalMapLiteral(const MapLiteral *node, SPtr<Environment> env) {
    // 创建映射对象
    auto map = std::make_shared<MapObject>();

    // 创建映射遍历对
    for(const auto& [k,v] : node->pairs) {
        auto key = eval(k.get(),env);
        if(key != nullptr && key->type() == ObjectType::ERROR) {
            return key;
        }
        if(key != nullptr && !objectIn(key->type(),{ObjectType::STRING,ObjectType::I32,ObjectType::I64,ObjectType::U32,ObjectType::U64,ObjectType::F32,ObjectType::F64})) {
            return std::make_shared<ErrorObject>("Map key must be string , integer or floating point");
        }

        auto val = eval(v.get(),env);
        if(val != nullptr && val->type() == ObjectType::ERROR) {
            return val;
        }
        map->value[key] = val;
    }

    // 返回对象
    return map;
}

SPtr<Object> SyncRuntime::evalIndexExpression(const IndexExpression *node, SPtr<Environment> env) {
    // 求值被索引的对象
    auto left = eval(node->left.get(),env);
    if(left != nullptr && left->type() == ObjectType::ERROR) {
        return left;
    }

    // 求值索引对象
    auto index = eval(node->index.get(),env);
    if(index != nullptr && index->type() == ObjectType::ERROR) {
        return index;
    }

    switch(left->type()) {
        case ObjectType::LIST: {
            auto idx = toU64(index);
            auto arr = static_cast<ListObject*>(left.get());
            if(idx >= 0 && idx < arr->value.size()) {
                return arr->value.at(idx);
            } else {
                // 后期接越界处理
                return std::make_shared<ErrorObject>("Index out of range");
            }
        }
        case ObjectType::MAP: {
            auto hash = static_cast<MapObject*>(left.get());
            auto iter = hash->value.find(index);
            if(iter != hash->value.end()) {
                return iter->second;
            } else {
                // 返回空对象
                return Null;
            }
        }
        case ObjectType::STRING: {
            auto str = static_cast<StringObject*>(left.get());
            u64 idx = toU64(index);
            if(idx >= 0 && idx < str->value.size()) {
                return std::make_shared<StringObject>(String(1,str->value[idx]));
            } else {
                // 后期接越界处理
                return std::make_shared<ErrorObject>("Index out of range");
            }
        } 
        default:
            return std::make_shared<ErrorObject>("Index object not list or map or string");
    }


}

SPtr<Object> SyncRuntime::evalStructDeclareStatement(const StructDeclareStatement *node, SPtr<Environment> env) {
    // ========== 1. 构造类型定义对象 ==========
    auto def = std::make_shared<StructDeclObject>();
    def->structName = node->name->value;

    // 字段名 → 字段类型名，拷贝进类型定义
    for(const auto& [name, type] : node->fields) {
        if(name) {
            auto ident = static_cast<const Identifier*>(name.get());
            def->fieldsDecl.push_back({ident->value, type->name});
        }
    }

    // 注册类型定义到类型环境
    typeEnv.set(def->structName, def);

    // ========== 2. 注册构造器到变量环境 ==========
    // 构造器持有转换回调：捕获类型定义，接收 map 参数 → 建实例
    auto ctor = std::make_shared<ConstructorObject>(
        [def](const Array<SPtr<Object>>& args) -> SPtr<Object> {
            // 参数检查：必须一个 map 字面量
            if(args.size() != 1 || args[0]->type() != ObjectType::MAP) {
                return std::make_shared<ErrorObject>("constructor expects a map literal");
            }
            auto map = static_cast<MapObject*>(args[0].get());
            // 建实例，按字段声明顺序从 map 填值
            auto inst = std::make_shared<StructObject>();
            inst->structName = def->structName;
            for(const auto& [fieldName, fieldType] : def->fieldsDecl) {
                SPtr<Object> value = nullptr;
                // 在 map 里找同名 StringObject 键
                for(const auto& [key, val] : map->value) {
                    if(key->type() == ObjectType::STRING &&
                       static_cast<StringObject*>(key.get())->value == fieldName) {
                        value = val;
                        break;
                    }
                }
                // 字段缺省 → 全局 Nil 单例
                inst->fields.push_back({fieldName, value ? value : Null});
            }
            return inst;
        }
    );
    // 记录类型名，便于 inspect / 调试
    ctor->name = def->structName;
    env->set(def->structName, ctor);

    return Null;
}

SPtr<Object> SyncRuntime::evalMemberAccessExpression(const MemberAccessExpression *node, SPtr<Environment> env) {
    // 1. 求值 object 部分，判断是实例方法调用还是字段访问
    auto obj = eval(node->object.get(), env);
    if(obj != nullptr && obj->type() == ObjectType::ERROR) {
        return obj;
    }

    // 2. called 是 CallExpression → 方法调用 p.getName(...)
    if(node->called->type() == NodeType::CALL_EXPRESSION) {
        auto call = static_cast<const CallExpression*>(node->called.get());
        // 方法名是 call->func（Identifier）
        if(call->func->type() != NodeType::IDENTIFIER) {
            return std::make_shared<ErrorObject>("method name must be an identifier");
        }
        auto methodName = static_cast<const Identifier*>(call->func.get())->value;

        // 模块成员调用 Module.member(...) —— 查模块成员表
        if(obj->type() == ObjectType::MODULE) {
            auto mod = static_cast<ModuleObject*>(obj.get());
            auto it = mod->members.find(methodName);
            if(it == mod->members.end()) {
                return std::make_shared<ErrorObject>(
                    "module " + mod->moduleName + " has no member " + methodName);
            }
            auto member = it->second;

            // 求值参数
            Array<SPtr<Object>> args;
            for(const auto& argNode : call->arguments) {
                auto argVal = eval(argNode.get(), env);
                if(argVal != nullptr && argVal->type() == ObjectType::ERROR) {
                    return argVal;
                }
                args.push_back(argVal);
            }

            // 成员是可调用对象（NativeObject/FunctionObject）→ 走 evalCallExpression
            if(member->type() == ObjectType::NATIVE ||
               member->type() == ObjectType::FUNCTION ) {
                // 构造一个临时 CallExpression 不方便，直接按类型分发
                switch(member->type()) {
                case ObjectType::NATIVE: {
                    auto native = static_cast<NativeObject*>(member.get());
                    return native->func(args);
                }
                case ObjectType::FUNCTION: {
                    auto fn = static_cast<FunctionObject*>(member.get());
                    if(fn->params.size() != args.size()) {
                        return std::make_shared<ErrorObject>("Call function with wrong number of arguments");
                    }
                    auto callEnv = std::make_shared<Environment>(fn->env);
                    for(size_t i = 0; i < fn->params.size(); i++) {
                        callEnv->set(fn->params[i], args[i]);
                    }
                    auto res = evalStatements(fn->body->statements, callEnv);
                    if(res != nullptr && res->type() == ObjectType::ERROR) {
                        return res;
                    }
                    if(res != nullptr && res->type() == ObjectType::RETURN) {
                        return static_cast<ReturnObject*>(res.get())->value;
                    }
                    return res;
                }
                default:
                    break;
                }
            }
            return std::make_shared<ErrorObject>("module member is not callable");
        }

        // 只支持 struct 实例的方法调用
        if(obj->type() != ObjectType::STRUCT) {
            return std::make_shared<ErrorObject>("method call needs a struct instance");
        }
        auto inst = static_cast<StructObject*>(obj.get());

        // 求值显式参数（字段调用和方法调用都用）
        Array<SPtr<Object>> args;
        for(const auto& argNode : call->arguments) {
            auto argVal = eval(argNode.get(), env);
            if(argVal != nullptr && argVal->type() == ObjectType::ERROR) {
                return argVal;
            }
            args.push_back(argVal);
        }

        // 字段优先：方法名可能是存了 lambda/函数的字段（self.func(...)）
        for(const auto& [fname, fval] : inst->fields) {
            if(fname != methodName) continue;
            if(fval == nullptr || (fval->type() != ObjectType::FUNCTION && fval->type() != ObjectType::NATIVE)) {
                return std::make_shared<ErrorObject>(
                    inst->structName + " field " + methodName + " is not callable");
            }
            if(fval->type() == ObjectType::NATIVE) {
                auto native = static_cast<NativeObject*>(fval.get());
                return native->func(args);
            }
            auto fn = static_cast<FunctionObject*>(fval.get());
            if(fn->params.size() != args.size()) {
                return std::make_shared<ErrorObject>("Call function with wrong number of arguments");
            }
            auto callEnv = std::make_shared<Environment>(fn->env);
            for(size_t i = 0; i < fn->params.size(); i++) {
                callEnv->set(fn->params[i], args[i]);
            }
            auto res = evalStatements(fn->body->statements, callEnv);
            if(res != nullptr && res->type() == ObjectType::ERROR) {
                return res;
            }
            if(res != nullptr && res->type() == ObjectType::RETURN) {
                return static_cast<ReturnObject*>(res.get())->value;
            }
            return res;
        }

        // 从类型环境查方法
        auto method = typeEnv.getMethod(inst->structName, methodName);
        if(method == nullptr) {
            return std::make_shared<ErrorObject>(
                inst->structName + " has no method " + methodName);
        }
        if(method->type() != ObjectType::FUNCTION) {
            return std::make_shared<ErrorObject>("method is not a function");
        }
        auto fn = static_cast<FunctionObject*>(method.get());

        // 参数个数检查：params[0] 是 self，显式参数从 params[1] 开始
        if(fn->params.size() != args.size() + 1) {
            return std::make_shared<ErrorObject>("Call method with wrong number of arguments");
        }

        // 创建调用环境：外层 = 函数闭包环境
        auto callEnv = std::make_shared<Environment>(fn->env);

        // 绑定 self 为第一个参数
        callEnv->set(fn->params[0], obj);

        // 绑定显式参数
        for(size_t i = 0; i < args.size(); i++) {
            callEnv->set(fn->params[i + 1], args[i]);
        }

        // 执行方法体
        auto res = evalStatements(fn->body->statements, callEnv);
        if(res != nullptr && res->type() == ObjectType::ERROR) {
            return res;
        }
        // 遇到 return：拆掉 ReturnObject 壳，取回真实值
        if(res != nullptr && res->type() == ObjectType::RETURN) {
            return static_cast<ReturnObject*>(res.get())->value;
        }
        return res;
    }

    // 3. called 是 Identifier → 字段访问 p.age
    if(node->called->type() == NodeType::IDENTIFIER) {
        auto fieldName = static_cast<const Identifier*>(node->called.get())->value;

        // 模块成员访问 Module.member —— 查模块成员表
        if(obj->type() == ObjectType::MODULE) {
            auto mod = static_cast<ModuleObject*>(obj.get());
            auto it = mod->members.find(fieldName);
            if(it == mod->members.end()) {
                return std::make_shared<ErrorObject>(
                    "module " + mod->moduleName + " has no member " + fieldName);
            }
            return it->second;
        }

        // 只支持 struct 实例的字段访问
        if(obj->type() != ObjectType::STRUCT) {
            return std::make_shared<ErrorObject>("member access needs a struct instance");
        }
        auto inst = static_cast<StructObject*>(obj.get());

        // 在字段数组里按名查找
        for(const auto& [name, val] : inst->fields) {
            if(name == fieldName) {
                return val;
            }
        }
        return std::make_shared<ErrorObject>(
            inst->structName + " has no field " + fieldName);
    }

    return std::make_shared<ErrorObject>("unsupported member access");
}

SPtr<Object> SyncRuntime::evalImportStatement(const ImportStatement *node, SPtr<Environment> env) {
    auto moduleName = node->name->value;

    // 懒加载模块
    auto mod = moduleLoader_.loadModule(moduleName);

    if(mod != nullptr && mod->type() == ObjectType::ERROR) {
        return mod;
    }

    env->set(moduleName, mod);

    return None;
}

SPtr<Object> SyncRuntime::evalExportStatement(const ExportStatement *node, SPtr<Environment> env) {
    // export 现阶段无用：导出由模块系统处理，eval 层面不执行任何动作
    // 将来：模块加载器收集导出的名字
    (void)node;
    (void)env;
    return Null;
}

SPtr<Object> SyncRuntime::evalAnnotationStatement(const AnnotationStatement *node, SPtr<Environment> env) {
    // 注解包裹的 target 必须是函数声明
    if(node->target->type() != NodeType::FUNCTION_DECLARE_STATEMENT && node->target->type() != NodeType::STRUCT_DECLARE_STATEMENT) {
        return std::make_shared<ErrorObject>("annotation target must be a function declare");
    }
    auto fnStmt = static_cast<const FunctionDeclareStatement*>(node->target.get());

    // 构造 FunctionObject：params 只取参数名（含 self 占位符），body 用闭包环境
    Array<String> paramNames;
    for(const auto& [paramName, paramType] : fnStmt->params) {
        (void)paramType;
        if(paramName) {
            paramNames.push_back(static_cast<const Identifier*>(paramName.get())->value);
        }
    }
    auto fnObj = std::make_shared<FunctionObject>(
        std::move(paramNames),
        std::shared_ptr<BlockStatement>(const_cast<BlockStatement*>(fnStmt->body.get()), [](BlockStatement*){}),
        env);

    // 先收集注解标记，再统一注册（避免 @async @method 共存时重复注册）
    bool isAsync = false;
    String methodStruct;        // 非空表示 @method 归属的类型
    String nativePath;          // 非空表示 @native 的路径

    for(const auto& anno : node->annotations) {
        switch(anno->type()) {
            // @method(Person) —— 注册到类型方法表
            case NodeType::METHOD_ANNOTATION: {
                auto method = static_cast<const MethodAnnotation*>(anno.get());
                methodStruct = method->annoType->value;
                break;
            }
            // @async —— 标记异步（V2.0 并发调度用）
            case NodeType::ASYNC_ANNOTATION:
                isAsync = true;
                break;
            // @native('vm.file.ino') —— 注册 native 方法
            case NodeType::NATIVE_ANNOTATION: {
                auto native = static_cast<const NativeAnnotation*>(anno.get());
                auto implF = nativeRegistry_.get(native->path);
                if(!implF) {
                    // 未注册就是不存在
                    return std::make_shared<ErrorObject>("not found the function");
                }
                auto nativeObj = std::make_shared<NativeObject>(native->path,implF);
                nativeObj->isAsync = isAsync;
                env->set(fnStmt->name->value, nativeObj);
                nativePath = native->path;   // 标记 native 已注册，避免统一注册段重复注册 fnObj
                break;
            }
            // @platform / @object 预处理器处理，运行时忽略
            case NodeType::PLATFORM_ANNOTATION:
            case NodeType::OBJECT_ANNOTATION:
                break;
            default:
                break;
        }
    }

    // 按收集到的标记统一注册
    // 注意：@native 已在 switch 里注册 NativeObject，这里跳过，避免覆盖
    if(!nativePath.empty()) {
        // @native：已在 switch 里注册 NativeObject，无需重复
        // （nativePath 非空表示 native 分支已处理）
    } else if(!methodStruct.empty()) {
        // @method：注册到类型方法表（不进全局 env，避免重名冲突）
        fnObj->isAsync = isAsync;
        typeEnv.setMethod(methodStruct, fnStmt->name->value, fnObj);
    } else {
        // 普通函数（含纯 @async）：注册到 env
        fnObj->isAsync = isAsync;
        env->set(fnStmt->name->value, fnObj);
    }

    return Null;
}

SPtr<Object> SyncRuntime::evalAwaitExpression(const AwaitExpression *node, SPtr<Environment> env) {
    // V1.0 同步模式：无视 await，直接求值内部表达式
    // 将来 V2.0 并发调度：在这里挂起当前任务，等待异步结果
    return eval(node->argument.get(), env);
}

SPtr<Object> SyncRuntime::loadModuleFile(const String &path, const String &name) {
    std::ifstream file(path);
    if(!file.is_open()) {
        return std::make_shared<ErrorObject>("module file not found: " + path);
    }

    std::stringstream ss;
    ss << file.rdbuf();
    String source = ss.str();

    // 解析源码
    UPtr<Lexer> l = std::make_unique<Lexer>(source);

    Parser p(std::move(l));
    auto node = p.parseProgram();
    // 执行优化
    executeMiddleware(node.get());

    // 独立env
    auto moduleEnv = std::make_shared<Environment>(nullptr);

    auto result = eval(node.get(),moduleEnv);

    if(result != nullptr && result->type() == ObjectType::ERROR) {
        return result;
    }

    // 构造moduleObject
    auto mod = std::make_shared<ModuleObject>();
    mod->moduleName = name;
    collectExports(node.get(),moduleEnv,mod);
    return mod;

}

void SyncRuntime::collectExports(const Program *program, SPtr<Environment> env, SPtr<ModuleObject> mod) {
    for(const auto& stmt : program->statements) {
        if(stmt->type() == NodeType::EXPORT_STATEMENT) {
            auto exp = static_cast<const ExportStatement* >(stmt.get());
            for(const auto& ident : exp->exports) {
                auto val = env->get(ident->value);
                mod->members[ident->value] = val;
            }
        }
    }
}

void SyncRuntime::executeMiddleware(const Program *program) {
    pass
}
