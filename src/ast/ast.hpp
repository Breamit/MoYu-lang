#ifndef __AST_HPP__
#define __AST_HPP__

#include <sstream>
#include <src/marco.h>
#include <src/type.h>
#include <src/lexer/lexer.h>
// 预先声明
enum class NodeType;

// 接口
interface Node;
interface Expression;
interface Statement;
interface Annotation;

// 实现类
// 程序相关
class Program;                      // 程序节点
class Identifier;                   // 标识符

// 类型相关
class IntegerLiteral;               // 整数字面量
class FloatLiteral;                 // 浮点数字面量
class BoolLiteral;                  // 布尔字面量
class StringLiteral;                // 字符串字面量
class ArrayLiteral;                 // 数组字面量
class MapLiteral;                   // 哈希表字面量
class FunctionLiteral;              // 函数字面量, 常见于闭包
class NullLiteral;                  // null 字面量
class TypeLiteral;                  // 类型字面量

// 关键字相关
class LetStatement;                 // let 语句
class ConstStatement;               // const 语句
class ForStatement;                 // for 语句
class WhileStatement;               // while 语句
class IfStatement;                  // if 语句
class SwitchStatement;              // switch 语句
class FunctionDeclareStatement;     // 函数声明语句
class StructDeclareStatement;       // 结构体声明语句
class EnumDeclareStatement;         // 枚举声明语句
class ImportStatement;              // 导包语句
class ExportStatement;              // 导出语句
class ReturnStatement;              // 返回语句
class BreakStatement;               // break 语句
class ContinueStatement;            // continue 语句


// 语法特性相关
class ExpressionStatement;          // 表达式语句
class PrefixExpression;             // 前缀表达式
class InfixExpression;              // 中缀表达式
class BlockStatement;               // 块语句
class CallExpression;               // 函数调用
class IndexExpression;              // 索引访问语句
class AwaitExpression;              // await语句
class MemberAccessExpression;       // 成员访问语句

// 注解语句
class AnnotationStatement;          // 注解语句
class MethodAnnotation;             // 方法注解语句
class PlatformAnnotation;           // 平台注解语句
class NativeAnnotation;             // 原生注解语句
class AsyncAnnotation;              // 异步注解
class ObjectAnnotation;             // 对象注解



enum class NodeType {
    // 程序结构
    PROGRAM = 0,

    // 基础类型
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,
    ARRAY_LITERAL,
    MAP_LITERAL,
    FUNCTION_LITERAL,
    NULL_LITERAL,
    TYPE_LITERAL,

    // 语句
    LET_STATEMENT,
    CONST_STATEMENT,
    FOR_STATEMENT,
    WHILE_STATEMENT,
    IF_STATEMENT,
    SWITCH_STATEMENT,
    FUNCTION_DECLARE_STATEMENT,
    STRUCT_DECLARE_STATEMENT,
    ENUM_DECLARE_STATEMENT,
    IMPORT_STATEMENT,
    EXPORT_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    BLOCK_STATEMENT,
    EXPRESSION_STATEMENT,

    // 表达式
    PREFIX_EXPRESSION,
    INFIX_EXPRESSION,
    CALL_EXPRESSION,
    INDEX_EXPRESSION,
    AWAIT_EXPRESSION,
    MEMBER_ACCESS_EXPRESSION,

    // 注解
    ANNOTATION_STATEMENT,
    METHOD_ANNOTATION,
    PLATFORM_ANNOTATION,
    NATIVE_ANNOTATION,
    ASYNC_ANNOTATION,
    OBJECT_ANNOTATION
};

// JSON 字符串转义辅助
inline String escapeJson(const String& s) {
    String r;
    r.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n";  break;
            case '\t': r += "\\t";  break;
            case '\r': r += "\\r";  break;
            default:   r += c;
        }
    }
    return r;
}

// 两个方法需要实现
interface Node {
public:
    virtual ~Node() = default;
    virtual NodeType type() const = 0;
    virtual String tokenLiteral() const = 0;
    virtual String toString() const = 0;
};

// 一个方法需要实现
interface Expression : public Node {
public:
    virtual ~Expression() = default;
    virtual void expressionNode() = 0;
};

// 一个方法需要实现
interface Statement : public Node {
public:
    virtual ~Statement() = default;
    virtual void statementNode() = 0;
};

interface Annotation : public Node {
public:
    virtual ~Annotation() = default;
    virtual void annotationNode() = 0;
};

// 块语句 — 提前定义，供所有包含 BlockStatement 成员的类使用
class BlockStatement : impl Statement {
public:
    Array<UPtr<Statement>> statements;
    Token token;
    void statementNode() override {}
    NodeType type() const {
        return NodeType::BLOCK_STATEMENT;
    }
    String tokenLiteral() const override {
        return "{";
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"BLOCK_STATEMENT\"";
        oss << ",\"statements\":[";
        for (size_t i = 0; i < statements.size(); i++) {
            if (i > 0) oss << ",";
            if (statements[i]) oss << statements[i]->toString();
            else oss << "null";
        }
        oss << "]}";
        return oss.str();
    }
    ~BlockStatement() = default;
};


// 程序对象，所有需要运行的代码的AST结点都需要挂在这个类下面
class Program : impl Node {
public:
    Array<UPtr<Statement>> statements;
    NodeType type() const override {
        return NodeType::PROGRAM;
    }
    String tokenLiteral() const override {
        return "program";
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"PROGRAM\"";
        oss << ",\"statements\":[";
        for (size_t i = 0; i < statements.size(); i++) {
            if (i > 0) oss << ",";
            if (statements[i]) oss << statements[i]->toString();
            else oss << "null";
        }
        oss << "]}";
        return oss.str();
    }
    ~Program() = default;
};

// 标识符对象
class Identifier : impl Expression {
public:
    Token token;
    String value;
    NodeType type() const {
        return NodeType::IDENTIFIER;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"IDENTIFIER\"";
        oss << ",\"value\":\"" << escapeJson(value) << "\"";
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~Identifier() = default;
};

/**
 * 字面量对象
 */

// 整数字面量
class IntegerLiteral : impl Expression {
public:
    Token token;
    String value;   // AST阶段不区分 byte , i32 , i64 等
    NodeType type() const {
        return NodeType::INTEGER_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"INTEGER_LITERAL\"";
        oss << ",\"value\":" << value;
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~IntegerLiteral() = default;
};

// 浮点数字面量
class FloatLiteral : impl Expression {
public:
    Token token;
    String value;
    NodeType type() const {
        return NodeType::FLOAT_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"FLOAT_LITERAL\"";
        oss << ",\"value\":" << value;
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~FloatLiteral() = default;
};

// 布尔字面量
class BoolLiteral : impl Expression {
public:
    Token token;
    bool value;
    NodeType type() const {
        return NodeType::BOOL_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"BOOL_LITERAL\"";
        oss << ",\"value\":" << (value ? "true" : "false");
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~BoolLiteral() = default;
};

// 字符串字面量
class StringLiteral : impl Expression {
public:
    Token token;
    String value;
    NodeType type() const {
        return NodeType::STRING_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"STRING_LITERAL\"";
        oss << ",\"value\":\"" << escapeJson(value) << "\"";
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~StringLiteral() = default;
};

// 数组字面量
class ArrayLiteral : impl Expression {
public:
    Token token;
    Array<UPtr<Expression>> elements;
    NodeType type() const {
        return NodeType::ARRAY_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"ARRAY_LITERAL\"";
        oss << ",\"elements\":[";
        for (size_t i = 0; i < elements.size(); i++) {
            if (i > 0) oss << ",";
            if (elements[i]) oss << elements[i]->toString();
            else oss << "null";
        }
        oss << "]}";
        return oss.str();
    }
    void expressionNode() override {}
    ~ArrayLiteral() = default;
};

// 哈希表字面量
class MapLiteral : impl Expression {
public:
    Token token;
    Array<Pair<UPtr<Expression>,UPtr<Expression>>> pairs;
    NodeType type() const {
        return NodeType::MAP_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"MAP_LITERAL\"";
        oss << ",\"pairs\":[";
        for (size_t i = 0; i < pairs.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"key\":";
            if (pairs[i].first) oss << pairs[i].first->toString();
            else oss << "null";
            oss << ",\"value\":";
            if (pairs[i].second) oss << pairs[i].second->toString();
            else oss << "null";
            oss << "}";
        }
        oss << "]}";
        return oss.str();
    }
    void expressionNode() override {}
    ~MapLiteral() = default;
};

// 函数字面量
class FunctionLiteral : impl Expression {
public:
    Token token;
    Array<UPtr<Expression>> params;
    UPtr<BlockStatement> body;
    NodeType type() const {
        return NodeType::FUNCTION_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"FUNCTION_LITERAL\"";
        oss << ",\"params\":[";
        for (size_t i = 0; i < params.size(); i++) {
            if (i > 0) oss << ",";
            if (params[i]) oss << params[i]->toString();
            else oss << "null";
        }
        oss << "]";
        oss << ",\"body\":";
        if (body) oss << body->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~FunctionLiteral() = default;
};

// null 字面量
class NullLiteral : impl Expression {
public:
    NodeType type() const {
        return NodeType::NULL_LITERAL;
    }
    String tokenLiteral() const override {
        return "null";
    }
    String toString() const override {
        return "{\"type\":\"NULL_LITERAL\"}";
    }
    void expressionNode() override {}
    ~NullLiteral() = default;
};

class TypeLiteral : impl Expression {
public:
    Token token;
    String name;                           // 类型名字符串，如 "i32"、"string"、"list"、"Person"
    Array<UPtr<TypeLiteral>> typeParams;   // 泛型参数，如 list<i32> → [i32]; map<string,i32> → [string,i32]
    NodeType type() const {
        return NodeType::TYPE_LITERAL;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"TYPE_LITERAL\"";
        oss << ",\"name\":\"" << escapeJson(name) << "\"";
        if (!typeParams.empty()) {
            oss << ",\"typeParams\":[";
            for (size_t i = 0; i < typeParams.size(); i++) {
                if (i > 0) oss << ",";
                if (typeParams[i]) oss << typeParams[i]->toString();
                else oss << "null";
            }
            oss << "]";
        }
        oss << "}";
        return oss.str();
    }
    void expressionNode() override {}
    ~TypeLiteral() = default;
};

/**
 * 语句对象
 */

 // let
class LetStatement : impl Statement {
public:
    //all  required
    Token token;    // let
    UPtr<Identifier> name;  // 变量名
    UPtr<TypeLiteral> varType; // 类型标注
    UPtr<Expression> value; // 赋予的值
    void statementNode()override {}
    NodeType type() const {
        return NodeType::LET_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"LET_STATEMENT\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"varType\":";
        if (varType) oss << varType->toString();
        else oss << "null";
        oss << ",\"value\":";
        if (value) oss << value->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~LetStatement() = default;
};

// const
class ConstStatement : impl Statement {
public:
    //all  required
    Token token;    // const
    UPtr<Identifier> name;  // 变量名
    UPtr<TypeLiteral> varType; // 类型标注
    UPtr<Expression> value; // 赋予的值
    void statementNode()override {}
    NodeType type() const {
        return NodeType::CONST_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"CONST_STATEMENT\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"varType\":";
        if (varType) oss << varType->toString();
        else oss << "null";
        oss << ",\"value\":";
        if (value) oss << value->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~ConstStatement() = default;
};

// for
class ForStatement : impl Statement {
public:
    // for in 会被翻译成下面的
    Token token;
    UPtr<Statement> init;   //  初始化
    UPtr<Expression> condition; // 条件
    UPtr<Expression> update;    // 步进
    UPtr<BlockStatement> body;  // 循环体
    void statementNode() override {}
    NodeType type() const {
        return NodeType::FOR_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"FOR_STATEMENT\"";
        oss << ",\"init\":";
        if (init) oss << init->toString();
        else oss << "null";
        oss << ",\"condition\":";
        if (condition) oss << condition->toString();
        else oss << "null";
        oss << ",\"update\":";
        if (update) oss << update->toString();
        else oss << "null";
        oss << ",\"body\":";
        if (body) oss << body->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~ForStatement() =  default;
};

// while
class WhileStatement : impl Statement {
public:
    Token token;
    UPtr<Expression> condition;
    UPtr<BlockStatement> body;
    void statementNode()override {}
    NodeType type() const {
        return NodeType::WHILE_STATEMENT;
    }
    String tokenLiteral() const override {
        return "while";
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"WHILE_STATEMENT\"";
        oss << ",\"condition\":";
        if (condition) oss << condition->toString();
        else oss << "null";
        oss << ",\"body\":";
        if (body) oss << body->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~WhileStatement() = default;
};

// if
class IfStatement : impl Statement {
public:
    Token token;    // if
    UPtr<Expression> condition;
    UPtr<BlockStatement> consequence;
    // elif部分
    Array<Pair<UPtr<Expression>,UPtr<BlockStatement>>> elifClauses;
    // else部分
    UPtr<BlockStatement> alternative;
    void statementNode() override {}
    NodeType type() const {
        return NodeType::IF_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"IF_STATEMENT\"";
        oss << ",\"condition\":";
        if (condition) oss << condition->toString();
        else oss << "null";
        oss << ",\"consequence\":";
        if (consequence) oss << consequence->toString();
        else oss << "null";
        oss << ",\"elifClauses\":[";
        for (size_t i = 0; i < elifClauses.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"condition\":";
            if (elifClauses[i].first) oss << elifClauses[i].first->toString();
            else oss << "null";
            oss << ",\"body\":";
            if (elifClauses[i].second) oss << elifClauses[i].second->toString();
            else oss << "null";
            oss << "}";
        }
        oss << "]";
        oss << ",\"alternative\":";
        if (alternative) oss << alternative->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~IfStatement() = default;
};

class SwitchStatement : impl Statement {
public:
    Token token;    // switch
    UPtr<Expression> condition;
    Array<Pair<UPtr<Expression>,UPtr<BlockStatement>>> cases;
    UPtr<BlockStatement> defaultCase;
    void statementNode() override {}
    NodeType type() const {
        return NodeType::SWITCH_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"SWITCH_STATEMENT\"";
        oss << ",\"condition\":";
        if (condition) oss << condition->toString();
        else oss << "null";
        oss << ",\"cases\":[";
        for (size_t i = 0; i < cases.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"condition\":";
            if (cases[i].first) oss << cases[i].first->toString();
            else oss << "null";
            oss << ",\"body\":";
            if (cases[i].second) oss << cases[i].second->toString();
            else oss << "null";
            oss << "}";
        }
        oss << "]";
        oss << ",\"defaultCase\":";
        if (defaultCase) oss << defaultCase->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~SwitchStatement() = default;
};

// @annotation
// fn demo(param1 : i32 , param2 : f64):void {}
class FunctionDeclareStatement : impl Statement {
public:
    Token token;    // fn
    UPtr<Identifier> name;  // 函数名
    Array<Pair<UPtr<Expression>,UPtr<TypeLiteral>>> params; // 参数
    UPtr<TypeLiteral> returnType; // 返回类型
    UPtr<BlockStatement> body; // 函数体

    void statementNode() override {}
    NodeType type() const {
        return NodeType::FUNCTION_DECLARE_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"FUNCTION_DECLARE_STATEMENT\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"params\":[";
        for (size_t i = 0; i < params.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"name\":";
            if (params[i].first) oss << params[i].first->toString();
            else oss << "null";
            oss << ",\"type\":";
            if (params[i].second) oss << params[i].second->toString();
            else oss << "null";
            oss << "}";
        }
        oss << "]";
        oss << ",\"body\":";
        if (body) oss << body->toString();
        else oss << "null";
        oss << ",\"returnType\":";
        if (returnType) oss << returnType->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~FunctionDeclareStatement() = default;
};

/**
 * struct Person {
 * name: string;
 * age: i32;
 * };
 */

class StructDeclareStatement : impl Statement {
public:
    Token token;    // struct
    UPtr<Identifier> name;  // 结构体名
    // 结构体本体是KV键值对 ， age : i32
    Array<Pair<UPtr<Expression>,UPtr<TypeLiteral>>> fields;
    void statementNode() override {}
    NodeType type() const {
        return NodeType::STRUCT_DECLARE_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"STRUCT_DECLARE_STATEMENT\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"fields\":[";
        for (size_t i = 0; i < fields.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"name\":";
            if (fields[i].first) oss << fields[i].first->toString();
            else oss << "null";
            oss << ",\"type\":";
            if (fields[i].second) oss << fields[i].second->toString();
            else oss << "null";
            oss << "}";
        }
        oss << "]";
        oss << "}";
        return oss.str();
    }
    ~StructDeclareStatement() = default;
};

/**
 * enum Color {
 * white = 0,
 * black
 * };
 */
class EnumDeclareStatement : impl Statement {
public:
    Token token;    // enum
    UPtr<Identifier> name;  // 枚举名
    // 内部是顺序标识符
    Array<UPtr<Identifier>> values;
    void statementNode() override {}
    NodeType type() const {
        return NodeType::ENUM_DECLARE_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"ENUM_DECLARE_STATEMENT\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"values\":[";
        for (size_t i = 0; i < values.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"name\":";
            if (values[i]) oss << values[i]->toString();
            else oss << "null";
            oss << "}";
        }
        oss << "]";
        oss << "}";
        return oss.str();
    }
    ~EnumDeclareStatement() = default;
};

// import
class ImportStatement : impl Statement {
public:
    Token token;    // import
    UPtr<Identifier> name;  // 导入模块名，导入项目内其他文件需要在config.json配置别名 项目名_模块名
    void statementNode() override {}
    NodeType type() const {
        return NodeType::IMPORT_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"IMPORT_STATEMENT\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~ImportStatement() = default;
};

// export
class ExportStatement : impl Statement {
public:
    Token token;    // export
    Array<UPtr<Identifier>> exports;  // 导出项目
    void statementNode() override {}
    NodeType type() const {
        return NodeType::EXPORT_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"EXPORT_STATEMENT\"";
        oss << ",\"body\":[";
        for(size_t i = 0;i<exports.size();i++) {
            oss<<exports[i].get()->toString();
            if(i<exports.size() - 1) {
                oss<<",";
            }
        }
        oss << "]}";
        return oss.str();
    }
    ~ExportStatement() = default;
};

// return
class ReturnStatement : impl Statement {
public:
    Token token;    // return
    UPtr<Expression> value; // 返回值
    void statementNode() override {}
    NodeType type() const {
        return NodeType::RETURN_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"RETURN_STATEMENT\"";
        oss << ",\"value\":";
        if (value) oss << value->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~ReturnStatement() = default;
};

// break
class BreakStatement : impl Statement {
public:
    Token token;    // break
    void statementNode() override {}
    NodeType type() const {
        return NodeType::BREAK_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        return "{\"type\":\"BREAK_STATEMENT\"}";
    }
    ~BreakStatement() = default;
};

// continue
class ContinueStatement : impl Statement {
public:
    Token token;    // continue
    void statementNode() override {}
    NodeType type() const {
        return NodeType::CONTINUE_STATEMENT;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        return "{\"type\":\"CONTINUE_STATEMENT\"}";
    }
    ~ContinueStatement() = default;
};

// 表达式语句
class ExpressionStatement : impl Statement {
public:
    Token token;
    UPtr<Expression> expression;
    void statementNode() override {}
    NodeType type() const {
        return NodeType::EXPRESSION_STATEMENT;
    }
    String tokenLiteral() const override {
        if (expression) return expression->tokenLiteral();
        return "";
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"EXPRESSION_STATEMENT\"";
        oss << ",\"expression\":";
        if (expression) oss << expression->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~ExpressionStatement() = default;
};

// 前缀表达式
class PrefixExpression : impl Expression {
public:
    Token token;    // 前缀运算符
    String op;      // 运算符
    UPtr<Expression> right;
    void expressionNode() override {}
    NodeType type() const {
        return NodeType::PREFIX_EXPRESSION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"PREFIX_EXPRESSION\"";
        oss << ",\"op\":\"" << escapeJson(op) << "\"";
        oss << ",\"right\":";
        if (right) oss << right->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~PrefixExpression() = default;
};

// 中缀表达式
class InfixExpression : impl Expression {
public:
    Token token;    // 中缀运算符
    String op;      // 运算符
    UPtr<Expression> left;
    UPtr<Expression> right;
    void expressionNode() override {}
    NodeType type() const {
        return NodeType::INFIX_EXPRESSION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"INFIX_EXPRESSION\"";
        oss << ",\"op\":\"" << escapeJson(op) << "\"";
        oss << ",\"left\":";
        if (left) oss << left->toString();
        else oss << "null";
        oss << ",\"right\":";
        if (right) oss << right->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~InfixExpression() = default;
};

// 函数调用
class CallExpression : impl Expression {
public:
    Token token;    // 函数调用
    UPtr<Expression> func; // 函数名
    Array<UPtr<Expression>> arguments; // 参数
    void expressionNode() override {}
    NodeType type() const {
        return NodeType::CALL_EXPRESSION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"CALL_EXPRESSION\"";
        oss << ",\"func\":";
        if (func) oss << func->toString();
        else oss << "null";
        oss << ",\"arguments\":[";
        for (size_t i = 0; i < arguments.size(); i++) {
            if (i > 0) oss << ",";
            if (arguments[i]) oss << arguments[i]->toString();
            else oss << "null";
        }
        oss << "]}";
        return oss.str();
    }
    ~CallExpression() = default;
};

// 索引访问
class IndexExpression : impl Expression {
public:
    Token token;    // 索引
    UPtr<Expression> left;
    UPtr<Expression> index;
    void expressionNode() override {}
    NodeType type() const {
        return NodeType::INDEX_EXPRESSION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"INDEX_EXPRESSION\"";
        oss << ",\"left\":";
        if (left) oss << left->toString();
        else oss << "null";
        oss << ",\"index\":";
        if (index) oss << index->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~IndexExpression() = default;
};

// await
class AwaitExpression : impl Expression {
public:
    Token token;                // await
    UPtr<Expression> argument;  // 后面的表达式
    void expressionNode() override {}
    NodeType type() const {
          return NodeType::AWAIT_EXPRESSION;
      }
    String tokenLiteral() const override {
          return token.getLiteral();
      }
    String toString() const override {
          std::ostringstream oss;
          oss << "{\"type\":\"AWAIT_EXPRESSION\"";
          oss << ",\"argument\":";
          if (argument) oss << argument->toString();
          else oss << "null";
          oss << "}";
          return oss.str();
      }
};

// . 访问表达式
class MemberAccessExpression : impl Expression {
public:
    Token token;                // .
    UPtr<Expression> object;
    UPtr<Expression> called;
    void expressionNode() override {}
    NodeType type() const {
          return NodeType::MEMBER_ACCESS_EXPRESSION;
      }
    String tokenLiteral() const override {
          return token.getLiteral();
      }
    String toString() const override {
          std::ostringstream oss;
          oss << "{\"type\":\"MEMBER_ACCESS_EXPRESSION\"";
          oss << ",\"object\":";
          if (object) oss << object->toString();
          else oss << "null";
          oss<<",\"called\":";
          if (called) oss << called->toString();
          else oss << "null";
          oss << "}";
          return oss.str();
      }
};
/**
 * 注解
 */

class AnnotationStatement : impl Statement {
public:
    Token token;    // @
    Array<UPtr<Annotation>> annotations;
    UPtr<Statement> target;     // function declare 或者 struct declare

    NodeType type() const {
        return NodeType::ANNOTATION_STATEMENT; 
    }
    String tokenLiteral() const override{
        return token.getLiteral();
    }
    void statementNode() override{}
    String toString()const override {
        std::ostringstream oss;
        oss << "{\"type\":\"ANNOTATION_STATEMENT\"";
        oss<<",\"annotations\":[";
        for(size_t i = 0;i<annotations.size();i++) {
            oss<<annotations[i]->toString();
            if(i < annotations.size() - 1) {
                oss<<",";
            }
        }
        oss << "],\"target\":";
        if (target) oss << target->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }

};

 // method
class MethodAnnotation : impl Annotation {
public:
// @method(Person)
    Token token;    // @
    UPtr<Identifier> name;  // 注解名
    UPtr<Identifier> annoType;  // 注解的类型
    void annotationNode() override {}
    NodeType type() const {
        return NodeType::METHOD_ANNOTATION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"METHOD_ANNOTATION\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"annoType\":";
        if (annoType) oss << annoType->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~MethodAnnotation() = default;
};

// platform
class PlatformAnnotation : impl Annotation {
public:
// @platform(Windows)
    Token token;    // @
    UPtr<Identifier> name;  // 注解名
    Array<UPtr<Identifier>> platforms;    // 平台名
    void annotationNode() override {}
    NodeType type() const {
        return NodeType::PLATFORM_ANNOTATION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"PLATFORM_ANNOTATION\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"platforms\":[";
        for (size_t i = 0; i < platforms.size(); i++) {
            if (i > 0) oss << ",";
            if (platforms[i]) oss << platforms[i]->toString();
            else oss << "null";
        }
        oss << "]}";
        return oss.str();
    }
    ~PlatformAnnotation() = default;
};

// native
class NativeAnnotation : impl Annotation {
public:
// @native('vm.file.ino')
    Token token;    // @
    UPtr<Identifier> name;  // 注解名
    String path;  // 路径标识符
    void annotationNode() override {}
    NodeType type() const {
        return NodeType::NATIVE_ANNOTATION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"NATIVE_ANNOTATION\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << ",\"path\":\"" << escapeJson(path) << "\"";
        oss << "}";
        return oss.str();
    }
    ~NativeAnnotation() = default;
};

// async
class AsyncAnnotation : impl Annotation {
public:
// @async
    Token token;    // @
    UPtr<Identifier> name;  // 注解名
    void annotationNode() override {}
    NodeType type() const {
        return NodeType::ASYNC_ANNOTATION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"ASYNC_ANNOTATION\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~AsyncAnnotation() = default;
};

// object
class ObjectAnnotation : impl Annotation {
public:
// @object
    Token token;    // @
    UPtr<Identifier> name;  // 注解名
    void annotationNode() override {}
    NodeType type() const {
        return NodeType::OBJECT_ANNOTATION;
    }
    String tokenLiteral() const override {
        return token.getLiteral();
    }
    String toString() const override {
        std::ostringstream oss;
        oss << "{\"type\":\"OBJECT_ANNOTATION\"";
        oss << ",\"name\":";
        if (name) oss << name->toString();
        else oss << "null";
        oss << "}";
        return oss.str();
    }
    ~ObjectAnnotation() = default;
};


#endif
