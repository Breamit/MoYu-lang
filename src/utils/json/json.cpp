#include "json.h"
#include <algorithm>


JToken::JToken(JTokenType t, const String &l) {
    this->type = t;
    this->literal = l;
    this->line = 0;
    this->col = 0;
}

void JToken::setPos(u32 l, u32 c) {
    this->line = l;
    this->col = c;
}

JLexer::JLexer(const String &input) {
    this->input_ = input;
    this->position_ = 0;
    this->readPosition_ = 0;
    this->ch_ = 0;
    this->line_ = 1;
    this->col_ = 0;
    this->errors_ = {};
    readChar();
}

JToken JLexer::nextToken() {
    JToken jtk;
    skipWhitespace();
    jtk.setPos(this->line_, this->col_);
    
    // 数字，点，字符串，字母，大中括号，斜杠，冒号，负号，正号
    switch(this->ch_) {
        case '-':
            jtk.type = JTokenType::JMY_MINUS;
            jtk.literal = "-";
            break;
        case '+':
            jtk.type = JTokenType::JMY_PLUS;
            jtk.literal = "+";
            break;
        case '[':
            jtk.type = JTokenType::JMY_LBRACK;
            jtk.literal = "[";
            break;
        case ']':
            jtk.type = JTokenType::JMY_RBRACK;
            jtk.literal = "]";
            break;
        case '{':
            jtk.type = JTokenType::JMY_LBRACE;
            jtk.literal = "{";
            break;
        case '}':
            jtk.type = JTokenType::JMY_RBRACE;
            jtk.literal = "}";
            break;
        case ':':
            jtk.type = JTokenType::JMY_COLON;
            jtk.literal = ":";
            break;
        case ',':   
            jtk.type = JTokenType::JMY_COMMA;
            jtk.literal = ",";
            break;
        case '\"':
        case '\'':
            jtk.type = JTokenType::JMY_STRING;
            jtk.literal = readString(this->ch_);
            return jtk;
        case '\0':
            jtk.type = JTokenType::JMY_EOF;
            jtk.literal = "";
            break;
        case '/':
            // 处理注释
            if(peekChar() == '/') {
                // 跳过两个 /
                readChar();
                readChar();
                skipComment();
                return nextToken();
            } else {
                jtk.type = JTokenType::JMY_ILEGALL;
                jtk.literal = String(1, this->ch_);
            }
            break;
        default:
            //处理数字，字面量等
            if(std::isdigit(this->ch_)) {
                // 数字
                String num = readNumber();
                if(num.find('.') != String::npos) {
                    if(std::count(num.begin(),num.end(),'.') != 1) {
                        jtk.type = JTokenType::JMY_ILEGALL;
                    }
                    if(num.back() == '.') {
                        num+= '0';
                    }
                    jtk.type = JTokenType::JMY_FLOAT;
                } else {
                    jtk.type = JTokenType::JMY_NUMBER;
                }
                jtk.literal = num;
                return jtk;
            } else if(std::isalpha(this->ch_)) {
                // 字母
                jtk.literal = readIdent();
                if(jtk.literal == "true") {
                    jtk.type = JTokenType::JMY_TRUE;
                } else if(jtk.literal == "false") {
                    jtk.type = JTokenType::JMY_FALSE;
                } else if(jtk.literal == "null") {
                    jtk.type = JTokenType::JMY_NULL;
                } else {
                    jtk.type = JTokenType::JMY_ILEGALL;
                }
                return jtk;
            } else {
                // 其他字符
                jtk.type = JTokenType::JMY_ILEGALL;
                jtk.literal = String(1, this->ch_);
                break;
            }

    }
    readChar();
    return jtk;
}

const Array<ErrorInfo> &JLexer::getErrors() const {
    return this->errors_;
}

void JLexer::readChar() {
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

void JLexer::skipWhitespace() {
    while(std::isspace(this->ch_)) {
        readChar();
    }
}

String JLexer::readNumber() {
    // 支持整数和浮点字面量
    u32 pos = position_;
    while(std::isdigit(ch_) || ch_ == '.') {
        readChar();
    }
    return input_.substr(pos , position_ - pos);
}

String JLexer::readIdent() {
    u32 pos = position_;
    // 标识符支持数字、字母、下划线，但是不能以数字开头
    while(std::isalpha(ch_)) {
        readChar();
    }
    return input_.substr(pos,position_ - pos);
}

String JLexer::readString(char quote) {
    // 需要结局字符串的转义问题
    // 跳过开头的引号
    readChar();
    std::stringstream ss;
    // 没有遇到结束符号或者相同的引号
    while(ch_ != quote && ch_ != '\0') {
        if(ch_ == '\\') {
            readChar(); // 消耗 '\'
            switch(ch_) {
                case 'a':
                    ss<<'\a';
                    readChar(); // 消耗掉 a
                    continue;
                case 'b':
                    ss<<'\b';
                    readChar(); // 消耗掉 b
                    continue;
                case 'f':
                    ss<<'\f';
                    readChar(); // 消耗掉 f
                    continue;
                case 'n':
                    ss<<'\n';
                    readChar(); // 消耗掉 n
                    continue;
                case 'r':
                    ss<<'\r';
                    readChar(); // 消耗掉 r
                    continue;
                case 't':
                    ss<<'\t';
                    readChar(); // 消耗掉 t
                    continue;
                case 'v':
                    ss<<'\v';
                    readChar(); // 消耗掉 v
                    continue;
                case '\\':
                    ss<<'\\';
                    readChar(); // 消耗掉 '\'
                    continue;
                case '\'':
                    ss<<'\'';
                    readChar(); // 消耗掉 \'
                    continue;
                case '\"':
                    ss<<'\"';
                    readChar(); // 消耗掉 \"
                    continue;
                case '\0':
                    ss<<'\0';
                    readChar(); // 消耗掉 \0
                    continue;  
            }
        }
        ss<<ch_;
        readChar();
    }
    
    if(ch_ == quote) {
        readChar(); // 消耗掉引号
    }
    return ss.str();
}

char JLexer::peekChar() {
    if(readPosition_ >= input_.size()) {
        return 0;
    } else {
        return input_[readPosition_];
    }
}

void JLexer::skipComment() {
    // 只要没有遇到 \n 和 \0 就说明还是注释
    while(ch_ != '\n' && ch_ != '\0') {
        readChar();
    }
}

JSON::JSON(const String &input) {
    lexer_ = std::make_unique<JLexer>(input);
    cache_ = nullptr;
}

JSON::JSON(std::ifstream &is) {
    std::stringstream ss;
    ss << is.rdbuf();
    lexer_ = std::make_unique<JLexer>(ss.str());
    cache_ = nullptr;
}

SPtr<Object> JSON::parse() {
    // 保存解析结果，避免重复解析
    if(cache_ == nullptr) {
        cache_ = parseJson();
    }
    return cache_;
}

String JSON::stringify(SPtr<Object> obj) {
    // 按 Object 类型递归序列化
    switch(obj->type()) {
        case ObjectType::MAP: {
            // 对象：{ "key": value, ... }
            const auto& mp = static_cast<const MapObject*>(obj.get())->value;
            String out = "{";
            bool first = true;
            for(const auto& [k, v] : mp) {
                if(!first) out += ",";
                first = false;
                // 键必须是字符串 → 带引号
                out += "\"" + k->inspect() + "\":";
                out += stringify(v);
            }
            return out + "}";
        }
        case ObjectType::LIST: {
            // 数组：[ value, ... ]
            const auto& arr = static_cast<const ListObject*>(obj.get())->value;
            String out = "[";
            bool first = true;
            for(const auto& v : arr) {
                if(!first) out += ",";
                first = false;
                out += stringify(v);
            }
            return out + "]";
        }
        case ObjectType::STRING:
            // 字符串带引号
            return "\"" + static_cast<const StringObject*>(obj.get())->value + "\"";
        case ObjectType::BOOL:
            return static_cast<const BoolObject*>(obj.get())->value ? "true" : "false";
        case ObjectType::NIL:
            return "null";
        case ObjectType::I32:
            return std::to_string(static_cast<const I32Object*>(obj.get())->value);
        case ObjectType::I64:
            return std::to_string(static_cast<const I64Object*>(obj.get())->value);
        case ObjectType::U32:
            return std::to_string(static_cast<const U32Object*>(obj.get())->value);
        case ObjectType::U64:
            return std::to_string(static_cast<const U64Object*>(obj.get())->value);
        case ObjectType::F64:
            return std::to_string(static_cast<const F64Object*>(obj.get())->value);
        default:
            return "null";
    }
}

void JSON::nextToken() {
    curToken_ = peekToken_;
    peekToken_ = lexer_->nextToken();
}

SPtr<Object> JSON::parseJson() {
    nextToken();
    nextToken();
    // 只解析一个 JSON 值（标准 JSON：顶层是一个值，非语句列表）
    auto val = parseJsonStatement();
    return val;
}

SPtr<Object> JSON::parseJsonStatement() {
    // 按当前 token 类型分发到具体解析函数（递归下降）
    switch(curToken_.type) {
        case JTokenType::JMY_LBRACE:     return parseObject();
        case JTokenType::JMY_LBRACK:     return parseArray();
        case JTokenType::JMY_STRING:     return parseString();
        case JTokenType::JMY_MINUS:      return parseInt();      // 负数：交给 parseInt 统一处理
        case JTokenType::JMY_NUMBER:     return parseInt();
        case JTokenType::JMY_FLOAT:      return parseFloat();
        case JTokenType::JMY_TRUE:
        case JTokenType::JMY_FALSE:      return parseBoolean();
        case JTokenType::JMY_NULL:       return parseNull();
        default:
            return std::make_shared<ErrorObject>("json: unexpected token");
    }
}

SPtr<Object> JSON::parseInt() {
    // 整数：可带负号，构造 I64Object（long long）
    i64 val = std::stoll(curToken_.literal);
    nextToken();
    return std::make_shared<I64Object>(val);
}

SPtr<Object> JSON::parseFloat() {
    // 浮点数：构造 F64Object
    f64 val = std::stod(curToken_.literal);
    nextToken();
    return std::make_shared<F64Object>(val);
}

SPtr<Object> JSON::parseNull() {
    nextToken();
    return Null;
}

SPtr<Object> JSON::parseBoolean() {
    // true / false
    bool b = (curToken_.literal == "true");
    nextToken();
    return b ? True : False;
}

SPtr<Object> JSON::parseString() {
    // 字符串字面量：构造 StringObject
    auto str = std::make_shared<StringObject>(curToken_.literal);
    nextToken();
    return str;
}

SPtr<Object> JSON::parseArray() {
    // 数组：[ value, value, ... ]
    auto arr = std::make_shared<ListObject>();
    nextToken();   // 吃掉 '['
    // 空数组 []
    if(curToken_.type == JTokenType::JMY_RBRACK) {
        nextToken();
        return arr;
    }
    while(curToken_.type != JTokenType::JMY_EOF) {
        auto val = parseJsonStatement();   // ← 递归：元素可以是任何值
        arr->value.push_back(val);
        if(curToken_.type == JTokenType::JMY_COMMA) {
            nextToken();   // 吃掉 ','
            continue;
        }
        if(curToken_.type == JTokenType::JMY_RBRACK) {
            nextToken();   // 吃掉 ']'
            break;
        }
        return std::make_shared<ErrorObject>("json: array expect , or ]");
    }
    return arr;
}

SPtr<Object> JSON::parseObject() {
    // 对象：{ "key": value, ... }
    auto mp = std::make_shared<MapObject>();
    nextToken();   // 吃掉 '{'
    // 空对象 {}
    if(curToken_.type == JTokenType::JMY_RBRACE) {
        nextToken();
        return mp;
    }
    while(curToken_.type != JTokenType::JMY_EOF) {
        // 键必须是字符串
        if(curToken_.type != JTokenType::JMY_STRING) {
            return std::make_shared<ErrorObject>("json: object key must be string");
        }
        String key = curToken_.literal;
        nextToken();   // 吃掉 key
        if(curToken_.type != JTokenType::JMY_COLON) {
            return std::make_shared<ErrorObject>("json: object expect :");
        }
        nextToken();   // 吃掉 ':'
        auto val = parseJsonStatement();   // ← 递归：值可以是对象/数组/任何值
        mp->value[std::make_shared<StringObject>(key)] = val;
        if(curToken_.type == JTokenType::JMY_COMMA) {
            nextToken();   // 吃掉 ','
            continue;
        }
        if(curToken_.type == JTokenType::JMY_RBRACE) {
            nextToken();   // 吃掉 '}'
            break;
        }
        return std::make_shared<ErrorObject>("json: object expect , or }");
    }
    return mp;
}