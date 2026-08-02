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
        case TokenType::ES_ILEGALL:        return "ILLEGAL";
        case TokenType::ES_EOF:            return "EOF";
        case TokenType::ES_LET:            return "LET";
        case TokenType::ES_CONST:          return "CONST";
        case TokenType::ES_I32:            return "I32";
        case TokenType::ES_U32:            return "U32";
        case TokenType::ES_I64:            return "I64";
        case TokenType::ES_U64:            return "U64";
        case TokenType::ES_F32:            return "F32";
        case TokenType::ES_F64:            return "F64";
        case TokenType::ES_STRING:         return "STRING_TYPE";
        case TokenType::ES_MAP:            return "MAP";
        case TokenType::ES_LIST:           return "LIST";
        case TokenType::ES_BOOL:           return "BOOL";
        case TokenType::ES_ANY:            return "ANY";
        case TokenType::ES_FOR:            return "FOR";
        case TokenType::ES_IN:             return "IN";
        case TokenType::ES_WHILE:          return "WHILE";
        case TokenType::ES_CONTINUE:       return "CONTINUE";
        case TokenType::ES_BREAK:          return "BREAK";
        case TokenType::ES_IF:             return "IF";
        case TokenType::ES_ELSE:           return "ELSE";
        case TokenType::ES_ELIF:           return "ELIF";
        case TokenType::ES_SWITCH:         return "SWITCH";
        case TokenType::ES_CASE:           return "CASE";
        case TokenType::ES_DEFAULT:        return "DEFAULT";
        case TokenType::ES_FUNCTION:       return "FN";
        case TokenType::ES_VOID:           return "VOID";
        case TokenType::ES_RETURN:         return "RETURN";
        case TokenType::ES_AT:             return "AT";
        case TokenType::ES_STRUCT:         return "STRUCT";
        case TokenType::ES_NULL:           return "NULL";
        case TokenType::ES_NONE:           return "NONE";
        case TokenType::ES_ENUM:           return "ENUM";
        case TokenType::ES_NOT:            return "NOT";
        case TokenType::ES_AND:            return "AND";
        case TokenType::ES_OR:             return "OR";
        case TokenType::ES_IMPORT:         return "IMPORT";
        case TokenType::ES_EXPORT:         return "EXPORT";
        case TokenType::ES_AS:             return "AS";
        case TokenType::ES_AWAIT:          return "AWAIT";
        case TokenType::ES_IDENT:          return "IDENTIFIER";
        case TokenType::ES_PLUS:           return "PLUS";
        case TokenType::ES_MINUS:          return "MINUS";
        case TokenType::ES_MULTI:          return "MULTIPLY";
        case TokenType::ES_DIV:            return "DIVIDE";
        case TokenType::ES_MOD:            return "MOD";
        case TokenType::ES_BIT_AND:        return "BIT_AND";
        case TokenType::ES_BIT_OR:         return "BIT_OR";
        case TokenType::ES_BIT_CARET:      return "BIT_CARET";
        case TokenType::ES_ASSIGN:         return "ASSIGN";
        case TokenType::ES_NOT_EQUAL:      return "NOT_EQUAL";
        case TokenType::ES_EQUAL:          return "EQUAL";
        case TokenType::ES_EQGT:           return "GE";
        case TokenType::ES_EQLT:           return "LE";
        case TokenType::ES_GT:             return "GT";
        case TokenType::ES_LT:             return "LT";
        case TokenType::ES_BIT_INVERSION:  return "BIT_INVERSION";
        case TokenType::ES_SEMICOLON:      return "SEMICOLON";
        case TokenType::ES_COMMA:          return "COMMA";
        case TokenType::ES_LBRACE:         return "LBRACE";
        case TokenType::ES_RBRACE:         return "RBRACE";
        case TokenType::ES_LBRACK:         return "LBRACKET";
        case TokenType::ES_RBRACK:         return "RBRACKET";
        case TokenType::ES_LPAREN:         return "LPAREN";
        case TokenType::ES_RPAREN:         return "RPAREN";
        case TokenType::ES_COLON:          return "COLON";
        case TokenType::ES_DQUOTE:         return "DQUOTE";
        case TokenType::ES_SQUOTE:         return "SQUOTE";
        case TokenType::ES_INTEGER:        return "INTEGER";
        case TokenType::ES_FLOATING:       return "FLOATING";
        case TokenType::ES_STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::ES_TRUE:           return "TRUE";
        case TokenType::ES_FALSE:          return "FALSE";
    }
    return "UNKNOWN";
}

int main() {
    std::string root    = getProjectRoot();
    std::string inPath  = root + "/es_test_code/tokenizer_test.es";
    std::string outPath = root + "/es_test_log/tokenizer_test.log";

    // 1. 读取 .es 源文件
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

        if (type == TokenType::ES_EOF) {
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
