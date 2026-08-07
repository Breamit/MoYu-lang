#include "parser.h"
void Parser::nextToken() {
    curToken_ = peekToken_;
    peekToken_ = lexer_->nextToken();
}

bool Parser::curTokenIs(TokenType type) const {
    if(curToken_.getTokenType() == type) {
        return true;
    }
    return false;
}

bool Parser::peekTokenIs(TokenType type) const {
    if(peekToken_.getTokenType() == type) {
        return true;
    }
    return false;
}

bool Parser::expectPeek(TokenType type) {
    if(peekTokenIs(type)) {
        nextToken();
        return true;
    }
    return false;
}

Precedence Parser::peekPrecedence() const {
    return tokenPrecedence(peekToken_.getTokenType());
}

Precedence Parser::curPrecedence() const {
    return tokenPrecedence(curToken_.getTokenType());
}

Precedence Parser::tokenPrecedence(TokenType type) {
    switch(type) {
        case TokenType::MY_ASSIGN:
            return Precedence::ASSIGN;
        case TokenType::MY_OR:
        case TokenType::MY_BIT_OR:
            return Precedence::OR;
        case TokenType::MY_BIT_CARET:
            return Precedence::XOR;
        case TokenType::MY_AND:
        case TokenType::MY_BIT_AND:
            return Precedence::AND;
        case TokenType::MY_EQUAL:
        case TokenType::MY_NOT_EQUAL:
            return Precedence::EQUALS;
        case TokenType::MY_GT:
        case TokenType::MY_LT:
        case TokenType::MY_EQGT:
        case TokenType::MY_EQLT:
            return Precedence::LESSGREATER;
        case TokenType::MY_PLUS:
        case TokenType::MY_MINUS:
            return Precedence::SUM;
        case TokenType::MY_MULTI:
        case TokenType::MY_DIV:
        case TokenType::MY_MOD:
            return Precedence::PRODUCT;
        case TokenType::MY_LPAREN:
        case TokenType::MY_DOT:
        case TokenType::MY_LBRACK:
            return Precedence::CALL;
        default:
            return Precedence::LOWEST;
    }
}

Parser::Parser(UPtr<Lexer> lexer) : lexer_(std::move(lexer)) {
    nextToken();    
    nextToken();
    // 归位 token 指针
}

Array<ErrorInfo> Parser::getErrors() const {
    return errors_;
}

// =================================================================================================================

UPtr<Program> Parser::parseProgram() {
    // 只进行收集
    // 创建程序对象
    auto program = std::make_unique<Program>();
    while(curToken_.getTokenType() != TokenType::MY_EOF) {
        auto stmt = parseStatement();   // 交给解析函数执行
        if(stmt != nullptr) {
            program->statements.push_back(std::move(stmt));
        } else {
            nextToken();
        }
    }
    return program;
}

UPtr<Program> Parser::parseProgramWithoutTopLevelStatements() {
    // 只进行收集
    // 创建程序对象
    auto program = std::make_unique<Program>();
    while(curToken_.getTokenType() != TokenType::MY_EOF) {
        auto stmt = parseStatement();   // 交给解析函数执行
        if(stmt != nullptr) {
            NodeType tp = stmt->type();
            switch(tp) {
                // 只允许注解语句、函数声明、结构体声明、let 、 const语句
                // import , export
                case NodeType::ANNOTATION_STATEMENT:
                case NodeType::FUNCTION_DECLARE_STATEMENT:
                case NodeType::STRUCT_DECLARE_STATEMENT:
                case NodeType::LET_STATEMENT:
                case NodeType::CONST_STATEMENT:
                case NodeType::IMPORT_STATEMENT:
                case NodeType::EXPORT_STATEMENT:
                    program->statements.push_back(std::move(stmt));
                    break;
                default:
                    ErrorInfo ei(curToken_);
                    ei.setMsg(" not allow excutable expression ",ErrorType::LEXER);
                    errors_.push_back(ei);
            }
        } else {
            nextToken();
        }
    }
    return program;
}

UPtr<Statement> Parser::parseStatement() {
    // 分发函数
    switch(curToken_.getTokenType()) {
        case TokenType::MY_LET:
            // 解析let statement
            return parseLetStatement();
        case TokenType::MY_CONST:
            // 解析const statement
            return parseConstStatement();
        case TokenType::MY_RETURN:
            // 解析 return 语句
            return parseReturnStatement();
        case TokenType::MY_CONTINUE:
            // 解析 continue 语句
            return parseContinueStatement();
        case TokenType::MY_BREAK:
            // 解析 break 语句
            return parseBreakStatement();
        case TokenType::MY_IF:
            // 解析 if 语句
            return parseIfStatement();
        case TokenType::MY_SWITCH:
            // 解析 switch 语句
            return parseSwitchStatement();
        case TokenType::MY_FOR:
            // 解析 for 语句
            return parseForStatement();
        case TokenType::MY_WHILE:
            // 解析 while 语句
            return parseWhileStatement();
        case TokenType::MY_FUNCTION:
            // 解析函数声明
            return parseFunctionDeclareStatement();
        case TokenType::MY_STRUCT:
            // 解析结构体声明
            return parseStructDeclareStatement();
        case TokenType::MY_IMPORT:
            // 解析导入语句声明
            return parseImportStatement();
        case TokenType::MY_EXPORT:
            // 导出语句声明
            return parseExportStatement();
        case TokenType::MY_AT:
            // 解析注解
            return parseAnnotationStatement();
        case TokenType::MY_LBRACE:
            // 解析块语句
            return parseBlockStatement();
        default:
            return parseExpressionStatement();
    }
}

UPtr<Expression> Parser::parseExpression(Precedence precedence) {
    // 100 * -10 + 10 * 20
    // -100  !flag flag prefix
    // 指向第一个标识符  -1 + 1
    // 不处理分号，处理完指向分号
    auto prefix = prefixParseFn(curToken_.getTokenType());   // 获得前缀处理函数，来进行处理
    if(prefix == nullptr) {
        // 没有前缀需要处理，标识符也算前缀
        return nullptr;
    }
    auto leftExp = prefix();
    while(precedence < curPrecedence()) {
        auto infix = infixParseFn(curToken_.getTokenType());
        if(infix == nullptr) {
            return leftExp;
        }
        leftExp = infix(std::move(leftExp));
    }
    return leftExp;
}

UPtr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto stmt = std::make_unique<ExpressionStatement>();
    stmt->token = curToken_;
    stmt->expression = parseExpression(Precedence::LOWEST);
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("expression need end with \' ; \' ",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return stmt;

}

UPtr<Expression> Parser::parseIdentifier() {
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();
    nextToken();
    return ident;
}

UPtr<Expression> Parser::parseIntegerLiteral() {
    auto integer = std::make_unique<IntegerLiteral>();
    integer->token = curToken_;
    integer->value = curToken_.getLiteral();
    nextToken();
    return integer;

}

UPtr<Expression> Parser::parseFloatingLiteral() {
    auto fl = std::make_unique<FloatLiteral>();
    fl->token = curToken_;
    fl->value = curToken_.getLiteral();
    nextToken();
    return fl;
}

UPtr<Expression> Parser::parseStringLiteral() {
    auto str = std::make_unique<StringLiteral>();
    str->token = curToken_;
    str->value = curToken_.getLiteral();
    nextToken();
    return str;
}

UPtr<Expression> Parser::parseBooleanLiteral() {
    auto bl = std::make_unique<BoolLiteral>();
    bl->token = curToken_;
    bl->value = curToken_.getLiteral() == "true" ? true : false;
    nextToken();
    return bl;
}

UPtr<Expression> Parser::parseNullLiteral() {
    auto nil = std::make_unique<NullLiteral>();
    nextToken();    // 不需要存储 null 字面量
    return nil;
}

UPtr<TypeLiteral> Parser::parseTypeLiteral() {
    // 当前curToken必须是合法的类型token
    if(!curToken_.isTypeToken()) {
        ErrorInfo ei(curToken_);
        ei.setMsg("type annotation is not valid ",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    auto typel = std::make_unique<TypeLiteral>();
    typel->token = curToken_;
    typel->name = curToken_.getLiteral();
    nextToken();    // 消耗类型名

    // 泛型参数：list<i32>、map<string, i32>
    if (curToken_.getTokenType() == TokenType::MY_LT) {
        nextToken();    // 消耗 <
        while (curToken_.getTokenType() != TokenType::MY_GT) {
            auto param = parseTypeLiteral();
            if (param) {
                typel->typeParams.push_back(std::move(param));
            } else {
                // 参数解析失败
                ErrorInfo ei(curToken_);
                ei.setMsg(" fatal parser type annotation ",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            if (curToken_.getTokenType() == TokenType::MY_COMMA) {
                nextToken();    // 消耗 ,
            } else {
                break;
            }
        }
        // 现在应该是 >
        if (curToken_.getTokenType() != TokenType::MY_GT) {
            ErrorInfo ei(curToken_);
            ei.setMsg(" type annotation need end with > ",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();    // 消耗 >
    }

    return typel;
}

UPtr<Expression> Parser::parseFunctionLiteral() {
    // let x : lambda = fn(x:bytes):any{...};
    // 当前是 fn
    auto lit = std::make_unique<FunctionLiteral>();
    lit->token = curToken_;

    // 消耗 fn
    nextToken();
    // 消耗 (
    nextToken();

    // 解析函数列表
    auto params = parseParamsList();
    if(params == nullptr) {
        return nullptr;
    }

    // 现在应该是 )

    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("lambda expression need end with )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();    // 消耗 )
    
    // 消耗 :
    nextToken();

    auto returnType = parseTypeLiteral();

    if(returnType == nullptr) {
        return nullptr;
    }

    // 现在应该是 {
    auto stmt = parseBlockStatement();

    if(stmt == nullptr) {
        return nullptr;
    }

    // 组装
    lit->params = std::move(*params);
    lit->body = std::move(stmt);
    lit->returnType = std::move(returnType);

    return lit;
}

UPtr<Array<UPtr<Expression>>> Parser::parseListExpression(TokenType endToken) {
    auto list = std::make_unique<Array<UPtr<Expression>>>();
    if(curTokenIs(endToken)) {
        return list;    // 空列表
    }
    while(true) {
        // 现在应该是expression
        auto expr = parseExpression(Precedence::LOWEST);
        if(!expr) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser list expression",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        list->push_back(std::move(expr));
        if(curTokenIs(endToken)) {
            // 遇到 ] 说明也该结束了
            break;
        }
        if(!curTokenIs(TokenType::MY_COMMA)) {
            break;  // 不是 , 说明解析完毕了
        } else {    
            nextToken();    //  消耗 ,
        }
    }
    return list;
}

UPtr<ArrayLiteral> Parser::parseArrayLiteral() {
    //现在是 [
    auto arr = std::make_unique<ArrayLiteral>();
    arr->token = curToken_;
    nextToken();    //  消耗 [
    auto list = parseListExpression(TokenType::MY_RBRACK);
    if(!list) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser array element",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    arr->elements = std::move(*list);
    // 现在是 ]
    if(!curTokenIs(TokenType::MY_RBRACK)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("array need end with ]",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return arr;
}

UPtr<MapLiteral> Parser::parseMapLiteral() {
    // 现在是 {
    auto hash = std::make_unique<MapLiteral>();
    hash->token = curToken_;
    nextToken();
    if(curTokenIs(TokenType::MY_RBRACE)) {
        // 空哈希表
        nextToken();
        return hash;
    }

    while(true) {
        // k : v ,
        // 现在是第一个字符
        auto key = parseExpression(Precedence::LOWEST);

        if(!key) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser map key",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        // 现在应该是 :
        if(!curTokenIs(TokenType::MY_COLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("map need : between key and value",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();    // 消费 ,
        auto val = parseExpression(Precedence::LOWEST);
        if(!val) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser map value",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        hash->pairs.push_back({std::move(key),std::move(val)});

        // 现在如果是 , 就继续 loop , 否则就是应该结束了
        if(curTokenIs(TokenType::MY_COMMA)) {
            // 消费 ,
            nextToken();
            continue;
        } else {
            break;
        }
    }

    if(!curTokenIs(TokenType::MY_RBRACE)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("map need end with }",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return hash;
}

UPtr<Expression> Parser::parsePrefixExpression()
{
    auto prefix = std::make_unique<PrefixExpression>();
    // 现在指向前缀的标识符
    prefix->token = curToken_;
    prefix->op = curToken_.getLiteral();
    nextToken();    // 消耗标识符
    prefix->right = parseExpression(Precedence::PREFIX);
    return prefix;
}

UPtr<Expression> Parser::parseInfixExpression(UPtr<Expression> left) {
    auto expr = std::make_unique<InfixExpression>();
    expr->token = curToken_; // op
    expr->op = curToken_.getLiteral();
    expr->left = std::move(left);

    auto precedence = curPrecedence();  // op precedence
    nextToken();
    // curtoken -> right
    expr->right = parseExpression(precedence);

    return expr;
}

UPtr<Expression> Parser::parseGroupedExpression() {
    // 当前token是 '('
    nextToken();    // 消耗 '('
    auto expr = parseExpression(Precedence::LOWEST);

    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("grouped expression need end with )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    return expr;
}

UPtr<BlockStatement> Parser::parseBlockStatement() {
    // 当前是 {
    auto block = std::make_unique<BlockStatement>();
    block->token = curToken_;
    nextToken();
    while(!curTokenIs(TokenType::MY_RBRACE) && !curTokenIs(TokenType::MY_EOF)) {
        auto stmt = parseStatement();
        if(stmt != nullptr) {
            block->statements.push_back(std::move(stmt));
        } else {
            nextToken();
        }
    }
    nextToken();    // 消耗}
    return block;
}

UPtr<Array<Pair<UPtr<Expression>, UPtr<TypeLiteral>>>> Parser::parseParamsList() {
    // 不解析 (
    // 当前指向第一个token
    auto params = std::make_unique<Array<Pair<UPtr<Expression>, UPtr<TypeLiteral>>>>();
    // 无参
    if(!curTokenIs(TokenType::MY_IDENT)) {
        // 不是标识符，就代表无参，不消耗
        return params;
    } 
    while(true) {
        auto name = parseIdentifier();
        if(!name) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser param name",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr; // 解析存在错误
        }

        // 当前 token指向 :
        if(!curTokenIs(TokenType::MY_COLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("param need : after name",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        nextToken(); // 消耗 :
        auto type = parseTypeLiteral();
        if(!type) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser param type",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        params->push_back({std::move(name),std::move(type)});

        // 更多参数
        if(curTokenIs(TokenType::MY_COMMA)) {
            // 逗号代表有更多的参数
            nextToken(); // 消耗掉,
        }else {
            // 没有参数了
            break;
        }

    }

    return params;
}

UPtr<LetStatement> Parser::parseLetStatement() {
    // 解析 let statement
    // 标准的let语句： let <标识符> (: <类型>) = <表达式>
    auto stmt = std::make_unique<LetStatement>();
    stmt->token = curToken_;    // 保存 let
    if(!expectPeek(TokenType::MY_IDENT)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("let need variable name",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    // 这个时候 curToken 指向标识符
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();
    // name就是ident
    stmt->name = std::move(ident);

    nextToken();    // 消耗掉标识符
    
    // 当前token 如果是 :
    if(curTokenIs(TokenType::MY_COLON)) {
        // 消耗 :，前进到类型名起始token
        nextToken();
        stmt->varType = parseTypeLiteral();
    }

    // 不强制类型标注
    // 当前的token就是 =
    if(!curTokenIs(TokenType::MY_ASSIGN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("let need = init value",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();    // 消耗掉 =

    stmt->value = parseExpression(Precedence::LOWEST);

    // 现在指向 ;
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("let need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        nextToken();
        return nullptr;
    }

    // 消费 ;
    nextToken();

    return stmt;
}

UPtr<ConstStatement> Parser::parseConstStatement() {
    // 解析 const statement
    // 标准const语句： const <标识符> : <类型> = <表达式>
    auto stmt = std::make_unique<ConstStatement>();
    stmt->token = curToken_;    // 保存 const
    if(!expectPeek(TokenType::MY_IDENT)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("const need variable name",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    // 这个时候 curToken 指向标识符
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();
    // name就是ident
    stmt->name = std::move(ident);

    nextToken();    // 消耗掉标识符
    
    // 当前token 如果是 :
    if(curTokenIs(TokenType::MY_COLON)) {
        // 消耗 :，前进到类型名起始token
        nextToken();
        stmt->varType = parseTypeLiteral();
    }

    // 现在的token就是 =
    if(!curTokenIs(TokenType::MY_ASSIGN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("const need = init value",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();    // 消耗掉 =

    stmt->value = parseExpression(Precedence::LOWEST);

    // 现在指向 ;
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("const need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        nextToken();
        return nullptr;
    }

    // 消费 ;
    nextToken();

    return stmt;
}

UPtr<ReturnStatement> Parser::parseReturnStatement() {
    auto stmt = std::make_unique<ReturnStatement>();
    // return <表达式>

    // 当前是 return 这个 token
    stmt->token = curToken_;

    nextToken();

    stmt->value = parseExpression(Precedence::LOWEST);

    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("return need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return stmt;

}

UPtr<BreakStatement> Parser::parseBreakStatement() {
    // 当前是break
    auto stmt = std::make_unique<BreakStatement>();

    stmt->token = curToken_;
    nextToken();
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("break need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return stmt;
}

UPtr<ContinueStatement> Parser::parseContinueStatement() {
    // 当前是contine
    auto stmt = std::make_unique<ContinueStatement>();

    stmt->token = curToken_;
    nextToken();
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("continue need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return stmt;
}

UPtr<IfStatement> Parser::parseIfStatement() {
    // curToken 是 if
    auto stmt = std::make_unique<IfStatement>();
    stmt->token = curToken_;
    nextToken();

    // 现在应该是左括号
    if(!curTokenIs(TokenType::MY_LPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("if need ( condition )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    auto cond = parseExpression(Precedence::LOWEST);
    if(!cond) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser if condition",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    stmt->condition = std::move(cond);
    // 现在应该是右括号
    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("if condition need end with )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();    //消耗)

    auto fs = parseBlockStatement();
    if(!fs) {
        ErrorInfo ei(curToken_);
        ei.setMsg("if need body block",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    stmt->consequence = std::move(fs);

    // 现在指向else elif 或者下一个语句

    // 解析 elif
    while(curTokenIs(TokenType::MY_ELIF)) {
        // 现在的token是 elif
        nextToken();
        // 现在的token是 (
        if(!curTokenIs(TokenType::MY_LPAREN)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("elif need ( condition )",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();
        // 现在的token是expression
        auto expr = parseExpression(Precedence::LOWEST);
        if(!expr) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser elif condition",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        // 现在的token是 )
        if(!curTokenIs(TokenType::MY_RPAREN)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("elif condition need end with )",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();
        //现在的token是{
        auto bl = parseBlockStatement();
        if(!bl) {
            ErrorInfo ei(curToken_);
            ei.setMsg("elif need body block",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        stmt->elifClauses.push_back({std::move(expr),std::move(bl)});

    }


    // 解析 else
    if(curTokenIs(TokenType::MY_ELSE)) {
        nextToken();    //消费else

        auto bl = parseBlockStatement();
        if(!bl) {
            ErrorInfo ei(curToken_);
            ei.setMsg("else need body block",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        stmt->alternative = std::move(bl);
    }

    return stmt;

}

UPtr<SwitchStatement> Parser::parseSwitchStatement() {
    // 当前是switch
    auto stmt = std::make_unique<SwitchStatement>();
    stmt->token = curToken_;
    nextToken();

    // 现在应该是 (
    if(!curTokenIs(TokenType::MY_LPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("switch need ( expr )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    auto expr = parseExpression(Precedence::LOWEST);

    if(!expr) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser switch condition",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    stmt->condition = std::move(expr);

    // 现在应该是 )
    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("switch condition need end with )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    // switch语句的{
    if(!curTokenIs(TokenType::MY_LBRACE)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("switch need body block",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    // 现在应该是第一个case
    // case 流
    while(curTokenIs(TokenType::MY_CASE)) {
        // 当前是 case
        nextToken();
        
        //现在是表达式
        auto subExpr = parseExpression(Precedence::LOWEST);
        if(!subExpr) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser case expr",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        if(!curTokenIs(TokenType::MY_COLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("case need : after expr",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        nextToken();

        // 现在是{ 或者下一个case , 需要支持case 穿透
        if(curTokenIs(TokenType::MY_CASE)) {
            stmt->cases.push_back({std::move(subExpr),nullptr});  //   没有block
        } else if(curTokenIs(TokenType::MY_LBRACE)) {
            auto block = parseBlockStatement();
            if(!block) {
                ErrorInfo ei(curToken_);
                ei.setMsg("case block parse failed",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            stmt->cases.push_back({std::move(subExpr),std::move(block)});
        } else if(curTokenIs(TokenType::MY_RBRACE)) {
            break;
        } else {
            ErrorInfo ei(curToken_);
            ei.setMsg("case body invalid",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
    }

    // default流
    if(curTokenIs(TokenType::MY_DEFAULT)) {
        nextToken();
        //现在应该是 :
        if(!curTokenIs(TokenType::MY_COLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("default need :",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();

        // 现在应该是 {
        if(!curTokenIs(TokenType::MY_LBRACE)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("default need body block",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        auto block = parseBlockStatement();

        if(!block) {
            ErrorInfo ei(curToken_);
            ei.setMsg("default body parse failed",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        stmt->defaultCase = std::move(block);

    }

    if(!curTokenIs(TokenType::MY_RBRACE)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("switch need end with }",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return stmt;
}

UPtr<ForStatement> Parser::parseForStatement() {
    // for(init;condition;update){body}
    // for(let i in 可迭代容器){body}
    // 上面的增强for翻译：
    // for(let $idx = 0 ; $idx < len(可迭代容器);$idx = $idx + 1) {let i = 可迭代容器[$idx];body}
    // for 不强制类型标注，但是其他的需要
    // 当前的 token 是 for
    auto stmt = std::make_unique<ForStatement>();
    stmt->token = curToken_;

    nextToken();

    // 当前是(
    if(!curTokenIs(TokenType::MY_LPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("for need ( ... )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    // ==================== 解析 init 部分 ====================
    // 现在是 let const ident 开头或者 ;

    if(curTokenIs(TokenType::MY_SEMICOLON)) {
        // 无 init
        nextToken();
    } else if(curTokenIs(TokenType::MY_LET) || curTokenIs(TokenType::MY_CONST)) {
        bool isConst = curTokenIs(TokenType::MY_CONST);
        nextToken();

        if(!curTokenIs(TokenType::MY_IDENT)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("for need var name",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        auto varIdent = std::make_unique<Identifier>();
        varIdent->token = curToken_;
        varIdent->value = curToken_.getLiteral();
        nextToken();    // 跳过变量名

        // 关键分支：看 curToken 是 = (C风格) 还是 in (for-in)
        if(curTokenIs(TokenType::MY_IN)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("unsupport for-in grammar",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
            // ==================== for-in ====================
            nextToken();    // 跳过 in
            auto iterable = parseExpression(Precedence::LOWEST);
            if(!iterable) {
                ErrorInfo ei(curToken_);
                ei.setMsg("fatal parser for-in iterable",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }

            if(!curTokenIs(TokenType::MY_RPAREN)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for-in need ) after iterable",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            nextToken();    // 跳过 )

            auto forBody = parseBlockStatement();
            if(!forBody) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for-in need body block",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }

            // ---- 翻译 for(let i in iterable){body} 为 C 风格 ----
            // 构造唯一临时变量名
            String idxName = "$idx";
            Token idxToken;
            idxToken.setTL(TokenType::MY_IDENT, idxName);

            auto idxLet = std::make_unique<LetStatement>();
            idxLet->token = idxToken;
            auto idxIdent = std::make_unique<Identifier>();
            idxIdent->token = idxToken;
            idxIdent->value = idxName;
            idxLet->name = std::move(idxIdent);
            auto zeroLit = std::make_unique<IntegerLiteral>();
            zeroLit->value = "0";
            idxLet->value = std::move(zeroLit);
            stmt->init = std::move(idxLet);
            auto typeLit = std::make_unique<TypeLiteral>();

            Token tk = Token(TokenType::MY_I32,"i32");
            typeLit->name = tk.getLiteral();
            typeLit->token = tk;
            idxLet->varType = std::move(typeLit);

            // 2. condition: __fo_0 < len(iterable)
            auto condLeft = std::make_unique<Identifier>();
            condLeft->token = idxToken;
            condLeft->value = idxName;

            // len(iterable) — 构造 CallExpression
            Token lenToken;
            lenToken.setTL(TokenType::MY_IDENT, "len");
            auto lenCall = std::make_unique<CallExpression>();
            lenCall->token = lenToken;
            auto lenIdent = std::make_unique<Identifier>();
            lenIdent->token = lenToken;
            lenIdent->value = "len";
            lenCall->func = std::move(lenIdent);
            lenCall->arguments.push_back(std::move(iterable));

            auto condExpr = std::make_unique<InfixExpression>();
            condExpr->token = idxToken;
            condExpr->op = "<";
            condExpr->left = std::move(condLeft);
            condExpr->right = std::move(lenCall);
            stmt->condition = std::move(condExpr);

            // 3. update: __fo_0 = __fo_0 + 1
            auto upLeft = std::make_unique<Identifier>();
            upLeft->token = idxToken;
            upLeft->value = idxName;

            auto addRight = std::make_unique<IntegerLiteral>();
            addRight->value = "1";

            auto addExpr = std::make_unique<InfixExpression>();
            addExpr->op = "+";
            auto addLeft = std::make_unique<Identifier>();
            addLeft->token = idxToken;
            addLeft->value = idxName;
            addExpr->left = std::move(addLeft);
            addExpr->right = std::move(addRight);

            auto upExpr = std::make_unique<InfixExpression>();
            upExpr->token = idxToken;
            upExpr->op = "=";
            upExpr->left = std::move(upLeft);
            upExpr->right = std::move(addExpr);
            stmt->update = std::move(upExpr);

            // 4. body 前面插入 let i = iterable[__fo_0];
            Token varToken;
            varToken.setTL(TokenType::MY_IDENT, varIdent->value);
            auto innerLet = std::make_unique<LetStatement>();
            innerLet->token = varToken;

            // varIdent 是之前的变量名，直接用
            innerLet->name = std::move(varIdent);

            // iterable[__fo_0] — 构造 IndexExpression
            auto idxExpr = std::make_unique<IndexExpression>();
            idxExpr->token = idxToken;
            auto idxArg = std::make_unique<Identifier>();
            idxArg->token = idxToken;
            idxArg->value = idxName;
            idxExpr->index = std::move(idxArg);
            // left 需要 clone iterable… 但 unique_ptr 不能拷贝
            // 所以这里需要依赖外部传入的 iterable，但 iterable 已经被 move 进 lenCall 了
            // 这里需要先存一份，翻译时就需要多 pass

            // 重新解析 iterable — 因为已经消费掉了，没法重新解析
            // 这里说明 parse 阶段做翻译的局限性：需要保留 iterable 的副本
            // TODO: 等 IndexExpression 和 CallExpression 的解析器实现后，
            //       这里的翻译可以挪到代码生成阶段做，更干净
            // 目前先把 body 原样返回，iterable[$idx] 部分暂缺
            // 但为了能输出正确结构，将 idxExpr 的 left 设为 nullptr
            idxExpr->left = nullptr;    // 待后续补充
            innerLet->value = std::move(idxExpr);
            forBody->statements.insert(forBody->statements.begin(), std::move(innerLet));

            stmt->body = std::move(forBody);
            return stmt;

        } else if(curTokenIs(TokenType::MY_COLON)) {
            // ==================== C风格 for(let x : type = expr; ...) ====================
            nextToken();    // 跳过 :
            auto initType = parseTypeLiteral();

            if(!curTokenIs(TokenType::MY_ASSIGN)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need = after type",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            nextToken();    // 跳过 =
            auto initVal = parseExpression(Precedence::LOWEST);
            if(!initVal) {
                ErrorInfo ei(curToken_);
                ei.setMsg("fatal parser for init value",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }

            if(!curTokenIs(TokenType::MY_SEMICOLON)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need ; after init",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            nextToken();

            // 强制类型标注
            if(isConst) {
                auto constStmt = std::make_unique<ConstStatement>();
                constStmt->token = varIdent->token;
                constStmt->name = std::move(varIdent);
                constStmt->varType = std::move(initType);
                constStmt->value = std::move(initVal);
                stmt->init = std::move(constStmt);
            } else {
                auto letStmt = std::make_unique<LetStatement>();
                letStmt->token = varIdent->token;
                letStmt->name = std::move(varIdent);
                letStmt->varType = std::move(initType);
                letStmt->value = std::move(initVal);
                stmt->init = std::move(letStmt);
            }

            // condition
            stmt->condition = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::MY_SEMICOLON)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need ; after condition",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            nextToken();

            // update
            stmt->update = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::MY_RPAREN)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need ) after update",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            nextToken();

            stmt->body = parseBlockStatement();
            if(!stmt->body) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need body block",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            return stmt;
        } else {
            ErrorInfo ei(curToken_);
            ei.setMsg("for init invalid, need : type = expr or in iterable",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
    } else {
        // C 风格: for(expr; condition; update) — 无 let/const
        auto initExpr = parseExpression(Precedence::LOWEST);
        if(!initExpr) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser for expr init",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }

        auto exprStmt = std::make_unique<ExpressionStatement>();
        exprStmt->token = curToken_;
        exprStmt->expression = std::move(initExpr);
        stmt->init = std::move(exprStmt);

        if(!curTokenIs(TokenType::MY_SEMICOLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("for need ; after init expr",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();

        // condition
        if(!curTokenIs(TokenType::MY_SEMICOLON)) {
            stmt->condition = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::MY_SEMICOLON)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need ; after condition",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
        }
        nextToken();

        // update
        if(!curTokenIs(TokenType::MY_RPAREN)) {
            stmt->update = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::MY_RPAREN)) {
                ErrorInfo ei(curToken_);
                ei.setMsg("for need ) after update",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
        }
        nextToken();

        stmt->body = parseBlockStatement();
        if(!stmt->body) {
            ErrorInfo ei(curToken_);
            ei.setMsg("for need body block",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        return stmt;
    }

    // ==================== 解析 condition 和 update ====================
    // 走到这里说明 init 是空（已经处理了 ;）
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        stmt->condition = parseExpression(Precedence::LOWEST);
    }
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("for need ; after condition",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    if(!curTokenIs(TokenType::MY_RPAREN)) {
        stmt->update = parseExpression(Precedence::LOWEST);
    }
    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("for need ) after update",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    stmt->body = parseBlockStatement();
    if(!stmt->body) {
        ErrorInfo ei(curToken_);
        ei.setMsg("for need body block",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    return stmt;
}

UPtr<WhileStatement> Parser::parseWhileStatement() {
    auto stmt = std::make_unique<WhileStatement>();
    // 现在是 while 这个 token
    stmt->token = curToken_;
    nextToken();    // 消费 while

    if(!curTokenIs(TokenType::MY_LPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("while need ( condition )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();    // 消费 (
    auto expr = parseExpression(Precedence::LOWEST);

    if(!expr) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser while condition",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    stmt->condition = std::move(expr);

    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("while condition need end with )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    nextToken();

    if(!curTokenIs(TokenType::MY_LBRACE)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("while need body block",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    auto block = parseBlockStatement();
    if(!block) {
        ErrorInfo ei(curToken_);
        ei.setMsg("while body parse failed",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    stmt->body = std::move(block);
    return stmt;

}

UPtr<FunctionDeclareStatement> Parser::parseFunctionDeclareStatement() {
    // 这里展示纯function声明语法 ， 不涉及注解
    //  fn demo(params1 : i32 , params2 : i32):void;
    //  fn demo(params1 : i32 , params2 : i32):void{...}
    auto fd = std::make_unique<FunctionDeclareStatement>();
    // 当前是 fn 
    fd->token = curToken_;
    nextToken();
    // 当前是函数名
    if(!curTokenIs(TokenType::MY_IDENT)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fn need function name",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();

    fd->name = std::move(ident);
    nextToken();

    // 当前是括号
    if(!curTokenIs(TokenType::MY_LPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fn need ( params )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    auto paramList = parseParamsList();

    if(paramList == nullptr) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser fn params",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    fd->params = std::move(*paramList);

    // 当前是右括号
    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fn params need end with )",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    // 现在是 :
    if(!curTokenIs(TokenType::MY_COLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fn need : return type",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();

    auto rt = parseTypeLiteral();
    if(!rt) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser fn return type",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    fd->returnType = std::move(rt);

    // 现在指向的是 { 或者 ;
    if(curTokenIs(TokenType::MY_SEMICOLON)) {
        nextToken();
        return fd;
    } else if(curTokenIs(TokenType::MY_LBRACE)) {
        // 解析 block语句
        auto bd = parseBlockStatement();
        if(bd == nullptr) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser fn body",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        fd->body = std::move(bd);
        return fd;

    } else {
        ErrorInfo ei(curToken_);
        ei.setMsg("fn need ; or { after return type",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

}

UPtr<StructDeclareStatement> Parser::parseStructDeclareStatement() {
    /**
     * struct Person {
        name : string , 
        age : u32
       };
     */

    // 当前token是 struct
    auto decl = std::make_unique<StructDeclareStatement>();
    decl->token = curToken_;
    nextToken();    // 消费 struct

    // 现在是 标识符
    if(!curTokenIs(TokenType::MY_IDENT)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("struct need name",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    auto ident = parseIdentifier();
    if(!ident) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser struct name",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    decl->name = std::move(UPtr<Identifier>(static_cast<Identifier*>(ident.release())));

    if(!curTokenIs(TokenType::MY_LBRACE)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("struct need { fields }",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    auto list = parseParamsList();
    if(!list) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser struct fields",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    decl->fields = std::move(*list);
    if(!curTokenIs(TokenType::MY_RBRACE)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("struct need end with }",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("struct need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return decl;
}

UPtr<ImportStatement> Parser::parseImportStatement() {
    auto stmt = std::make_unique<ImportStatement>();
    stmt->token = curToken_;
    nextToken();
    if(!curTokenIs(TokenType::MY_IDENT)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("import need module name",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    auto name = parseIdentifier();
    stmt->name = UPtr<Identifier>(static_cast<Identifier*>(name.release()));
    if(!curTokenIs(TokenType::MY_SEMICOLON)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("import need end with ;",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return stmt;
}

UPtr<ExportStatement> Parser::parseExportStatement() {
    auto stmt = std::make_unique<ExportStatement>();
    // 当前 token是 export
    stmt->token = curToken_;
    nextToken();

    // 现在的token可以是 标识符 或者是 {
    if(curTokenIs(TokenType::MY_IDENT)) {
        auto ident = parseIdentifier();
        stmt->exports.push_back(UPtr<Identifier>(static_cast<Identifier*>(ident.release())));
        if(!curTokenIs(TokenType::MY_SEMICOLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("export need end with ;",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();
        return stmt;
    } else if(curTokenIs(TokenType::MY_LBRACE)) {
        nextToken();
        auto idnet1 = parseIdentifier();
        stmt->exports.push_back(UPtr<Identifier>(static_cast<Identifier*>(idnet1.release())));
        while(true) {
            if(!curTokenIs(TokenType::MY_COMMA)) {
                break;
            }
            nextToken();
            if(!curTokenIs(TokenType::MY_IDENT)) {
                break;
            }
            auto ident = parseIdentifier();
            if(!ident) {
                ErrorInfo ei(curToken_);
                ei.setMsg("fatal parser export ident",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
            stmt->exports.push_back(UPtr<Identifier>(static_cast<Identifier*>(ident.release())));
        }
        if(!curTokenIs(TokenType::MY_RBRACE)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("export need end with }",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();
        if(!curTokenIs(TokenType::MY_SEMICOLON)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("export need end with ;",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();
        return stmt;
    } else {
        ErrorInfo ei(curToken_);
        ei.setMsg("export need ident or { }",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
}

UPtr<AnnotationStatement> Parser::parseAnnotationStatement() {
    // 当前的token是@
    // 处理多注解
    auto stmt = std::make_unique<AnnotationStatement>();
    // 注解名字 -> id 映射表 ， 用于方便switch 运算
    static Map<String,i32> annotationMap = {
        {"native",1},
        {"platform",2},
        {"method",3},
        {"object",4},
        {"async",5}
    };
    while(curTokenIs(TokenType::MY_AT)) {
        // 当前的是 @
        Token at = curToken_;
        nextToken();

        // 现在应该是标识符
        if(!curTokenIs(TokenType::MY_IDENT)) {
            ErrorInfo ei(curToken_);
            ei.setMsg("annotation need name",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        auto it = annotationMap.find(curToken_.getLiteral());
        if(it == annotationMap.end()) {
            ErrorInfo ei(curToken_);
            ei.setMsg("unknown annotation type",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        auto ident = parseIdentifier();
        if(!ident) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser annotation name",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        i32 id = it->second;

        // 现在应该是标识符后的token了
        switch(id) {
            case 1: {
                // native 
                auto anno = std::make_unique<NativeAnnotation>();
                
                anno->name = UPtr<Identifier>(static_cast<Identifier*>(ident.release()));
                anno->token = at;

                if(!curTokenIs(TokenType::MY_LPAREN)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("native need ( path )",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                nextToken();
                if(!curTokenIs(TokenType::MY_STRING_LITERAL)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("native need string path",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                anno->path = curToken_.getLiteral();
                nextToken();
                if(!curTokenIs(TokenType::MY_RPAREN)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("native need ) after path",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                nextToken();
                stmt->annotations.push_back(std::move(anno));
            }
            break;
            case 2: {
                auto anno = std::make_unique<PlatformAnnotation>();
                anno->name = UPtr<Identifier>(static_cast<Identifier*>(ident.release()));
                anno->token = at;
                if(!curTokenIs(TokenType::MY_LPAREN)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("platform need ( list )",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                nextToken();
                if(!curTokenIs(TokenType::MY_IDENT)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("platform need platform name",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                while(true) {
                    auto identf = parseIdentifier();
                    anno->platforms.push_back(UPtr<Identifier>(static_cast<Identifier*>(identf.release())));
                    if(!curTokenIs(TokenType::MY_COMMA)) {
                        break;
                    } else {
                        nextToken();
                    }
                }
                if(!curTokenIs(TokenType::MY_RPAREN)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("platform need )",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                nextToken();
                stmt->annotations.push_back(std::move(anno));
            }
            break;
            case 3: {
                auto anno = std::make_unique<MethodAnnotation>();
                anno->name = UPtr<Identifier>(static_cast<Identifier*>(ident.release()));
                if(!curTokenIs(TokenType::MY_LPAREN)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("method need ( Type )",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                nextToken();
                auto typeName = parseIdentifier();
                if(!typeName) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("fatal parser method type",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                anno->token = at;
                anno->annoType = UPtr<Identifier>(static_cast<Identifier*>(typeName.release()));
                if(!curTokenIs(TokenType::MY_RPAREN)) {
                    ErrorInfo ei(curToken_);
                    ei.setMsg("method need )",ErrorType::ASTBUILD);
                    errors_.push_back(ei);
                    return nullptr;
                }
                nextToken();
                stmt->annotations.push_back(std::move(anno));
            }
            break;
            case 4: {
                auto anno = std::make_unique<ObjectAnnotation>();
                anno->name = UPtr<Identifier>(static_cast<Identifier*>(ident.release()));
                anno->token = at;
                stmt->annotations.push_back(std::move(anno));
            }
            break;
            case 5: {
                auto anno = std::make_unique<AsyncAnnotation>();
                anno->name = UPtr<Identifier>(static_cast<Identifier*>(ident.release()));
                anno->token = at;
                stmt->annotations.push_back(std::move(anno));
            }
            break;
            default: {
                ErrorInfo ei(curToken_);
                ei.setMsg("unknown annotation id",ErrorType::ASTBUILD);
                errors_.push_back(ei);
                return nullptr;
            }
        }
    }
    // 现在应该是 fn 或者 struct

    if(curTokenIs(TokenType::MY_FUNCTION)) {
        auto func = parseFunctionDeclareStatement();
        if(!func) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser annotation fn target",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        stmt->target = std::move(func);
    } else if(curTokenIs(TokenType::MY_STRUCT)) {
        auto stru = parseStructDeclareStatement();
        if(!stru) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser annotation struct target",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        stmt->target = std::move(stru);
    } else {
        ErrorInfo ei(curToken_);
        ei.setMsg("annotation target need fn or struct",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    return stmt;
}

UPtr<AwaitExpression> Parser::parseAwaitExpression() {
    auto expr = std::make_unique<AwaitExpression>();
    expr->token = curToken_;

    nextToken();
    // 现在是新的表达式
    if(curTokenIs(TokenType::MY_AWAIT)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("cannot await after await",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    auto args = parseExpression(Precedence::LOWEST);
    if(!args) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser await argument",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    expr->argument = std::move(args);

    return expr;
}

UPtr<Array<UPtr<Expression>>> Parser::parseCallArguments() {
    Array<UPtr<Expression>> args;
    // 当前的token是第一个参数
    if(curTokenIs(TokenType::MY_RPAREN)) {
        return std::make_unique<Array<UPtr<Expression>>>(std::move(args)); // 括号都交给 parseCallExpression 处理
    }
    // 解析第一个参数
    auto first = parseExpression(Precedence::LOWEST);
    if(!first) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser call arg",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    args.push_back(std::move(first));

    while(curTokenIs(TokenType::MY_COMMA)) {
        nextToken();
        auto ident = parseExpression(Precedence::LOWEST);
        if(!ident) {
            ErrorInfo ei(curToken_);
            ei.setMsg("fatal parser call arg after ,",ErrorType::ASTBUILD);
            errors_.push_back(ei);
            return nullptr;
        }
        args.push_back(std::move(ident));
    }
    return std::make_unique<Array<UPtr<Expression>>>(std::move(args));
    // 现在是)
}

UPtr<CallExpression> Parser::parseCallExpression(UPtr<Expression> func) {
    // 现在是 (
    auto expr = std::make_unique<CallExpression>();
    expr->token = curToken_;
    nextToken();
    
    
    auto args = parseCallArguments();

    if(!args) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser call args",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }

    if(!curTokenIs(TokenType::MY_RPAREN)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("call need ) after args",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    expr->arguments = std::move(*args);

    expr->func = std::move(func);


    return expr;
}

UPtr<IndexExpression> Parser::parseIndexExpression(UPtr<Expression> left) {
    // 现在是 [
    auto expr = std::make_unique<IndexExpression>();
    expr->token = curToken_;
    expr->left = std::move(left);

    nextToken();    // 消耗 [
    
    auto index = parseExpression(Precedence::LOWEST);
    if(!index) {
        ErrorInfo ei(curToken_);
        ei.setMsg("fatal parser index expr",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    expr->index = std::move(index);
    if(!curTokenIs(TokenType::MY_RBRACK)) {
        ErrorInfo ei(curToken_);
        ei.setMsg("index need ]",ErrorType::ASTBUILD);
        errors_.push_back(ei);
        return nullptr;
    }
    nextToken();
    return expr;
}

UPtr<MemberAccessExpression> Parser::parseMemberAccessExpression(UPtr<Expression> left) {
      auto expr = std::make_unique<MemberAccessExpression>();
      expr->token = curToken_;
      expr->object = std::move(left);    // object = r
      nextToken();                        // 消费 .

      // 右边必须是标识符（成员名）
      if(!curTokenIs(TokenType::MY_IDENT)) {
          ErrorInfo ei(curToken_);
          ei.setMsg("member access need a name after .", ErrorType::ASTBUILD);
          errors_.push_back(ei);
          return nullptr;
      }
      auto name = parseIdentifier();      // 只解析 w，不再 parseExpression

      // 如果后面是 ( -> 方法调用 p.getName()
      if(curTokenIs(TokenType::MY_LPAREN)) {
          auto call = parseCallExpression(std::move(name));
          expr->called = std::move(call);  // called = CallExpression(getName)
      } else {
          expr->called = std::move(name);  // called = Identifier(w)
      }
      return expr;
  }
Parser::PrefixFn Parser::prefixParseFn(TokenType type) {
    switch(type) {
        case TokenType::MY_MINUS:
        case TokenType::MY_PLUS:
        case TokenType::MY_BIT_INVERSION:
            return [this]{return parsePrefixExpression(); };
        case TokenType::MY_IDENT:
            return [this]{return parseIdentifier(); };
        case TokenType::MY_INTEGER:
            return [this]{return parseIntegerLiteral(); };
        case TokenType::MY_FLOATING:
            return [this]{return parseFloatingLiteral(); };
        case TokenType::MY_STRING_LITERAL:
            return [this]{return parseStringLiteral(); };
        case TokenType::MY_NULL:
            return [this]{return parseNullLiteral(); };
        case TokenType::MY_LPAREN:
            return [this]{return parseGroupedExpression(); };
        case TokenType::MY_TRUE:
        case TokenType::MY_FALSE:
            return [this]{return parseBooleanLiteral(); };
        case TokenType::MY_LBRACK:  // Array
            return [this]{return parseArrayLiteral(); };
        case TokenType::MY_LBRACE:  // map
            return [this]{return parseMapLiteral(); };
        case  TokenType::MY_AWAIT:
            return [this]{return parseAwaitExpression(); };
        case TokenType::MY_FUNCTION:
            return [this]{return parseFunctionLiteral(); };
        default:
            return nullptr;
    }
}

Parser::InfixFn Parser::infixParseFn(TokenType type) {
    // 1 + 1 , 1 * 1 ... LEFT OP RIGHT
    switch(type) {
        case TokenType::MY_PLUS:        // +
        case TokenType::MY_MINUS:       // -
        case TokenType::MY_MULTI:       // *
        case TokenType::MY_DIV:         // /
        case TokenType::MY_MOD:         // %
        case TokenType::MY_BIT_AND:     // &
        case TokenType::MY_BIT_OR:      // |
        case TokenType::MY_BIT_CARET:   // ^
        case TokenType::MY_ASSIGN:      // =
        case TokenType::MY_NOT_EQUAL:   // !=
        case TokenType::MY_EQUAL:       // ==
        case TokenType::MY_EQGT:        // >=
        case TokenType::MY_EQLT:        // <=
        case TokenType::MY_LT:          // <
        case TokenType::MY_GT:          // >
        case TokenType::MY_AND:         // and
        case TokenType::MY_OR:          // or
            return [this](auto left){return parseInfixExpression(std::move(left));};
        case TokenType::MY_LPAREN:  // 函数调用
            return [this](auto left){return parseCallExpression(std::move(left));} ;
        case TokenType::MY_LBRACK:  // 索引访问
            return [this](auto left){return parseIndexExpression(std::move(left));};
        case TokenType::MY_DOT:
            return [this](auto left){return parseMemberAccessExpression(std::move(left)); };
        default:
            return nullptr;
    }
}
