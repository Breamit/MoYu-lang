#ifndef __ERROR_INFO_HPP__
#define __ERROR_INFO_HPP__

#include <src/type.h>
#include <src/lexer/token.h>
#include <ostream>
#include <sstream>
#include <fstream>
class Token;
enum ErrorType {
    NONE = 0,
    TOKENIZER,
    LEXER,
    ASTBUILD
};
class ErrorInfo {
private:
    String msg;
    u32 line;
    u32 col;
    ErrorType et;
    // String fileName; 将来引入多文件的时候处理
    static String getErrorMsgHeader(ErrorType et) {
        switch(et) {
            case ErrorType::NONE:
                return "error: ";
            case ErrorType::TOKENIZER:
                return "tokenizer error: ";
            case ErrorType::LEXER:
                return "lexerial error: ";
            case ErrorType::ASTBUILD:
                return "ast build error: ";
            default:
                return "unknown error: ";
        }
    }
public:

    ErrorInfo() = delete;
    ErrorInfo(Token tk) {
        auto pos = tk.getPosition();
        this->col = pos.first;
        this->line = pos.second;
        msg = String();
        et = ErrorType::NONE;
    }

    void setMsg(const String& str, ErrorType et = ErrorType::NONE) {
        msg = str;
        if(et != this->et) {
            this->et = et;
        }
    }
    void println(std::ostream& os) const {
        std::stringstream ss;
        ss<<getErrorMsgHeader(this->et);
        ss<<" at line: "<<this->line<<" col : "<<this->col<<" , ";
        ss<<this->msg<<" "<<std::endl;
        os<<ss.str();
    }
    void print(std::ostream& os) const{
        std::stringstream ss;
        ss<<getErrorMsgHeader(this->et);
        ss<<" at line: "<<this->line<<" col : "<<this->col<<" , ";
        ss<<this->msg<<" ";
        os<<ss.str();
    }
    void println(std::ofstream& os) const{
        std::stringstream ss;
        ss<<getErrorMsgHeader(this->et);
        ss<<" at line: "<<this->line<<" col : "<<this->col<<" , ";
        ss<<this->msg<<" "<<std::endl;
        os<<ss.str();
    }
    void print(std::ofstream& os) const {
        std::stringstream ss;
        ss<<getErrorMsgHeader(this->et);
        ss<<" at line: "<<this->line<<" col : "<<this->col<<" , ";
        ss<<this->msg<<" ";
        os<<ss.str();
    }
};


#endif