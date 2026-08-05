// Copyright 2026 Breamit. All rights reserved.
// 这个文件是声明解释器可以识别的
// token的类型TokenType以及token对象 Token

#ifndef __TOKEN_H__
#define __TOKEN_H__



#include <src/type.h>
#include <src/marco.h>
enum class TokenType {
    MY_ILEGALL = 0,     // 非法字符
    MY_EOF,             // 文件结束
    MY_LET,             // let
    MY_CONST,           // const
    MY_I32,             //i32
    MY_U32,             //u32
    MY_I64,             //i64
    MY_U64,             //u64
    MY_F32,             //f32
    MY_F64,             //f64
    MY_STRING,          //string
    MY_MAP,             //map
    MY_LIST,            //list
    MY_BOOL,            //bool
    MY_ANY,             //any
    MY_LAMBDA,          //lambda 闭包类型标注
    MY_BYTES,           //bytes
    MY_U8,              //u8 bytes = u8[]
    MY_FOR,             //for
    MY_IN,              //in
    MY_WHILE,           //while
    MY_CONTINUE,        //continue
    MY_BREAK,           //break
    MY_IF,              //if
    MY_ELSE,            //else
    MY_ELIF,            //elif
    MY_SWITCH,          //switch
    MY_CASE,            //case
    MY_DEFAULT,         //default
    MY_FUNCTION,        //fn
    MY_VOID,            //void
    MY_RETURN,          //return
    MY_AT,              //@
    MY_STRUCT,          //struct
    MY_NULL,            //null
    MY_NONE,            // none
    MY_ENUM,            //enum
    MY_NOT,             //not
    MY_AND,             //and
    MY_OR,              //or
    MY_IMPORT,          //import
    MY_EXPORT,          //export
    MY_AS,              //as
    MY_AWAIT,           //await
    MY_IDENT,           //标识符
    MY_PLUS,            // +
    MY_MINUS,           // -
    MY_MULTI,           // *
    MY_DIV,             // /
    MY_MOD,             // %
    MY_BIT_AND,         // &
    MY_BIT_OR,          // |
    MY_BIT_CARET,       // ^
    MY_ASSIGN,          // =
    MY_NOT_EQUAL,       // !=
    MY_EQUAL,           // ==
    MY_EQGT,            // >=
    MY_EQLT,            // <=
    MY_GT,              // >
    MY_LT,              // <
    MY_BIT_INVERSION,   // ~
    MY_SEMICOLON,       // ;
    MY_COMMA,           // , 
    MY_LBRACE,          // {
    MY_RBRACE,          // }
    MY_LBRACK,          // [
    MY_RBRACK,          // ]
    MY_LPAREN,          // (
    MY_RPAREN,          // )
    MY_COLON,           // :
    MY_DQUOTE,          // "
    MY_SQUOTE,          // '
    MY_DOT,             // .
    MY_INTEGER,         // 整数字面量
    MY_FLOATING,        // 浮点数字面量
    MY_STRING_LITERAL,  // 字符串字面量（""或''包裹的内容）
    MY_TRUE,            // true , 布尔字面量
    MY_FALSE,           // false , 布尔字面量
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
     * @return 如果是关键字，返回关键字的token，否则返回MY_IDENT
     */
    static TokenType lookUpIdent(const Token& tk) ;
};

#endif