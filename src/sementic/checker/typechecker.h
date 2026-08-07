#ifndef __TYPE_CHECKER_H__
#define __TYPE_CHECKER_H__

#include <src/marco.h>
#include <src/type.h>
#include <src/ast/ast.hpp>
#include <src/ast/parser.h>
#include <src/error/errorinfo.hpp>
#include <functional>
#include <stack>


class TypeChecker {
public:
    // 模块自加载回调：由调用方注入，读文件 + parse，返回 AST
    // 不依赖 interpreter 的 ModuleLoader / ModuleObject
    using FileLoader = std::function<SPtr<Program>(const String& path, const String& name)>;
    void setFileLoader(FileLoader fn);

    // 入口：检查一个 Program，返回错误列表
    // 副作用：把推导出的类型回填进节点的 TypeInfo
    Array<ErrorInfo> check(Program* program);

    // 错误是否为空
    bool hasErrors() const { return !errors_.empty(); }

private:
    //类型环境

    // 变量名 -> TypeInfo
    struct Scope {
        Map<String, TypeInfo> vars;
    };
    std::stack<Scope> scopes_;

    // 类型名 -> 字段表（struct 声明登记）
    Map<String, Array<Pair<String, SPtr<TypeLiteral>>>> structs_;
    // 函数名 -> 签名（参数类型表, 返回类型）——含模块导出
    struct FuncSig {
        Array<SPtr<TypeLiteral>> params;
        SPtr<TypeLiteral> ret;
    };
    Map<String, FuncSig> funcs_;
    // 模块名 -> 该模块导出的名字 -> 类型
    Map<String, Map<String, SPtr<TypeLiteral>>> modules_;
    // 正在检查的模块（循环检测）
    Set<String> checkingModules_;

    FileLoader fileLoader_;
    Array<ErrorInfo> errors_;

    //报错
    void error(const String& msg);

    //作用域
    void pushScope();
    void popScope();
    // 登记变量类型；重复声明报错
    void declareVar(const String& name, SPtr<TypeLiteral> type, bool explicitSrc);
    // 查变量类型，未找到返回 nullptr
    SPtr<TypeLiteral> lookupVar(const String& name);

    // 推导表达式类型，并回填对应节点的 TypeInfo。返回 nullptr 表示推导不出。
    SPtr<TypeLiteral> deriveExpr(Expression* node);

    // 各节点推导
    SPtr<TypeLiteral> deriveArray(ArrayLiteral* node);
    SPtr<TypeLiteral> deriveMap(MapLiteral* node);
    SPtr<TypeLiteral> deriveIdentifier(Identifier* node);
    SPtr<TypeLiteral> deriveInfix(InfixExpression* node);
    SPtr<TypeLiteral> deriveCall(CallExpression* node);
    SPtr<TypeLiteral> deriveIndex(IndexExpression* node);
    SPtr<TypeLiteral> deriveMember(MemberAccessExpression* node);

    // 语句检查 
    void checkStatement(Statement* stmt);
    void checkLetStatement(LetStatement* node);
    void checkConstStatement(ConstStatement* node);
    void checkFunctionDeclareStatement(FunctionDeclareStatement* node);
    void checkReturnStatement(ReturnStatement* node);
    void checkIfStatement(IfStatement* node);
    void checkWhileStatement(WhileStatement* node);
    void checkForStatement(ForStatement* node);
    void checkSwitchStatement(SwitchStatement* node);
    void checkBlockStatement(BlockStatement* node);
    void checkExpressionStatement(ExpressionStatement* node);
    void checkImportStatement(ImportStatement* node);
    void checkAssign(InfixExpression* node);
    // 仿 eval 自加载：import -> checkModule 递归检查模块，登记导出类型
    void checkModule(const String& moduleName, const String& path);
    // 便利：内置类型名 -> TypeLiteral
    static SPtr<TypeLiteral> builtinType(const String& name);

    // 类型兼容
    // 核心：source 类型能否赋给 target 类型
    static bool isAssignable(const SPtr<TypeLiteral>& target, const SPtr<TypeLiteral>& source);
    // 是否是内置基本类型名
    static bool isBuiltinName(const String& name);
};

#endif
