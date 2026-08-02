#include "token.h"

// 构造函数
Token::Token(TokenType t , const String& l) {
    this->type_ = t;
    this->literal_ = l;
}

// set 和 get
void Token::setTokenType(TokenType t) {
    this->type_ = t;
}
void Token::setLiteral(const String& l) {
    this->literal_ = l;
}
TokenType Token::getTokenType() const  {
    return this->type_;
}
String Token::getLiteral() const {
    return this->literal_;
}
void Token::setPosition(u32 col , u32 row) {
    this->col_ = col;
    this->row_ = row;
}
Pair<u32,u32> Token::getPosition() {
    return std::make_pair(this->col_,this->row_); 
}
void Token::setTL(TokenType tt ,const String& l) {
    this->type_ = tt;
    this->literal_ = l;
}

bool Token::isTypeToken() const {
    switch(this->type_) {
        case TokenType::MY_I32:
        case TokenType::MY_U32:
        case TokenType::MY_I64:
        case TokenType::MY_U64:
        case TokenType::MY_F32:
        case TokenType::MY_F64:
        case TokenType::MY_STRING:
        case TokenType::MY_MAP:
        case TokenType::MY_LIST:
        case TokenType::MY_BOOL:
        case TokenType::MY_ANY:
        case TokenType::MY_VOID:
        case TokenType::MY_IDENT:
            return true;
        default:
            return false;
    }
}

TokenType Token::lookUpIdent(const Token& tk) {

    // 定义全局唯一的关键字表
    static Map<String, TokenType> keywordsMap = {
            {"let", TokenType::MY_LET},
            {"const", TokenType::MY_CONST},
            {"i32", TokenType::MY_I32},
            {"u32", TokenType::MY_U32},
            {"i64", TokenType::MY_I64},
            {"u64", TokenType::MY_U64},
            {"f32", TokenType::MY_F32},
            {"f64", TokenType::MY_F64},
            {"string", TokenType::MY_STRING},
            {"map", TokenType::MY_MAP},
            {"list", TokenType::MY_LIST},
            {"bool", TokenType::MY_BOOL},
            {"any", TokenType::MY_ANY},
            {"for", TokenType::MY_FOR},
            {"in", TokenType::MY_IN},
            {"while", TokenType::MY_WHILE},
            {"continue", TokenType::MY_CONTINUE},
            {"break", TokenType::MY_BREAK},
            {"if", TokenType::MY_IF},
            {"else", TokenType::MY_ELSE},
            {"elif", TokenType::MY_ELIF},
            {"switch", TokenType::MY_SWITCH},
            {"case", TokenType::MY_CASE},
            {"default", TokenType::MY_DEFAULT},
            {"fn", TokenType::MY_FUNCTION},
            {"void", TokenType::MY_VOID},
            {"return", TokenType::MY_RETURN},
            {"struct", TokenType::MY_STRUCT},
            {"null", TokenType::MY_NULL},
            {"none",TokenType::MY_NONE},
            {"enum", TokenType::MY_ENUM},
            {"not", TokenType::MY_NOT},
            {"and", TokenType::MY_AND},
            {"or", TokenType::MY_OR},
            {"import", TokenType::MY_IMPORT},
            {"export", TokenType::MY_EXPORT},
            {"as", TokenType::MY_AS},
            {"await", TokenType::MY_AWAIT},
            {"true", TokenType::MY_TRUE},
            {"false", TokenType::MY_FALSE}
        };

    // 如果识别到指定的关键字，就返回对应的TokenType。
    // 未识别到视作标识符
    auto it = keywordsMap.find(tk.getLiteral());
    if(it != keywordsMap.end()) {
        return it->second;
    } else {
        return TokenType::MY_IDENT;
    }
}

