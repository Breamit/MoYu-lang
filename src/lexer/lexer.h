#ifndef __LEXER_H__
#define __LEXER_H__

#include <cctype>
#include <src/type.h>
#include "token.h"
#include <src/error/errorinfo.hpp>



class Lexer {
    String input_;              // 输入的字符串，这个需要依靠io流读取出来之后传入
    u32 position_;              // 当前的字符指针位置
    u32 readPosition_;          // 下一个要读取的指针的位置
    char ch_;                   // 当前指针指向的字符
    u32 line_;                  // 当前行号
    u32 col_;                   // 当前列号
    Array<ErrorInfo> errors_;   // 错误信息
public:

    // 构造函数
    Lexer(const String& input);

    // 用来获取下一个token
    Token nextToken();

    // 获取错误信息
    const Array<ErrorInfo>& getErrors() const;
private:
    // 读取一个字符并移动字符指针
    void readChar();

    // 读取完整的标识符名字
    String readIdentifier();

    // 跳过空白段
    void skipWhitespace();

    // 读取数字字面量 ， 包含整数、浮点数
    String readNumber();

    // 读取字符串字面量
    String readString(char quote);

    // 获取下一个字符，但是不移动指针
    char peekChar();

    // 跳过单行注释，不生成任何Token
    void skipComment();

};


#endif