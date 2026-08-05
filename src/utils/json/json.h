#ifndef __JSON_H__
#define __JSON_H__
#include <src/marco.h>
#include <src/type.h>
#include <src/error/errorinfo.hpp>
#include <src/object/object.hpp>
#include <fstream>
#include <cctype>
#include <sstream>
// TokenType
enum class JTokenType {
    JMY_ILEGALL = 0,     // 非法字符
    JMY_EOF,             // 文件结束
    JMY_IDENT,           // 标识符
    JMY_COMMA,           // , 
    JMY_LBRACE,          // {
    JMY_RBRACE,          // }
    JMY_LBRACK,          // [
    JMY_RBRACK,          // ]
    JMY_COLON,           // :
    JMY_MINUS,           // 负号
    JMY_PLUS,            // 加号
    JMY_NUMBER,          // 整数字面量
    JMY_FLOAT,           // 浮点数字面量
    JMY_STRING,          // 字符串字面量（""或''包裹的内容）
    JMY_TRUE,            // true , 布尔字面量
    JMY_FALSE,           // false , 布尔字面量
    JMY_NULL,            // null , 空字面量
};  

class JToken {
public:
    JTokenType type;
    u32 line;
    u32 col;
    String literal;
    JToken() = default;
    JToken(JTokenType t , const String& l);
    void setPos(u32 l, u32 c);
    ~JToken() = default;
};

class JLexer {
    String input_;              // 输入的字符串，这个需要依靠io流读取出来之后传入
    u32 position_;              // 当前的字符指针位置
    u32 readPosition_;          // 下一个要读取的指针的位置
    char ch_;                   // 当前指针指向的字符
    u32 line_;                  // 当前行号
    u32 col_;                   // 当前列号
    Array<ErrorInfo> errors_;   // 错误信息
public:
    JLexer(const String& input);
    JToken nextToken();
    const Array<ErrorInfo>& getErrors() const;

private:
    // 读取一个字符并移动字符指针
    void readChar();

    // 跳过空白段
    void skipWhitespace();

    // 读取数字字面量 ， 包含整数、浮点数
    String readNumber();
    
    // 读取标识符
    String readIdent();

    // 读取字符串字面量
    String readString(char quote);

    // 获取下一个字符，但是不移动指针
    char peekChar();

    // 跳过单行注释，不生成任何Token
    void skipComment();
};

// 数字是  i64 , 浮点是 f64 , 其余一致
class JSON {
    UPtr<JLexer> lexer_;
    JToken curToken_;
    JToken peekToken_;
    SPtr<Object> cache_;
public:
    JSON(const String& input);
    JSON(std::ifstream& is);
    ~JSON() = default;
    // 字符串
    SPtr<Object> parse();

    // 字符串化
    String stringify(SPtr<Object> obj);
private:
    // 获取下一个Token：推进 curToken_/peekToken_，返回值无意义
    void nextToken();

    // 解析json
    SPtr<Object> parseJson();

    // 解析json语句
    SPtr<Object> parseJsonStatement();

    // 解析json整数
    SPtr<Object> parseInt();

    // 解析json浮点数
    SPtr<Object> parseFloat();

    // 解析 null
    SPtr<Object> parseNull();

    // 解析true / false 字面量
    SPtr<Object> parseBoolean();

    // 解析字符串
    SPtr<Object> parseString();

    // 解析数组
    SPtr<Object> parseArray();

    // 解析对象
    SPtr<Object> parseObject();
};
 


#endif
