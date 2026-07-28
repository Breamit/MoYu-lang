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
        case TokenType::ES_I32:
        case TokenType::ES_U32:
        case TokenType::ES_I64:
        case TokenType::ES_U64:
        case TokenType::ES_F32:
        case TokenType::ES_F64:
        case TokenType::ES_STRING:
        case TokenType::ES_MAP:
        case TokenType::ES_LIST:
        case TokenType::ES_BOOL:
        case TokenType::ES_ANY:
        case TokenType::ES_VOID:
        case TokenType::ES_IDENT:
            return true;
        default:
            return false;
    }
}

TokenType Token::lookUpIdent(const Token& tk) {

    // 定义全局唯一的关键字表
    static Map<String, TokenType> keywordsMap = {
            {"let", TokenType::ES_LET},
            {"const", TokenType::ES_CONST},
            {"i32", TokenType::ES_I32},
            {"u32", TokenType::ES_U32},
            {"i64", TokenType::ES_I64},
            {"u64", TokenType::ES_U64},
            {"f32", TokenType::ES_F32},
            {"f64", TokenType::ES_F64},
            {"string", TokenType::ES_STRING},
            {"map", TokenType::ES_MAP},
            {"list", TokenType::ES_LIST},
            {"bool", TokenType::ES_BOOL},
            {"any", TokenType::ES_ANY},
            {"for", TokenType::ES_FOR},
            {"in", TokenType::ES_IN},
            {"while", TokenType::ES_WHILE},
            {"continue", TokenType::ES_CONTINUE},
            {"break", TokenType::ES_BREAK},
            {"if", TokenType::ES_IF},
            {"else", TokenType::ES_ELSE},
            {"elif", TokenType::ES_ELIF},
            {"switch", TokenType::ES_SWITCH},
            {"case", TokenType::ES_CASE},
            {"default", TokenType::ES_DEFAULT},
            {"fn", TokenType::ES_FUNCTION},
            {"void", TokenType::ES_VOID},
            {"return", TokenType::ES_RETURN},
            {"struct", TokenType::ES_STRUCT},
            {"null", TokenType::ES_NULL},
            {"none",TokenType::ES_NONE},
            {"enum", TokenType::ES_ENUM},
            {"not", TokenType::ES_NOT},
            {"and", TokenType::ES_AND},
            {"or", TokenType::ES_OR},
            {"import", TokenType::ES_IMPORT},
            {"export", TokenType::ES_EXPORT},
            {"as", TokenType::ES_AS},
            {"await", TokenType::ES_AWAIT},
            {"async", TokenType::ES_ASYNC},
            {"true", TokenType::ES_TRUE},
            {"false", TokenType::ES_FALSE}
        };

    // 如果识别到指定的关键字，就返回对应的TokenType。
    // 未识别到视作标识符
    auto it = keywordsMap.find(tk.getLiteral());
    if(it != keywordsMap.end()) {
        return it->second;
    } else {
        return TokenType::ES_IDENT;
    }
}

