#ifndef __SYNCRUNTIME_H__
#define __SYNCRUNTIME_H__

#include <src/marco.h>
#include <src/type.h>
#include "./env.hpp"
#include <src/object/object.hpp>
#include <src/native/native.h>
#include "module.h"


class SyncRuntime {
    SPtr<Environment> globalEnv;        // 全局环境、内置方法注册到这里
    TypeEnvironment typeEnv;            // 类型环境、结构体类型定义和方法注册到这里
    NativeRegistry nativeRegistry;      // native 方法注册表，path -> func
    u32 exitCode;                       // 退出码，出现无法处理的异常的时候的退出码
    Array<UPtr<Program>> programPool_;  // 程序池，缓存已解析的程序，适用于多文件或者repl的场景
    ModuleLoader moduleLoader_;          // 模块加载器，负责加载模块
    NativeRegistry nativeRegistry_;      // native 方法注册表，path -> func
    static SPtr<BoolObject> True;
    static SPtr<BoolObject> False;
    static SPtr<NilObject>  Null;
    static SPtr<NoneObject> None;               
public:
    SyncRuntime();
    ~SyncRuntime() = default;
    u32 run(const String& content,String& res);
private:
    SPtr<Object> eval(const Node* node, SPtr<Environment> env);    // 全局运行器
    SPtr<Object> evalStatements(const Array<UPtr<Statement>>& stmts , SPtr<Environment> env);

    // 整数字面量求值
    SPtr<Object> evalInteger(const IntegerLiteral* node );
    
    // 浮点字面量求值
    SPtr<Object> evalFloating(const FloatLiteral* node );

    // 字符串字面量求值
    SPtr<Object> evalString(const StringLiteral* node );
    
    // 布尔字面量求值
    SPtr<Object> evalBool(const BoolLiteral* node );

    // nil 字面量求值
    SPtr<Object> evalNull(const NullLiteral* node );
    
    // 纯标识符
    SPtr<Object> evalIdentifier(const Identifier* node, SPtr<Environment> env);

    // prefix
    SPtr<Object> evalPrefixExpression(const PrefixExpression* node, SPtr<Environment> env);

    // infix
    SPtr<Object> evalInfixExpression(const InfixExpression* node, SPtr<Environment> env);

    // block
    SPtr<Object> evalBlockStatement(const BlockStatement* node, SPtr<Environment> env);
    
    // expression statement
    SPtr<Object> evalExpressionStatement(const ExpressionStatement* node, SPtr<Environment> env);

    // let 
    SPtr<Object> evalLetStatement(const LetStatement* node, SPtr<Environment> env);

    // const 
    SPtr<Object> evalConstStatement(const ConstStatement* node, SPtr<Environment> env);

    // return statement
    SPtr<Object> evalReturnStatement(const ReturnStatement* node, SPtr<Environment> env);

    // if statement
    SPtr<Object> evalIfStatement(const IfStatement* node, SPtr<Environment> env);
    
    // while statement
    SPtr<Object> evalWhileStatement(const WhileStatement* node, SPtr<Environment> env);

    // for statement
    SPtr<Object> evalForStatement(const ForStatement* node, SPtr<Environment> env);

    // break statement
    SPtr<Object> evalBreakStatement(const BreakStatement* node, SPtr<Environment> env);

    // continue statement
    SPtr<Object> evalContinueStatement(const ContinueStatement* node, SPtr<Environment> env);

    // switch statement
    SPtr<Object> evalSwitchStatement(const SwitchStatement* node, SPtr<Environment> env);

    // function declare statement
    SPtr<Object> evalFunctionDeclareStatement(const FunctionDeclareStatement* node, SPtr<Environment> env);
    
    // call expression
    SPtr<Object> evalCallExpression(const CallExpression* node, SPtr<Environment> env);
    
    // array literal
    SPtr<Object> evalArrayLiteral(const ArrayLiteral* node, SPtr<Environment> env);

    // map literal
    SPtr<Object> evalMapLiteral(const MapLiteral* node, SPtr<Environment> env);

    // index expression
    SPtr<Object> evalIndexExpression(const IndexExpression* node, SPtr<Environment> env);

    // struct declare statement
    SPtr<Object> evalStructDeclareStatement(const StructDeclareStatement* node, SPtr<Environment> env);
    
    // member access expression
    SPtr<Object> evalMemberAccessExpression(const MemberAccessExpression* node, SPtr<Environment> env);

    // import statement
    SPtr<Object> evalImportStatement(const ImportStatement* node, SPtr<Environment> env);

    // export statement
    SPtr<Object> evalExportStatement(const ExportStatement* node, SPtr<Environment> env);

    // annotation statement（@method/@native/@async/@platform/@object）
    SPtr<Object> evalAnnotationStatement(const AnnotationStatement* node, SPtr<Environment> env);

    // await expression（V1.0 同步模式：直接求值内部表达式）
    SPtr<Object> evalAwaitExpression(const AwaitExpression* node, SPtr<Environment> env);

    // 加载模块
    SPtr<Object> loadModuleFile(const String& path , const String& name);
    
    // 收集export
    void collectExports(const Program* program , SPtr<Environment> env, SPtr<ModuleObject> mod);

};

#endif