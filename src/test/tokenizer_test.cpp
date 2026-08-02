#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

#include <src/lexer/lexer.h>

namespace fs = std::filesystem;

/**
 * @brief 获取项目根目录（src/test/../../）
 */
static std::string getProjectRoot() {
    // __FILE__ 是 src/test/tokenizer_test.cpp
    fs::path cur = fs::absolute(__FILE__);
    return cur.parent_path().parent_path().parent_path().string();
}

/**
 * @brief 读取文件内容为字符串
 */
static String readFile(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << path << std::endl;
        std::exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief 将 TokenType 枚举转换为可读的字符串名称
 */
static String tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::MY_ILEGALL:        return "ILLEGAL";
        case TokenType::MY_EOF:            return "EOF";
        case TokenType::MY_LET:            return "LET";
        case TokenType::MY_CONST:          return "CONST";
        case TokenType::MY_I32:            return "I32";
        case TokenType::MY_U32:            return "U32";
        case TokenType::MY_I64:            return "I64";
        case TokenType::MY_U64:            return "U64";
        case TokenType::MY_F32:            return "F32";
        case TokenType::MY_F64:            return "F64";
        case TokenType::MY_STRING:         return "STRING_TYPE";
        case TokenType::MY_MAP:            return "MAP";
        case TokenType::MY_LIST:           return "LIST";
        case TokenType::MY_BOOL:           return "BOOL";
        case TokenType::MY_ANY:            return "ANY";
        case TokenType::MY_FOR:            return "FOR";
        case TokenType::MY_IN:             return "IN";
        case TokenType::MY_WHILE:          return "WHILE";
        case TokenType::MY_CONTINUE:       return "CONTINUE";
        case TokenType::MY_BREAK:          return "BREAK";
        case TokenType::MY_IF:             return "IF";
        case TokenType::MY_ELSE:           return "ELSE";
        case TokenType::MY_ELIF:           return "ELIF";
        case TokenType::MY_SWITCH:         return "SWITCH";
        case TokenType::MY_CASE:           return "CASE";
        case TokenType::MY_DEFAULT:        return "DEFAULT";
        case TokenType::MY_FUNCTION:       return "FN";
        case TokenType::MY_VOID:           return "VOID";
        case TokenType::MY_RETURN:         return "RETURN";
        case TokenType::MY_AT:             return "AT";
        case TokenType::MY_STRUCT:         return "STRUCT";
        case TokenType::MY_NULL:           return "NULL";
        case TokenType::MY_NONE:           return "NONE";
        case TokenType::MY_ENUM:           return "ENUM";
        case TokenType::MY_NOT:            return "NOT";
        case TokenType::MY_AND:            return "AND";
        case TokenType::MY_OR:             return "OR";
        case TokenType::MY_IMPORT:         return "IMPORT";
        case TokenType::MY_EXPORT:         return "EXPORT";
        case TokenType::MY_AS:             return "AS";
        case TokenType::MY_AWAIT:          return "AWAIT";
        case TokenType::MY_IDENT:          return "IDENTIFIER";
        case TokenType::MY_PLUS:           return "PLUS";
        case TokenType::MY_MINUS:          return "MINUS";
        case TokenType::MY_MULTI:          return "MULTIPLY";
        case TokenType::MY_DIV:            return "DIVIDE";
        case TokenType::MY_MOD:            return "MOD";
        case TokenType::MY_BIT_AND:        return "BIT_AND";
        case TokenType::MY_BIT_OR:         return "BIT_OR";
        case TokenType::MY_BIT_CARET:      return "BIT_CARET";
        case TokenType::MY_ASSIGN:         return "ASSIGN";
        case TokenType::MY_NOT_EQUAL:      return "NOT_EQUAL";
        case TokenType::MY_EQUAL:          return "EQUAL";
        case TokenType::MY_EQGT:           return "GE";
        case TokenType::MY_EQLT:           return "LE";
        case TokenType::MY_GT:             return "GT";
        case TokenType::MY_LT:             return "LT";
        case TokenType::MY_BIT_INVERSION:  return "BIT_INVERSION";
        case TokenType::MY_SEMICOLON:      return "SEMICOLON";
        case TokenType::MY_COMMA:          return "COMMA";
        case TokenType::MY_LBRACE:         return "LBRACE";
        case TokenType::MY_RBRACE:         return "RBRACE";
        case TokenType::MY_LBRACK:         return "LBRACKET";
        case TokenType::MY_RBRACK:         return "RBRACKET";
        case TokenType::MY_LPAREN:         return "LPAREN";
        case TokenType::MY_RPAREN:         return "RPAREN";
        case TokenType::MY_COLON:          return "COLON";
        case TokenType::MY_DQUOTE:         return "DQUOTE";
        case TokenType::MY_SQUOTE:         return "SQUOTE";
        case TokenType::MY_INTEGER:        return "INTEGER";
        case TokenType::MY_FLOATING:       return "FLOATING";
        case TokenType::MY_STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::MY_TRUE:           return "TRUE";
        case TokenType::MY_FALSE:          return "FALSE";
    }
    return "UNKNOWN";
}

int main() {
    std::string root    = getProjectRoot();
    std::string inPath  = root + "/es_test_code/tokenizer_test.my";
    std::string outPath = root + "/es_test_log/tokenizer_test.log";

    // 1. 读取 .my 源文件
    std::cout << "[INFO] Reading: " << inPath << std::endl;
    String source = readFile(inPath);

    // 2. 创建词法分析器
    Lexer lexer(source);

    // 3. 打开输出文件
    std::ofstream log(outPath);
    if (!log.is_open()) {
        std::cerr << "[ERROR] Cannot create output file: " << outPath << std::endl;
        return 1;
    }

    // 4. 逐 token 分析，写入日志（序号\t类型\t字面量）
    int index = 0;
    while (true) {
        Token tk = lexer.nextToken();
        TokenType type = tk.getTokenType();
        String literal = tk.getLiteral();
        auto pos = tk.getPosition();
        u32 col = pos.first;
        u32 row = pos.second;

        log << index << "\t"
            << "[ " << col << " , " <<row <<" ]" << "\t"
            << tokenTypeName(type) << "\t"
            << literal << "\n";

        ++index;

        if (type == TokenType::MY_EOF) {
            break;
        }
    }
    const auto errs = lexer.getErrors();
    if(errs.size() != 0) {
        log<<"errors"<<"\n";
        for(auto& err : errs) {
            err.println(log);
        }
    }

    log.close();

    std::cout << "[OK] Tokenization finished: "
              << index << " tokens -> " << outPath << std::endl;

    return 0;
}
