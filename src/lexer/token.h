// Copyright 2026 Breamit. All rights reserved.
// 这个文件是声明解释器可以识别的
// token的类型TokenType以及token对象 Token

#ifndef __TOKEN_H__
#define __TOKEN_H__



#include <src/type.h>
#include <src/marco.h>
enum class TokenType {
    ES_ILEGALL = 0,     // 非法字符
    ES_EOF,             // 文件结束
    ES_LET,             // let
    ES_CONST,           // const
    ES_I32,             //i32
    ES_U32,             //u32
    ES_I64,             //i64
    ES_U64,             //u64
    ES_F32,             //f32
    ES_F64,             //f64
    ES_STRING,          //string
    ES_MAP,             //map
    ES_LIST,            //list
    ES_BOOL,            //bool
    ES_ANY,             //any
    ES_LAMBDA,          //lambda 闭包类型标注
    ES_FOR,             //for
    ES_IN,              //in
    ES_WHILE,           //while
    ES_CONTINUE,        //continue
    ES_BREAK,           //break
    ES_IF,              //if
    ES_ELSE,            //else
    ES_ELIF,            //elif
    ES_SWITCH,          //switch
    ES_CASE,            //case
    ES_DEFAULT,         //default
    ES_FUNCTION,        //fn
    ES_VOID,            //void
    ES_RETURN,          //return
    ES_AT,              //@
    ES_STRUCT,          //struct
    ES_NULL,            //null
    ES_NONE,            // none
    ES_ENUM,            //enum
    ES_NOT,             //not
    ES_AND,             //and
    ES_OR,              //or
    ES_IMPORT,          //import
    ES_EXPORT,          //export
    ES_AS,              //as
    ES_AWAIT,           //await
    ES_IDENT,           //标识符
    ES_PLUS,            // +
    ES_MINUS,           // -
    ES_MULTI,           // *
    ES_DIV,             // /
    ES_MOD,             // %
    ES_BIT_AND,         // &
    ES_BIT_OR,          // |
    ES_BIT_CARET,       // ^
    ES_ASSIGN,          // =
    ES_NOT_EQUAL,       // !=
    ES_EQUAL,           // ==
    ES_EQGT,            // >=
    ES_EQLT,            // <=
    ES_GT,              // >
    ES_LT,              // <
    ES_BIT_LEFT,        // <<
    ES_BIT_RIGHT,       // >>
    ES_BIT_INVERSION,   // ~
    ES_SEMICOLON,       // ;
    ES_COMMA,           // , 
    ES_LBRACE,          // {
    ES_RBRACE,          // }
    ES_LBRACK,          // [
    ES_RBRACK,          // ]
    ES_LPAREN,          // (
    ES_RPAREN,          // )
    ES_COLON,           // :
    ES_DQUOTE,          // "
    ES_SQUOTE,          // '
    ES_DOT,             // .
    ES_INTEGER,         // 整数字面量
    ES_FLOATING,        // 浮点数字面量
    ES_STRING_LITERAL,  // 字符串字面量（""或''包裹的内容）
    ES_TRUE,            // true , 布尔字面量
    ES_FALSE,           // false , 布尔字面量
};  

class Token {
private:
    TokenType type_;
    String literal_;
    u32 col_;
    u32 row_;
public:

    // 构造函数
    Token() = default;
    Token(TokenType t , const String& l);

    // 默认析构函数
    ~Token() = default;

    // get 和 set 函数
    void setTokenType(TokenType t);
    void setLiteral(const String& l);
    TokenType getTokenType() const ;
    String getLiteral() const ;
    void setPosition(u32 col , u32 row);
    Pair<u32,u32> getPosition();
    void setTL(TokenType tt , const String& l);

    /**
     * @brief 判断当前token是否为合法的类型名（内置类型或自定义标识符）
     */
    bool isTypeToken() const;

    /**
     * @brief 用于区分字面量是不是关键字
     * @param 需要进行判断的token
     * @return 如果是关键字，返回关键字的token，否则返回ES_IDENT
     */
    static TokenType lookUpIdent(const Token& tk) ;
};

#endif