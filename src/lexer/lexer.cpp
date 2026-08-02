#include "lexer.h"
#include <algorithm>
#include <sstream>

Lexer::Lexer(const String& input) {
    this->input_ = input;
    this->position_ = 0;
    this->readPosition_ = 0;
    this->ch_ = 0;
    this->line_ = 1;
    this->col_ = 0;
    this->errors_ = {};
    // 移动指针，此时readPosition指向第2个字符，不移动position指针
    readChar();
}


Token Lexer::nextToken() {
    Token tk;
    skipWhitespace();
    tk.setPosition(this->col_,this->line_);

    switch(this->ch_) {
        case '@' :
            // 注解标识符
            tk.setTL(TokenType::ES_AT,"@");
            break;
        case '+' :
            // plus 
            tk.setTL(TokenType::ES_PLUS,"+");
            break;
        case '-' :
            // minus 
            tk.setTL(TokenType::ES_MINUS,"-");
            break;
        case '*' :
            // multi
            tk.setTL(TokenType::ES_MULTI,"*");
            break;
        case '/':
            // div
            tk.setTL(TokenType::ES_DIV,"/");
            break;
        case '%':
            // mod
            tk.setTL(TokenType::ES_MOD,"%");
            break;
        case '!' :
            // 需要分析是非还是不等于
            if(peekChar() == '=') {
                readChar();
                tk.setTL(TokenType::ES_NOT_EQUAL,"!=");
            } else {
                tk.setTL(TokenType::ES_NOT,"!");
            }
            break;
        case '&':
            // 按位与
            tk.setTL(TokenType::ES_BIT_AND,"&");
            break;
        case '|':
            // 按位或
            tk.setTL(TokenType::ES_BIT_OR,"|");
            break;
        case '^':
            // caret
            tk.setTL(TokenType::ES_BIT_CARET,"^");
            break;
        case '=':
            // assign
            if(peekChar() == '=' ) {
                tk.setTL(TokenType::ES_EQUAL,"==");
                readChar();
            } else {
                tk.setTL(TokenType::ES_ASSIGN,"=");
            }
            break;
        case '>':
            // greater
            if(peekChar() == '=') {
                // >=
                tk.setTL(TokenType::ES_EQGT,">=");
                readChar();
            } else {
                tk.setTL(TokenType::ES_GT,">");
            }
            break;
        case '<':
            // lower
            if(peekChar() == '=') {
                // <=
                tk.setTL(TokenType::ES_EQLT,"<=");
                readChar();
            } else {
                tk.setTL(TokenType::ES_LT,"<");
            }
            break;
        case '~':
            // inversion
            tk.setTL(TokenType::ES_BIT_INVERSION,"~");
            break;
        case ';':
            // semicolon
            tk.setTL(TokenType::ES_SEMICOLON,";");
            break;
        case ',':
            // comma
            tk.setTL(TokenType::ES_COMMA,",");
            break;
        case '.':
            // dot
            tk.setTL(TokenType::ES_DOT,".");
            break;
        case '{':
            // left brace
            tk.setTL(TokenType::ES_LBRACE,"{");
            break;
        case '}':
            // right brace
            tk.setTL(TokenType::ES_RBRACE,"}");
            break;
        case '[':
            // left brack
            tk.setTL(TokenType::ES_LBRACK,"[");
            break;
        case ']':
            // right brack
            tk.setTL(TokenType::ES_RBRACK,"]");
            break;
        case '(':
            // left paren
            tk.setTL(TokenType::ES_LPAREN,"(");
            break;
        case ')':
            // right paren
            tk.setTL(TokenType::ES_RPAREN,")");
            break;
        case ':':
            // colon
            tk.setTL(TokenType::ES_COLON,":");
            break;
        case '\"':
        case '\'':
            // 字符串字面量，统一处理，结果不含外层引号
            tk.setTL(TokenType::ES_STRING_LITERAL, readString(this->ch_));
            return tk;
        case '\0':
            // EOF
            tk.setTL(TokenType::ES_EOF, "");
            return tk;
        case '#':
            // comment
            readChar(); // 跳过这个comment的起始符号
            skipComment();
            return nextToken();     // 返回下一个token
            break;
        default:
            // identify , keyword
            if(std::isalpha(this->ch_)) {
                tk.setLiteral(readIdentifier());
                tk.setTokenType(Token::lookUpIdent(tk));
                return tk;
            } else if(std::isdigit(this->ch_)) {
                String num = readNumber();
                if(num.find('.') != String::npos) {
                    //   只能有一个小数点
                    if(std::count(num.begin(),num.end(),'.') != 1) {
                        tk.setTokenType(TokenType::ES_ILEGALL);
                    }
                    if(num.back() == '.') {
                        // 小数点结尾就加一个0，格式化处理
                        num += '.';
                    }
                    tk.setTokenType(TokenType::ES_FLOATING);
                } else {
                    tk.setTokenType(TokenType::ES_INTEGER);
                }
                tk.setLiteral(num); 
                return tk;
            } else {
                tk.setTL(TokenType::ES_ILEGALL,String(1,this->ch_));
                ErrorInfo ei(tk);
                ei.setMsg(" unknow token ",ErrorType::LEXER);
                readChar();     // 跳过非法字符，否则会死循环
                return tk;
            }
            break;
    }

    readChar();     //   这里读完了，下一个
    return tk;

}

const Array<ErrorInfo>& Lexer::getErrors() const {
    return this->errors_;
}


void Lexer::readChar() {
    if(readPosition_ >= input_.size()) {
        // 访问越界
        ch_ = 0;
    } else {
        ch_ = input_[readPosition_];
    }

    // 移动指针
    position_ = readPosition_;
    readPosition_++;
    if(ch_ == '\n') {
        line_ ++;
        col_ = 0;
    } else {
        col_ ++ ;
    }
}
String Lexer::readIdentifier() {
    u32 pos = position_;
    // 标识符支持数字、字母、下划线，但是不能以数字开头
    while(std::isalpha(ch_) || std::isdigit(ch_) || ch_ =='_') {
        readChar();
    }
    return input_.substr(pos,position_ - pos);
}
void Lexer::skipWhitespace() {
    // 遇到空白就移动一下指针
    while(std::isspace(ch_)) {
        readChar();
    }
}
String Lexer::readNumber(){
    // 支持整数和浮点字面量
    u32 pos = position_;
    while(std::isdigit(ch_) || ch_ == '.') {
        readChar();
    }
    return input_.substr(pos , position_ - pos);
}
String Lexer::readString(char quote) {
    // 跳过开头的引号
    readChar();
    u32 pos = position_;
    // 没有遇到结束符号或者相同的引号
    while(ch_ != quote && ch_ != '\0') {
        readChar();
    }
    String res = input_.substr(pos , position_ - pos);
    if(ch_ == quote) {
        // 正常遇到引号结束
        readChar(); // 消耗掉
    }
    return res;
}
char Lexer::peekChar() {
    if(readPosition_ >= input_.size()) {
        return 0;
    } else {
        return input_[readPosition_];
    }
}
void Lexer::skipComment() {
    // 只要没有遇到 \n 和 \0 就说明还是注释
    while(ch_ != '\n' && ch_ != '\0') {
        readChar();
    }
}
