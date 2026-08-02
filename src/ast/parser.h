#ifndef __PARSER_H__
#define __PARSER_H__

#include "ast.hpp"
#include <src/marco.h>
#include <src/type.h>
#include <src/lexer/lexer.h>
#include <functional>
#include <src/error/errorinfo.hpp>



enum class Precedence {
    LOWEST = 0,
    ASSIGN,      // =
    OR,          // |
    XOR,         // ^
    AND,         // &
    EQUALS,      // == !=
    LESSGREATER, // > < <= >= >= <=
    SUM,         // + -
    PRODUCT,     // * / %
    PREFIX,      // - !
    CALL         // 函数调用 () [] .
};


// 消费规则：消费token的函数，必须把所有的token都消耗完，
// 并且使得curToken指向下一个的起始的token

class Parser {
private:
    UPtr<Lexer> lexer_;               // 词法分析器
    Token curToken_;            // 当前token
    Token peekToken_;           // 下一个token
    Array<ErrorInfo> errors_;   // 错误信息

    // 前缀解析函数接口
    using PrefixFn = std::function<UPtr<Expression>()>;
    // 中缀解析函数接口
    using InfixFn = std::function<UPtr<Expression>(UPtr<Expression>)>;

    // 工具方法
    // 移动到下一个token
    void nextToken();

    // 检查当前token是否是指定的token类型
    bool curTokenIs(TokenType type) const;

    // 检查下一个token是否是指定的token类型
    bool peekTokenIs(TokenType type) const;

    // 检查下一个token是否是预期类型，如果是就消费
    bool expectPeek(TokenType type);

    // 获取下一个token的优先级
    Precedence peekPrecedence() const;

    // 获取当前token的优先级
    Precedence curPrecedence() const;

    // 获取token的优先级的函数
    static Precedence tokenPrecedence(TokenType type);

public:
    // 构造函数
    Parser(UPtr<Lexer> lexer);

    // 获取错误信息
    Array<ErrorInfo> getErrors() const;

    // 解析程序
    UPtr<Program> parseProgram();

    // 不允许顶层存在可执行语句的解析程序
    UPtr<Program> parseProgramWithoutTopLevelStatements();

    // 解析语句
    UPtr<Statement> parseStatement();

    // 解析表达式
    UPtr<Expression> parseExpression(Precedence precedence);

    // 解析表达式语句
    UPtr<ExpressionStatement> parseExpressionStatement();
    
    // 解析标识符
    UPtr<Expression> parseIdentifier();

    // 解析整数字面量
    UPtr<Expression> parseIntegerLiteral();

    // 解析浮点数字面量
    UPtr<Expression> parseFloatingLiteral();

    // 解析字符串字面量
    UPtr<Expression> parseStringLiteral();

    // 解析布尔字面量
    UPtr<Expression> parseBooleanLiteral();

    // 解析null字面量
    UPtr<Expression> parseNullLiteral();

    // 解析类型标注字面量
    UPtr<TypeLiteral> parseTypeLiteral();

    // 解析表达式列表
    UPtr<Array<UPtr<Expression>>> parseListExpression(TokenType endToken);

    // 解析数组字面量
    UPtr<ArrayLiteral> parseArrayLiteral();

    // 解析哈希表字面量
    UPtr<MapLiteral> parseMapLiteral();

    // 解析前缀表达式
    UPtr<Expression> parsePrefixExpression();
    
    // 解析中缀表达式
    UPtr<Expression> parseInfixExpression(UPtr<Expression> left);

    // 解析括号内容
    UPtr<Expression> parseGroupedExpression();

    // 解析块内内容
    UPtr<BlockStatement> parseBlockStatement();

    // 函数-解析参数列表
    UPtr<Array<Pair<UPtr<Expression>, UPtr<TypeLiteral>>>> parseParamsList();




    // 解析let语句
    UPtr<LetStatement> parseLetStatement();

    // 解析 const 语句
    UPtr<ConstStatement> parseConstStatement();

    // 解析 return 语句
    UPtr<ReturnStatement> parseReturnStatement() ;

    // 解析 break 语句
    UPtr<BreakStatement> parseBreakStatement();

    // 解析 continue 语句
    UPtr<ContinueStatement> parseContinueStatement();

    // 解析 if-elif-else 语句
    UPtr<IfStatement> parseIfStatement();

    // 解析 switch - case 语句
    UPtr<SwitchStatement> parseSwitchStatement();

    // 解析 for 语句
    UPtr<ForStatement> parseForStatement();

    // 解析 while 语句
    UPtr<WhileStatement> parseWhileStatement();

    // 解析函数声明
    UPtr<FunctionDeclareStatement> parseFunctionDeclareStatement();

    // 解析结构体声明
    UPtr<StructDeclareStatement> parseStructDeclareStatement();

    // 解析枚举声明
    UPtr<EnumDeclareStatement> parseEnumDeclareStatement();

    // 解析 import 语句
    UPtr<ImportStatement> parseImportStatement();

    // 解析 export 语句
    UPtr<ExportStatement> parseExportStatement();

    // 解析注解语句
    UPtr<AnnotationStatement> parseAnnotationStatement();

    // 解析 await 语句
    UPtr<AwaitExpression> parseAwaitExpression();

    // 解析函数调用参数
    UPtr<Array<UPtr<Expression>>> parseCallArguments();

    // 解析函数调用
    UPtr<CallExpression> parseCallExpression(UPtr<Expression> func);

    // 解析索引访问
    UPtr<IndexExpression> parseIndexExpression(UPtr<Expression> left);

    // 解析成员访问表达式
    UPtr<MemberAccessExpression> parseMemberAccessExpression(UPtr<Expression> left);

    // 前缀处理函数
    PrefixFn prefixParseFn(TokenType type);
    // 中缀处理函数
    InfixFn infixParseFn(TokenType type);

};


#endif
