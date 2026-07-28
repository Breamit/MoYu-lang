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
        case TokenType::ES_ASSIGN:
            return Precedence::ASSIGN;
        case TokenType::ES_OR:
            return Precedence::OR;
        case TokenType::ES_BIT_CARET:
            return Precedence::XOR;
        case TokenType::ES_AND:
            return Precedence::AND;
        case TokenType::ES_EQUAL:
        case TokenType::ES_NOT_EQUAL:
            return Precedence::EQUALS;
        case TokenType::ES_GT:
        case TokenType::ES_LT:
        case TokenType::ES_EQGT:
        case TokenType::ES_EQLT:
            return Precedence::LESSGREATER;
        case TokenType::ES_PLUS:
        case TokenType::ES_MINUS:
            return Precedence::SUM;
        case TokenType::ES_MULTI:
        case TokenType::ES_DIV:
        case TokenType::ES_MOD:
            return Precedence::PRODUCT;
        case TokenType::ES_LPAREN:
        case TokenType::ES_DOT:
        case TokenType::ES_LBRACK:
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

// =================================================================================================================

UPtr<Program> Parser::parseProgram() {
    // 只进行收集
    // 创建程序对象
    auto program = std::make_unique<Program>();
    while(curToken_.getTokenType() != TokenType::ES_EOF) {
        auto stmt = parseStatement();   // 交给解析函数执行
        if(stmt != nullptr) {
            program->statements.push_back(std::move(stmt));
        } else {
            nextToken();
        }
    }
    return program;
}

UPtr<Statement> Parser::parseStatement() {
    // 分发函数
    switch(curToken_.getTokenType()) {
        case TokenType::ES_LET:
            // 解析let statement
            return parseLetStatement();
        case TokenType::ES_CONST:
            // 解析const statement
            return parseConstStatement();
        case TokenType::ES_RETURN:
            // 解析 return 语句
            return parseReturnStatement();
        case TokenType::ES_CONTINUE:
            // 解析 continue 语句
            return parseContinueStatement();
        case TokenType::ES_BREAK:
            // 解析 break 语句
            return parseBreakStatement();
        case TokenType::ES_IF:
            // 解析 if 语句
            return parseIfStatement();
        case TokenType::ES_SWITCH:
            // 解析 switch 语句
            return parseSwitchStatement();
        case TokenType::ES_FOR:
            // 解析 for 语句
            return parseForStatement();
        case TokenType::ES_WHILE:
            // 解析 while 语句
            return parseWhileStatement();
        case TokenType::ES_FUNCTION:
            // 解析函数声明
            return parseFunctionDeclareStatement();
        case TokenType::ES_STRUCT:
            // 解析结构体声明
            return parseStructDeclareStatement();
        case TokenType::ES_ENUM:
            // 枚举声明
            return parseEnumDeclareStatement();
        case TokenType::ES_IMPORT:
            // 解析导入语句声明
            return parseImportStatement();
        case TokenType::ES_EXPORT:
            // 导出语句声明
            return parseExportStatement();
        case TokenType::ES_AT:
            // 解析注解
            return parseAnnotation();
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
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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
        ErrorInfo ei;
        ei.msg = "期望一个类型名，但遇到 '" + curToken_.getLiteral() + "'";
        auto pos = curToken_.getPosition();
        ei.col = pos.first;
        ei.line = pos.second;
        errors_.push_back(ei);
        return nullptr;
    }

    auto typel = std::make_unique<TypeLiteral>();
    typel->token = curToken_;
    typel->name = curToken_.getLiteral();
    nextToken();    // 消耗类型名

    // 泛型参数：list<i32>、map<string, i32>
    if (curToken_.getTokenType() == TokenType::ES_LT) {
        nextToken();    // 消耗 <
        while (curToken_.getTokenType() != TokenType::ES_GT) {
            auto param = parseTypeLiteral();
            if (param) {
                typel->typeParams.push_back(std::move(param));
            } else {
                // 参数解析失败
                return nullptr;
            }
            if (curToken_.getTokenType() == TokenType::ES_COMMA) {
                nextToken();    // 消耗 ,
            } else {
                break;
            }
        }
        // 现在应该是 >
        if (curToken_.getTokenType() != TokenType::ES_GT) {
            ErrorInfo ei;
            ei.msg = "类型参数缺少关闭的 >";
            auto pos = curToken_.getPosition();
            ei.col = pos.first;
            ei.line = pos.second;
            errors_.push_back(ei);
            return nullptr;
        }
        nextToken();    // 消耗 >
    }

    return typel;
}

UPtr<Expression> Parser::parsePrefixExpression() {
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

    if(!curTokenIs(TokenType::ES_RPAREN)) {
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
    while(!curTokenIs(TokenType::ES_RBRACE) && !curTokenIs(TokenType::ES_EOF)) {
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
    if(!curTokenIs(TokenType::ES_IDENT)) {
        // 不是标识符，就代表无参，不消耗
        return params;
    } 
    while(true) {
        auto name = parseIdentifier();
        if(!name) {
            return nullptr; // 解析存在错误
        }

        // 当前 token指向 :
        if(!curTokenIs(TokenType::ES_COLON)) {
            return nullptr;
        }

        nextToken(); // 消耗 :
        auto type = parseTypeLiteral();
        if(!type) {
            return nullptr;
        }

        params->push_back({std::move(name),std::move(type)});

        // 更多参数
        if(curTokenIs(TokenType::ES_COMMA)) {
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
    // 标准的let语句： let <标识符> : <类型> = <表达式>
    auto stmt = std::make_unique<LetStatement>();
    stmt->token = curToken_;    // 保存 let
    if(!expectPeek(TokenType::ES_IDENT)) {
        ErrorInfo ei;
        // 未识别到标识符
        ei.msg = "syntax invalid: identifier not found";
        auto pos = peekToken_.getPosition();
        ei.col = pos.first;
        ei.line = pos.second;
        errors_.push_back(ei);
        return nullptr;
    }
    // 这个时候 curToken 指向标识符
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();
    // name就是ident
    stmt->name = std::move(ident);

    // 下一个token 应该是 : 
    if(!expectPeek(TokenType::ES_COLON)) {
        return nullptr;
    }

    // 下一个token 应该是类型标注，也就是类型标识
    // i32 , list<i32> , list<list<i32>>
    // 消耗 :，前进到类型名起始token
    nextToken();
    stmt->varType = parseTypeLiteral();

    // 现在的token就是 =
    if(!curTokenIs(TokenType::ES_ASSIGN)) {
        return nullptr;
    }    
    nextToken();    // 消耗掉 =

    stmt->value = parseExpression(Precedence::LOWEST);

    // 现在指向 ;
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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
    if(!expectPeek(TokenType::ES_IDENT)) {
        ErrorInfo ei;
        // 未识别到标识符
        ei.msg = "syntax invalid: identifier not found";
        auto pos = peekToken_.getPosition();
        ei.col = pos.first;
        ei.line = pos.second;
        errors_.push_back(ei);
        return nullptr;
    }
    // 这个时候 curToken 指向标识符
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();
    // name就是ident
    stmt->name = std::move(ident);

    // 下一个token 应该是 : 
    if(!expectPeek(TokenType::ES_COLON)) {
        return nullptr;
    }

    // 下一个token 应该是类型标注，也就是类型标识
    // i32 , list<i32> , list<list<i32>>
    // 消耗 :，前进到类型名起始token
    nextToken();
    stmt->varType = parseTypeLiteral();

    // 现在的token就是 =
    if(!curTokenIs(TokenType::ES_ASSIGN)) {
        return nullptr;
    }    
    nextToken();    // 消耗掉 =

    stmt->value = parseExpression(Precedence::LOWEST);

    // 现在指向 ;
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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

    if(!stmt) {
        return nullptr;
    }

    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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
    if(!curTokenIs(TokenType::ES_LPAREN)) {
        return nullptr;
    }
    nextToken();

    auto cond = parseExpression(Precedence::LOWEST);
    if(!cond) {
        return nullptr;
    }
    stmt->condition = std::move(cond);
    // 现在应该是右括号
    if(!curTokenIs(TokenType::ES_RPAREN)) {
        return nullptr;
    }
    nextToken();    //消耗)
    
    auto fs = parseBlockStatement();
    if(!fs) {
        return nullptr;
    }
    stmt->consequence = std::move(fs);

    // 现在指向else elif 或者下一个语句

    // 解析 elif
    while(curTokenIs(TokenType::ES_ELIF)) {
        // 现在的token是 elif
        nextToken();
        // 现在的token是 (
        if(!curTokenIs(TokenType::ES_LPAREN)) {
            return nullptr;
        }
        nextToken();
        // 现在的token是expression
        auto expr = parseExpression(Precedence::LOWEST);
        if(!expr) {
            return nullptr;
        }
        // 现在的token是 )
        if(!curTokenIs(TokenType::ES_RPAREN)) {
            return nullptr;
        }
        nextToken();
        //现在的token是{
        auto bl = parseBlockStatement();
        if(!bl) {
            return nullptr;
        }
        stmt->elifClauses.push_back({std::move(expr),std::move(bl)});

    }


    // 解析 else
    if(curTokenIs(TokenType::ES_ELSE)) {
        nextToken();    //消费else

        auto bl = parseBlockStatement();
        if(!bl) {
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
    if(!curTokenIs(TokenType::ES_LPAREN)) {
        return nullptr;
    }
    nextToken();
    auto expr = parseExpression(Precedence::LOWEST);

    if(!expr) {
        return nullptr;
    }

    // 现在应该是 )
    if(!curTokenIs(TokenType::ES_RPAREN)) {
        return nullptr;
    }
    nextToken();
    
    // switch语句的{
    if(!curTokenIs(TokenType::ES_LBRACE)) {
        return nullptr;
    }
    nextToken();
    // 现在应该是第一个case
    // case 流
    while(curTokenIs(TokenType::ES_CASE)) {
        // 当前是 case
        nextToken();
        
        //现在是表达式
        auto subExpr = parseExpression(Precedence::LOWEST);
        if(!subExpr) {
            return nullptr;
        }

        if(!curTokenIs(TokenType::ES_COLON)) {
            return nullptr;
        }

        nextToken();

        // 现在是{ 或者下一个case , 需要支持case 穿透
        if(curTokenIs(TokenType::ES_CASE)) {
            stmt->cases.push_back({std::move(subExpr),nullptr});  //   没有block
        } else if(curTokenIs(TokenType::ES_LBRACE)) {
            auto block = parseBlockStatement();
            if(!block) {
                return nullptr;
            }
            stmt->cases.push_back({std::move(subExpr),std::move(block)});
        } else if(curTokenIs(TokenType::ES_RBRACE)) {
            break;
        } else {
            return nullptr;
        }
    }

    // default流
    if(curTokenIs(TokenType::ES_DEFAULT)) {
        nextToken();
        //现在应该是 :
        if(!curTokenIs(TokenType::ES_COLON)) {
            return nullptr;
        }
        nextToken();

        // 现在应该是 {
        if(!curTokenIs(TokenType::ES_LBRACE)) {
            return nullptr;
        }
        auto block = parseBlockStatement();

        if(!block) {
            return nullptr;
        }

        stmt->defaultCase = std::move(block);

    } 

    if(!curTokenIs(TokenType::ES_RBRACE)) {
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
    if(!curTokenIs(TokenType::ES_LPAREN)) {
        return nullptr;
    }
    nextToken();

    // ==================== 解析 init 部分 ====================
    // 现在是 let const ident 开头或者 ;

    if(curTokenIs(TokenType::ES_SEMICOLON)) {
        // 无 init
        nextToken();
    } else if(curTokenIs(TokenType::ES_LET) || curTokenIs(TokenType::ES_CONST)) {
        bool isConst = curTokenIs(TokenType::ES_CONST);
        nextToken();

        if(!curTokenIs(TokenType::ES_IDENT)) {
            return nullptr;
        }

        auto varIdent = std::make_unique<Identifier>();
        varIdent->token = curToken_;
        varIdent->value = curToken_.getLiteral();
        nextToken();    // 跳过变量名

        // 关键分支：看 curToken 是 = (C风格) 还是 in (for-in)
        if(curTokenIs(TokenType::ES_IN)) {
            // ==================== for-in ====================
            nextToken();    // 跳过 in
            auto iterable = parseExpression(Precedence::LOWEST);
            if(!iterable) return nullptr;

            if(!curTokenIs(TokenType::ES_RPAREN)) return nullptr;
            nextToken();    // 跳过 )

            auto forBody = parseBlockStatement();
            if(!forBody) return nullptr;

            // ---- 翻译 for(let i in iterable){body} 为 C 风格 ----
            // 构造唯一临时变量名
            static int forTempCounter = 0;
            String idxName = "$idx";
            Token idxToken;
            idxToken.setTL(TokenType::ES_IDENT, idxName);

            // 1. init: let __fo_0 = 0;
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

            // 2. condition: __fo_0 < len(iterable)
            auto condLeft = std::make_unique<Identifier>();
            condLeft->token = idxToken;
            condLeft->value = idxName;

            // len(iterable) — 构造 CallExpression
            Token lenToken;
            lenToken.setTL(TokenType::ES_IDENT, "len");
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
            varToken.setTL(TokenType::ES_IDENT, varIdent->value);
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

        } else if(curTokenIs(TokenType::ES_ASSIGN)) {
            // ==================== C风格 for(let x = expr; ...) ====================
            nextToken();    // 跳过 =
            auto initVal = parseExpression(Precedence::LOWEST);
            if(!initVal) return nullptr;

            if(!curTokenIs(TokenType::ES_SEMICOLON)) return nullptr;
            nextToken();

            // 手动构造 init 语句（不强制类型标注）
            if(isConst) {
                auto constStmt = std::make_unique<ConstStatement>();
                constStmt->token = varIdent->token;
                constStmt->name = std::move(varIdent);
                constStmt->value = std::move(initVal);
                stmt->init = std::move(constStmt);
            } else {
                auto letStmt = std::make_unique<LetStatement>();
                letStmt->token = varIdent->token;
                letStmt->name = std::move(varIdent);
                letStmt->value = std::move(initVal);
                stmt->init = std::move(letStmt);
            }

            // condition
            stmt->condition = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::ES_SEMICOLON)) return nullptr;
            nextToken();

            // update
            stmt->update = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::ES_RPAREN)) return nullptr;
            nextToken();

            stmt->body = parseBlockStatement();
            if(!stmt->body) return nullptr;
            return stmt;
        } else {
            return nullptr;
        }
    } else {
        // C 风格: for(expr; condition; update) — 无 let/const
        auto initExpr = parseExpression(Precedence::LOWEST);
        if(!initExpr) return nullptr;

        auto exprStmt = std::make_unique<ExpressionStatement>();
        exprStmt->token = curToken_;
        exprStmt->expression = std::move(initExpr);
        stmt->init = std::move(exprStmt);

        if(!curTokenIs(TokenType::ES_SEMICOLON)) return nullptr;
        nextToken();

        // condition
        if(!curTokenIs(TokenType::ES_SEMICOLON)) {
            stmt->condition = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::ES_SEMICOLON)) return nullptr;
        }
        nextToken();

        // update
        if(!curTokenIs(TokenType::ES_RPAREN)) {
            stmt->update = parseExpression(Precedence::LOWEST);
            if(!curTokenIs(TokenType::ES_RPAREN)) return nullptr;
        }
        nextToken();

        stmt->body = parseBlockStatement();
        if(!stmt->body) return nullptr;
        return stmt;
    }

    // ==================== 解析 condition 和 update ====================
    // 走到这里说明 init 是空（已经处理了 ;）
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
        stmt->condition = parseExpression(Precedence::LOWEST);
    }
    if(!curTokenIs(TokenType::ES_SEMICOLON)) return nullptr;
    nextToken();

    if(!curTokenIs(TokenType::ES_RPAREN)) {
        stmt->update = parseExpression(Precedence::LOWEST);
    }
    if(!curTokenIs(TokenType::ES_RPAREN)) return nullptr;
    nextToken();

    stmt->body = parseBlockStatement();
    if(!stmt->body) return nullptr;
    return stmt;
}

UPtr<WhileStatement> Parser::parseWhileStatement() {
    auto stmt = std::make_unique<WhileStatement>();
    // 现在是 while 这个 token
    stmt->token = curToken_;
    nextToken();    // 消费 while

    if(!curTokenIs(TokenType::ES_LPAREN)) {
        return nullptr;
    }
    nextToken();    // 消费 (
    auto expr = parseExpression(Precedence::LOWEST);

    if(!expr) {
        return nullptr;
    }

    stmt->condition = std::move(expr);

    if(!curTokenIs(TokenType::ES_RPAREN)) {
        return nullptr;
    }

    nextToken();

    if(!curTokenIs(TokenType::ES_LBRACE)) {
        return nullptr;
    }
    auto block = parseBlockStatement();
    if(!block) {
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
    if(!curTokenIs(TokenType::ES_IDENT)) {
        return nullptr;
    }
    auto ident = std::make_unique<Identifier>();
    ident->token = curToken_;
    ident->value = curToken_.getLiteral();

    fd->name = std::move(ident);
    nextToken();

    // 当前是括号
    if(!curTokenIs(TokenType::ES_LPAREN)) {
        // 当前不是左括号
        return nullptr;
    }
    nextToken();
    auto paramList = parseParamsList();

    if(paramList == nullptr) {
        return nullptr;
    }
    fd->params = std::move(*paramList);

    // 当前是右括号
    if(!curTokenIs(TokenType::ES_RPAREN)) {
        return nullptr;
    }
    nextToken();
    
    // 现在是 :
    if(!curTokenIs(TokenType::ES_COLON)) {
        // 没有类型标注
        return nullptr;
    }
    nextToken();

    auto rt = parseTypeLiteral();
    if(!rt) {
        return nullptr;
    }
    fd->returnType = std::move(rt);

    // 现在指向的是 { 或者 ;
    if(curTokenIs(TokenType::ES_SEMICOLON)) {
        return fd;
    } else if(curTokenIs(TokenType::ES_LBRACE)) {
        // 解析 block语句
        auto bd = parseBlockStatement();
        if(bd == nullptr) {
            return nullptr;
        }
        fd->body = std::move(bd);
        return fd;
        
    } else {
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
    if(!curTokenIs(TokenType::ES_IDENT)) {
        return nullptr;
    }
    auto ident = parseIdentifier();     // 标识符被消费

    if(!ident) {
        return nullptr;
    }

    decl->name = UPtr<Identifier>(static_cast<Identifier*>(ident.release()));
    
    // 当前应该是 {
    if(!curTokenIs(TokenType::ES_LBRACE)) {
        return nullptr;
    }
    nextToken(); // 消费 {
    auto list = parseParamsList();  // 用来解析函数参数列表的也可以用来解析结构体域
    if(!list) {
        return nullptr;
    }
    decl->fields = std::move(*list);
    // 当前的token是}
    if(!curTokenIs(TokenType::ES_RBRACE)) {
        return nullptr;
    }
    nextToken();


    // 当前应该是 ;
    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
        return nullptr;
    }
    nextToken();

    return decl;
}

UPtr<EnumDeclareStatement> Parser::parseEnumDeclareStatement() {
    // enum Color { white, black, red };
    auto decl = std::make_unique<EnumDeclareStatement>();
    decl->token = curToken_;    // enum
    nextToken();                // 跳过 enum

    if(!curTokenIs(TokenType::ES_IDENT)) {
        return nullptr;
    }
    auto nameIdent = std::make_unique<Identifier>();
    nameIdent->token = curToken_;
    nameIdent->value = curToken_.getLiteral();
    decl->name = std::move(nameIdent);
    nextToken();    // 跳过枚举名

    if(!curTokenIs(TokenType::ES_LBRACE)) {
        return nullptr;
    }
    nextToken();    // 跳过 {

    // 解析标识符列表
    while(curTokenIs(TokenType::ES_IDENT)) {
        auto ident = std::make_unique<Identifier>();
        ident->token = curToken_;
        ident->value = curToken_.getLiteral();
        decl->values.push_back(std::move(ident));
        nextToken();

        if(curTokenIs(TokenType::ES_COMMA)) {
            nextToken();    // 跳过 ,
        } else {
            break;
        }
    }

    if(!curTokenIs(TokenType::ES_RBRACE)) {
        return nullptr;
    }
    nextToken();    // 跳过 }

    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
        return nullptr;
    }
    nextToken();    // 跳过 ;

    return decl;
}

UPtr<ImportStatement> Parser::parseImportStatement() {
    // 当前 token 是 import
    auto stmt = std::make_unique<ImportStatement>();
    stmt->token = curToken_;

    nextToken();    // 消耗 import

    if(!curTokenIs(TokenType::ES_IDENT)) {
        return nullptr; 
    }
    auto name = parseIdentifier();
    stmt->name = UPtr<Identifier>(static_cast<Identifier*>(name.release()));

    // 当前token指向 ;

    if(!curTokenIs(TokenType::ES_SEMICOLON)) {
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
    if(curTokenIs(TokenType::ES_IDENT)) {
        auto ident = parseIdentifier();

        stmt->exports.push_back(UPtr<Identifier>(static_cast<Identifier*>(ident.release())));
        // 现在是 ;
        if(!curTokenIs(TokenType::ES_SEMICOLON)) {
            return nullptr;
        } 
        nextToken();
        return stmt;
    } else if(curTokenIs(TokenType::ES_LBRACE)) {
        nextToken();
        // 现在是标识符
        auto idnet1 = parseIdentifier();
        stmt->exports.push_back(UPtr<Identifier>(static_cast<Identifier*>(idnet1.release())));
        // 现在应该是 , ident
        while(true) {
            if(!curTokenIs(TokenType::ES_COMMA)) {
                break;
            }
            nextToken();
            if(!curTokenIs(TokenType::ES_IDENT)) {
                break;
            }
            auto ident = parseIdentifier();
            if(!ident) {
                return nullptr;
            }
            stmt->exports.push_back(UPtr<Identifier>(static_cast<Identifier*>(ident.release())));
        }
        if(!curTokenIs(TokenType::ES_RBRACE)) {
            return nullptr;
        }
        nextToken();
        if(!curTokenIs(TokenType::ES_SEMICOLON)) {
            return nullptr;
        }
        nextToken();
        return stmt;
        

    } else {
        return nullptr;
    }
}

UPtr<AnnotationStatement> Parser::parseAnnotation() {
    // 现在是 @
    Token tk = curToken_;
    nextToken();
    if(!curTokenIs(TokenType::ES_IDENT)) {
        return nullptr;
    }
    auto ident = parseIdentifier();
    UPtr<Annotation> anno = nullptr;

    // 根据ident决定anno的实现类
    static Map<String,i32> annoMap = {
        {"native",0},
        {"platform",1},
        {"method",2},
        {"object",3},
        {"async",4}
    };

    auto it = annoMap.find(ident->tokenLiteral());
    i32 id = -1;
    if(it != annoMap.end()) {
        id = it->second;
        switch(id) {
            case 0:
                anno = std::make_unique<NativeAnnotation>();
                break;
            case 1:
                anno =std::make_unique<PlatformAnnotation>();
                break;
            case 2:
                anno = std::make_unique<MethodAnnotation>();
                break;
            case 3:
                anno =std::make_unique<ObjectAnnotation>();
                break;
            case 4:
                anno = std::make_unique<AsyncAnnotation>();
                break;
            default:
                return nullptr;
        }
    } else {
        // 未注册注解
        return nullptr;
    }


    // 现在是左括号或者fn或者 struct
    if(curTokenIs(TokenType::ES_LPAREN)) {
        // 左括号只能是 native / platform / method
        if(id<0 || id >= 3) {
            return nullptr;
        }
        switch(id) {
            case 0:
            
        }
    }
}

Parser::PrefixFn Parser::prefixParseFn(TokenType type) {
    switch(type) {
        case TokenType::ES_MINUS:
        case TokenType::ES_PLUS:
            return [this]{return parsePrefixExpression(); };
        case TokenType::ES_IDENT:
            return [this]{return parseIdentifier(); };
        case TokenType::ES_INTEGER:
            return [this]{return parseIntegerLiteral(); };
        case TokenType::ES_LPAREN:
            return [this]{return parseGroupedExpression(); };
        case TokenType::ES_TRUE:
        case TokenType::ES_FALSE:
            return [this]{return parseBooleanLiteral(); };
        default:
            return nullptr;
    }
}

Parser::InfixFn Parser::infixParseFn(TokenType type) {
    // 1 + 1 , 1 * 1 ... LEFT OP RIGHT
    switch(type) {
        case TokenType::ES_PLUS:
        case TokenType::ES_MINUS:
        case TokenType::ES_MULTI:
        case TokenType::ES_DIV:
        case TokenType::ES_MOD:
        case TokenType::ES_BIT_AND:
        case TokenType::ES_BIT_OR:
        case TokenType::ES_BIT_CARET:
        case TokenType::ES_ASSIGN:
        case TokenType::ES_NOT_EQUAL:
        case TokenType::ES_EQUAL:
        case TokenType::ES_EQGT:
        case TokenType::ES_EQLT:
        case TokenType::ES_LT:
        case TokenType::ES_GT:
        case TokenType::ES_BIT_INVERSION:
            return [this](auto left){return parseInfixExpression(std::move(left));};
        default:
            return nullptr;
    }
}
