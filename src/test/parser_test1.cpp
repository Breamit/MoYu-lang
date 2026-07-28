#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

#include <src/lexer/lexer.h>
#include <src/ast/parser.h>

namespace fs = std::filesystem;

/**
 * @brief 获取项目根目录（src/test/../../）
 */
static std::string getProjectRoot() {
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

int main() {
    std::string root    = getProjectRoot();
    std::string inPath  = root + "/es_test_code/parser_test1.es";
    std::string outPath = root + "/es_test_log/parser_test1.log";

    // 1. 读取 .es 源文件
    std::cout << "[INFO] Reading: " << inPath << std::endl;
    String source = readFile(inPath);

    // 2. 创建词法分析器
    auto lexer = std::make_unique<Lexer>(source);

    auto lp = std::make_unique<Lexer>(source);
    

    // 3. 创建语法分析器
    Parser parser(std::move(lexer));

    // 4. 解析程序，得到 AST
    auto program = parser.parseProgram();

    // 5. 打开输出文件
    std::ofstream log(outPath);
    if (!log.is_open()) {
        std::cerr << "[ERROR] Cannot create output file: " << outPath << std::endl;
        return 1;
    }

    // while(true) {
    //     Token tk = lp->nextToken();
    //     log<<tk.getLiteral()<<std::endl;
    //     if(tk.getTokenType() == TokenType::ES_EOF) {
    //         break;
    //     }
    // }

    // 6. 输出 AST 树的 JSON 格式（递归调用每个节点的 toString()）
    log << program->toString() << std::endl;

    log.close();

    std::cout << "[OK] Parsing finished -> " << outPath << std::endl;

    return 0;
}
