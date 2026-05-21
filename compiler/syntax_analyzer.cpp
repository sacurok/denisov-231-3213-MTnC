#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include "common.cpp"

using namespace std;

// Типы токенов
//enum TokenType {
//    KEYWORD, IDENTIFIER, CONSTANT_INT, CONSTANT_FLOAT,
//    CONSTANT_STRING, CONSTANT_CHAR, OPERATOR, DELIMITER,
//    PREPROCESSOR, TOK_EOF, UNKNOWN
//};

TokenType parseTokenType(const string& s) {
    if (s == "KEYWORD") return KEYWORD;
    if (s == "IDENTIFIER") return IDENTIFIER;
    if (s == "CONSTANT_INT") return CONSTANT_INT;
    if (s == "CONSTANT_FLOAT") return CONSTANT_FLOAT;
    if (s == "CONSTANT_STRING") return CONSTANT_STRING;
    if (s == "CONSTANT_CHAR") return CONSTANT_CHAR;
    if (s == "OPERATOR") return OPERATOR;
    if (s == "DELIMITER") return DELIMITER;
    if (s == "PREPROCESSOR") return PREPROCESSOR;
    return UNKNOWN;
}

//struct Token {
//    TokenType type;
//    string value;
//    int line;
//};

// Загрузка лексем из файла
vector<Token> loadTokens(const string& filename) {
    vector<Token> tokens;

    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "Ошибка: не удалось открыть файл " << filename << "\n";
        return tokens;
    }

    // Читаем весь файл в строку
    string content((istreambuf_iterator<char>(f)), {});
    f.close();

    size_t i = 0;
    size_t n = content.size();

    while (i < n) {
        // Ищем начало пары '('
        while (i < n && content[i] != '(') i++;
        if (i >= n) break;
        i++; // пропускаем '('

        // Чтение типа до первой ', '
        size_t typeStart = i;
        while (i < n && !(content[i] == ',' && i + 1 < n && content[i + 1] == ' ')) i++;
        string typeName = content.substr(typeStart, i - typeStart);
        i += 2; // пропускаем ", "

        // Чтение значения
        string value;

        if (i < n && content[i] == '"') {
            size_t valStart = i;
            i++; // пропускаем открывающую кавычку

            while (i < n) {
                // Экранированный символ
                if (content[i] == '\\' && i + 1 < n) {
                    i += 2;
                    continue;
                }
                // Конец строки
                if (content[i] == '"') {
                    i++;
                    break;
                }
                i++;
            }
            value = content.substr(valStart, i - valStart);
        }
        else {
            size_t valStart = i;
            while (i < n &&
                !(content[i] == ',' &&
                    i + 1 < n &&
                    content[i + 1] == ' '))
            {
                i++;
            }
            value = content.substr(valStart, i - valStart);
        }
        i += 2; // пропускаем ", "

        // Чтение номера строки
        size_t lineStart = i;
        while (i < n && content[i] != ')')
            i++;
        string lineStr = content.substr(lineStart, i - lineStart);
        int line = 0;
        try {
            line = stoi(lineStr);
        }
        catch (...) {
            line = 0;
        }
        // пропускаем ')'
        if (i < n)
            i++;

        Token tok;
        tok.type = parseTokenType(typeName);
        tok.value = value;
        tok.line = line;
        tokens.push_back(tok);
    }

    tokens.push_back({ TOK_EOF, "", 0 });
    return tokens;
}

//Узлы AST
enum class ASTKind {
    Program,
    Preprocessor,
    FuncDef,
    ParamList,
    Param,
    Block,
    VarDecl,
    AssignStmt,
    ReturnStmt,
    IfStmt,
    WhileStmt,
    ExprStmt,
    BinaryExpr,
    UnaryExpr,
    CallExpr,
    ArgList,
    Identifier,
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    TypeSpec,
};

string astKindName(ASTKind k) {
    switch (k) {
    case ASTKind::Program: return "Program";
    case ASTKind::Preprocessor: return "Preprocessor";
    case ASTKind::FuncDef: return "FuncDef";
    case ASTKind::ParamList: return "ParamList";
    case ASTKind::Param: return "Param";
    case ASTKind::Block: return "Block";
    case ASTKind::VarDecl: return "VarDecl";
    case ASTKind::AssignStmt: return "AssignStmt";
    case ASTKind::ReturnStmt: return "ReturnStmt";
    case ASTKind::IfStmt: return "IfStmt";
    case ASTKind::WhileStmt: return "WhileStmt";
    case ASTKind::ExprStmt: return "ExprStmt";
    case ASTKind::BinaryExpr: return "BinaryExpr";
    case ASTKind::UnaryExpr: return "UnaryExpr";
    case ASTKind::CallExpr: return "CallExpr";
    case ASTKind::ArgList: return "ArgList";
    case ASTKind::Identifier: return "Identifier";
    case ASTKind::IntLiteral: return "IntLiteral";
    case ASTKind::FloatLiteral: return "FloatLiteral";
    case ASTKind::StringLiteral: return "StringLiteral";
    case ASTKind::CharLiteral: return "CharLiteral";
    case ASTKind::TypeSpec: return "TypeSpec";
    default: return "?";
    }
}

struct ASTNode {
    ASTKind kind;
    string attr;
    vector<shared_ptr<ASTNode>> children;

    ASTNode(ASTKind k, string a = "") : kind(k), attr(move(a)) {}
    void addChild(shared_ptr<ASTNode> c) { if (c) children.push_back(c); }
};

using NodePtr = shared_ptr<ASTNode>;

NodePtr makeNode(ASTKind k, const string& a = "") {
    return make_shared<ASTNode>(k, a);
}

// Печать AST
void printAST(const NodePtr& node, const string& prefix = "", bool last = true) {
    if (!node) return;
    cout << prefix << (last ? "+-- " : "|-- ");
    cout << astKindName(node->kind);
    if (!node->attr.empty()) cout << ": " << node->attr;
    cout << "\n";

    string childPrefix = prefix + (last ? "    " : "|   ");
    for (size_t i = 0; i < node->children.size(); i++)
        printAST(node->children[i], childPrefix, i + 1 == node->children.size());
}

//Синтаксический анализатор
struct SynError {
    string message;
    string expected;
    string got;
    int line;
};

class Parser {
public:
    vector<SynError> errors;

    explicit Parser(vector<Token> toks) : tokens(move(toks)), pos(0) {}

    NodePtr parse() {
        auto root = makeNode(ASTKind::Program);
        while (!at(TOK_EOF)) {
            if (cur().type == PREPROCESSOR) {
                root->addChild(makeNode(ASTKind::Preprocessor, cur().value));
                advance();
            }
            else {
                auto decl = parseTopLevel();
                if (decl) {
                    root->addChild(decl);
                }
                else if (!at(TOK_EOF)) {
                    addError("Неожиданный токен на верхнем уровне",
                        "объявление или определение функции", cur().value, cur().line);
                    advance();
                }
            }
        }
        return root;
    }

private:
    vector<Token> tokens;
    size_t        pos;

    // Вспомогательные методы
    Token& cur() { return tokens[pos]; }

    bool at(TokenType t) { return cur().type == t; }
    bool atVal(const string& v) { return cur().value == v; }

    Token advance() {
        Token t = cur();
        if (pos + 1 < tokens.size()) pos++;
        return t;
    }

    bool expect(TokenType t, const string& val = "") {
        if (cur().type != t || (!val.empty() && cur().value != val)) {
            string exp = val.empty() ? tokenTypeStr(t) : ("'" + val + "'");
            addError("Ожидался токен", exp,
                "'" + cur().value + "' (" + tokenTypeStr(cur().type) + ")", cur().line);
            return false;
        }
        advance();
        return true;
    }

    void addError(const string& msg, const string& exp,
        const string& got, int line) {
        errors.push_back({ msg, exp, got, line });
    }

    string tokenTypeStr(TokenType t) {
        switch (t) {
        case KEYWORD: return "KEYWORD";
        case IDENTIFIER: return "IDENTIFIER";
        case CONSTANT_INT: return "CONSTANT_INT";
        case CONSTANT_FLOAT: return "CONSTANT_FLOAT";
        case CONSTANT_STRING: return "CONSTANT_STRING";
        case OPERATOR: return "OPERATOR";
        case DELIMITER: return "DELIMITER";
        case PREPROCESSOR: return "PREPROCESSOR";
        case TOK_EOF: return "EOF";
        default: return "UNKNOWN";
        }
    }

    bool isTypeKeyword(const string& v) {
        static const set<string> types = {
            "int","char","float","double","void",
            "long","short","unsigned","signed","const","static"
        };
        return types.count(v) > 0;
    }

    // Грамматические правила

    // Разбор глобального объявления или определения функции
    NodePtr parseTopLevel() {
        if (cur().type != KEYWORD || !isTypeKeyword(cur().value))
            return nullptr;

        // Тип (возможно составной: const int, unsigned long, …)
        string typeStr = advance().value;
        while (cur().type == KEYWORD && isTypeKeyword(cur().value))
            typeStr += " " + advance().value;

        if (cur().type != IDENTIFIER) {
            addError("Ожидался идентификатор после типа",
                "IDENTIFIER", "'" + cur().value + "'", cur().line);
            return nullptr;
        }
        string name = advance().value;

        if (atVal("("))
            return parseFuncDef(typeStr, name);

        return parseGlobalVarDecl(typeStr, name);
    }

    // Разбор определения функции
    NodePtr parseFuncDef(const string& type, const string& name) {
        auto node = makeNode(ASTKind::FuncDef, name);
        node->addChild(makeNode(ASTKind::TypeSpec, type));

        expect(DELIMITER, "(");
        node->addChild(parseParamList());
        expect(DELIMITER, ")");

        if (atVal("{")) {
            node->addChild(parseBlock());
        }
        else if (atVal(";")) {
            advance();
        }
        else {
            addError("Ожидалось тело функции или ';'",
                "'{' или ';'", cur().value, cur().line);
            syncToTopLevel();
        }

        return node;
    }

    // Разбор списка параметров функции
    NodePtr parseParamList() {
        auto node = makeNode(ASTKind::ParamList);
        if (atVal(")")) return node;
        if (atVal("void")) { advance(); return node; }

        node->addChild(parseParam());
        /*while (atVal(",")) {
            advance();
            node->addChild(parseParam());
        }*/
        while (!at(TOK_EOF) && !atVal(")")) {
            if (atVal(",")) {
                advance();
                node->addChild(parseParam());
                continue;
            }

            if (cur().type == KEYWORD && isTypeKeyword(cur().value)) {
                addError("Пропущена запятая между параметрами", "','",
                    "'" + cur().value + "'", cur().line);
                node->addChild(parseParam());
                continue;
            }

            addError("Некорректный список параметров","',' или ')'",
                "'" + cur().value + "'", cur().line);
            advance();
        }
        return node;
    }

    // Разбор одного параметра функции
    NodePtr parseParam() {
        auto node = makeNode(ASTKind::Param);
        if (cur().type != KEYWORD || !isTypeKeyword(cur().value)) {
            addError("Ожидался тип параметра", "ключевое слово типа",
                "'" + cur().value + "'", cur().line);
            return node;
        }
        string typeStr = advance().value;
        while (cur().type == KEYWORD && isTypeKeyword(cur().value))
            typeStr += " " + advance().value;
        node->addChild(makeNode(ASTKind::TypeSpec, typeStr));

        if (cur().type == IDENTIFIER) {
            node->attr = advance().value;
        }
        else {
            addError("Ожидался идентификатор параметра",
                "IDENTIFIER", "'" + cur().value + "'", cur().line);
        }
        return node;
    }

    // Разбор глобального объявления переменной
    NodePtr parseGlobalVarDecl(const string& type, const string& name) {
        auto node = makeNode(ASTKind::VarDecl, name);
        node->addChild(makeNode(ASTKind::TypeSpec, type));
        if (atVal("=")) { advance(); node->addChild(parseExpr()); }
        if (!expect(DELIMITER, ";")) syncToSemicolon();
        return node;
    }

    // Разбор блока кода в фигурных скобках
    NodePtr parseBlock() {
        auto node = makeNode(ASTKind::Block);
        int startLine = cur().line;
        if (!expect(DELIMITER, "{")) return node;

        while (!atVal("}") && !at(TOK_EOF)) {
            auto s = parseStatement();
            if (s) node->addChild(s);
        }

        if (atVal("}"))
            advance();
        else
            /*addError("Незакрытый блок: ожидался '}'", "}", cur().value, cur().line);*/
            addError("Незакрытый блок: ожидался '}'", "}", "EOF", startLine);

        return node;
    }

    // Разбор одного оператора
    NodePtr parseStatement() {
        if (cur().type == KEYWORD && isTypeKeyword(cur().value))
            return parseLocalVarDecl();
        if (atVal("return")) return parseReturnStmt();
        if (atVal("if")) return parseIfStmt();
        if (atVal("while")) return parseWhileStmt();
        if (atVal("{")) return parseBlock();
        if (atVal(";")) { advance(); return nullptr; }
        return parseExprStmt();
    }

    // Разбор локального объявления переменной'
    NodePtr parseLocalVarDecl() {
        string typeStr = advance().value;
        while (cur().type == KEYWORD && isTypeKeyword(cur().value))
            typeStr += " " + advance().value;

        if (cur().type != IDENTIFIER) {
            addError("Ожидался идентификатор в объявлении переменной",
                "IDENTIFIER", "'" + cur().value + "'", cur().line);
            syncToSemicolon();
            return nullptr;
        }
        string name = advance().value;
        auto node = makeNode(ASTKind::VarDecl, name);
        node->addChild(makeNode(ASTKind::TypeSpec, typeStr));

        if (atVal("=")) { advance(); node->addChild(parseExpr()); }
        if (!expect(DELIMITER, ";")) syncToSemicolon();
        return node;
    }

    // Разбор возврата значения функции return
    NodePtr parseReturnStmt() {
        advance(); // 'return'
        auto node = makeNode(ASTKind::ReturnStmt);
        if (!atVal(";")) node->addChild(parseExpr());
        if (!expect(DELIMITER, ";")) syncToSemicolon();
        return node;
    }

    // Разбор ветвления if else
    NodePtr parseIfStmt() {
        advance(); // 'if'
        auto node = makeNode(ASTKind::IfStmt);
        expect(DELIMITER, "(");
        node->addChild(parseExpr());
        expect(DELIMITER, ")");

       /* while (!atVal("{") && !at(TOK_EOF)) advance();
        auto thenBlock = parseBlock();
        node->addChild(thenBlock);

        while (!atVal("else") && !at(TOK_EOF)) advance();

        if (atVal("else")) {
            advance();
            while (!atVal("{") && !at(TOK_EOF)) advance();
            auto elseBlock = parseBlock();
            node->addChild(elseBlock);
        }*/

        // then block
        if (atVal("{")) {
            node->addChild(parseBlock());
        }
        else {
            addError(
                "Ожидалось начало блока после if",
                "'{'",
                "'" + cur().value + "'",
                cur().line
            );
            syncToTopLevel();
            return node;
        }

        // else block
        if (atVal("else")) {
            advance();

            if (atVal("{")) {
                node->addChild(parseBlock());
            }
            else {
                addError("Ожидалось начало блока после else", "'{'", "'" + cur().value + "'", cur().line);
                syncToTopLevel();
            }
        }
        return node;
    }

    // Разбор цикла while
    NodePtr parseWhileStmt() {
        advance(); // 'while'
        auto node = makeNode(ASTKind::WhileStmt);
        expect(DELIMITER, "(");
        node->addChild(parseExpr());
        expect(DELIMITER, ")");

        while (!atVal("{") && !at(TOK_EOF)) advance();
        node->addChild(parseBlock());

        return node;
    }

    // Разбор выражения как отдельного оператора
    NodePtr parseExprStmt() {
        /*int ln = cur().line;*/
        if (atVal("=")) {
            addError( "Пропущена левая часть присваивания", "IDENTIFIER",
                "'='", cur().line);
            advance();
            parseExpr();
            expect(DELIMITER, ";");
            return nullptr;
        }

        auto expr = parseExpr();
        if (!expect(DELIMITER, ";")) syncToSemicolon();

        if (expr && expr->kind == ASTKind::BinaryExpr && expr->attr == "=") {
            auto node = makeNode(ASTKind::AssignStmt);
            for (auto& c : expr->children) node->addChild(c);
            return node;
        }
        auto node = makeNode(ASTKind::ExprStmt);
        node->addChild(expr);
        return node;
    }

    // Иерархия выражений

    NodePtr parseExpr() { return parseAssign(); }

    NodePtr parseAssign() {
        auto left = parseLogicalOr();
        if (cur().type == OPERATOR) {
            const string& op = cur().value;
            if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=") {
                string o = advance().value;
                auto right = parseAssign();
                auto node = makeNode(ASTKind::BinaryExpr, o);
                node->addChild(left);
                node->addChild(right);
                return node;
            }
        }
        return left;
    }

    NodePtr parseLogicalOr() {
        auto left = parseLogicalAnd();
        while (atVal("||")) {
            string op = advance().value;
            auto right = parseLogicalAnd();
            auto node = makeNode(ASTKind::BinaryExpr, op);
            node->addChild(left); node->addChild(right); left = node;
        }
        return left;
    }

    NodePtr parseLogicalAnd() {
        auto left = parseEquality();
        while (atVal("&&")) {
            string op = advance().value;
            auto right = parseEquality();
            auto node = makeNode(ASTKind::BinaryExpr, op);
            node->addChild(left); node->addChild(right); left = node;
        }
        return left;
    }

    NodePtr parseEquality() {
        auto left = parseRelational();
        while (atVal("==") || atVal("!=")) {
            string op = advance().value;
            auto right = parseRelational();
            auto node = makeNode(ASTKind::BinaryExpr, op);
            node->addChild(left); node->addChild(right); left = node;
        }
        return left;
    }

    NodePtr parseRelational() {
        auto left = parseAdditive();
        if (!left) return nullptr;
        /*while (atVal("<") || atVal(">") || atVal("<=") || atVal(">=")) {
            string op = advance().value;
            auto right = parseAdditive();
            auto node = makeNode(ASTKind::BinaryExpr, op);
            node->addChild(left); 
            node->addChild(right); 
            left = node;
        }*/

        while (true) {
            if (atVal("<") || atVal(">") || atVal("<=") || atVal(">=")) {
                string op = advance().value;
                auto right = parseAdditive();
                auto node = makeNode(ASTKind::BinaryExpr, op);
                node->addChild(left);
                node->addChild(right);
                left = node;
                continue;
            }

            if (cur().type == IDENTIFIER ||
                cur().type == CONSTANT_INT ||
                cur().type == CONSTANT_FLOAT ||
                cur().type == CONSTANT_CHAR ||
                cur().type == CONSTANT_STRING)
            {
                addError("Пропущен оператор сравнения", "оператор сравнения",
                    "'" + cur().value + "'", cur().line);
                auto right = parseAdditive();
                auto node = makeNode(ASTKind::BinaryExpr, "<missing_op>");
                node->addChild(left);
                node->addChild(right);
                left = node;
                continue;
            }
            break;
        }
        return left;
    }

    NodePtr parseAdditive() {
        auto left = parseMultiplicative();
        while (atVal("+") || atVal("-")) {
            string op = advance().value;
            auto right = parseMultiplicative();
            auto node = makeNode(ASTKind::BinaryExpr, op);
            node->addChild(left); node->addChild(right); left = node;
        }
        return left;
    }

    NodePtr parseMultiplicative() {
        auto left = parseUnary();
        while (atVal("*") || atVal("/") || atVal("%")) {
            string op = advance().value;
            auto right = parseUnary();
            auto node = makeNode(ASTKind::BinaryExpr, op);
            node->addChild(left); node->addChild(right); left = node;
        }
        return left;
    }

    NodePtr parseUnary() {
        if (cur().type == OPERATOR) {
            const string& op = cur().value;
            if (op == "+" || op == "-" || op == "!" || op == "~" || op == "*" || op == "&" || op == "++" || op == "--") {
                string o = advance().value;
                auto node = makeNode(ASTKind::UnaryExpr, o);
                node->addChild(parseUnary());
                return node;
            }
        }
        return parsePostfix();
    }

    NodePtr parsePostfix() {
        auto expr = parsePrimary();
        while (true) {
            if (atVal("(")) {
                advance();
                auto call = makeNode(ASTKind::CallExpr, expr ? expr->attr : "?");
                call->addChild(expr);
                auto args = makeNode(ASTKind::ArgList);
                if (!atVal(")")) {
                    args->addChild(parseExpr());
                    while (atVal(",")) { advance(); args->addChild(parseExpr()); }
                }
                call->addChild(args);
                expect(DELIMITER, ")");
                expr = call;
            }
            else if (atVal("[")) {
                advance();
                auto node = makeNode(ASTKind::BinaryExpr, "[]");
                node->addChild(expr);
                node->addChild(parseExpr());
                expect(DELIMITER, "]");
                expr = node;
            }
            else if (atVal("++") || atVal("--")) {
                string op = advance().value;
                auto node = makeNode(ASTKind::UnaryExpr, "post" + op);
                node->addChild(expr);
                expr = node;
            }
            else break;
        }
        return expr;
    }

    NodePtr parsePrimary() {
        if (cur().type == IDENTIFIER) return makeNode(ASTKind::Identifier, advance().value);
        if (cur().type == CONSTANT_INT) return makeNode(ASTKind::IntLiteral, advance().value);
        if (cur().type == CONSTANT_FLOAT) return makeNode(ASTKind::FloatLiteral, advance().value);
        if (cur().type == CONSTANT_STRING) return makeNode(ASTKind::StringLiteral, advance().value);
        if (cur().type == CONSTANT_CHAR) return makeNode(ASTKind::CharLiteral, advance().value);
        if (atVal("(")) {
            advance();
            auto expr = parseExpr();
            expect(DELIMITER, ")");
            return expr;
        }
        /*if (!at(TOK_EOF)) {
            addError("Ожидалось выражение", "выражение",
                "'" + cur().value + "'", cur().line);
            advance();
        }*/

        if (!at(TOK_EOF)) {
            addError("Ожидалось выражение", "выражение",
                "'" + cur().value + "'", cur().line);

            if (!atVal(";") && !atVal(")") && !atVal("}") 
                && !atVal("]") && !atVal(",")) {
                advance();
            }
        }

        return nullptr;
    }

    bool isStatementStart() {
        if (cur().type == KEYWORD) {
            return cur().value == "if" ||
                cur().value == "while" ||
                cur().value == "return" ||
                isTypeKeyword(cur().value) ||
                cur().value == "else";
        }

        return atVal("{") || atVal("}");
    }

    void syncToSemicolon() {
        while (!at(TOK_EOF)) {

            if (atVal(";")) {
                advance();
                return;
            }

            if (isStatementStart())
                return;
            advance();
        }
    }

    //void syncToTopLevel() {
    //    while (!at(TOK_EOF)) {

    //        // начало новой функции/глобального объявления
    //        if (cur().type == KEYWORD &&
    //            isTypeKeyword(cur().value))
    //        {
    //            return;
    //        }

    //        advance();
    //    }
    //}

    void syncToTopLevel() {
        while (!at(TOK_EOF)) {

            if (cur().type == KEYWORD &&
                isTypeKeyword(cur().value))
            {
                size_t save = pos;

                advance();

                if (cur().type == IDENTIFIER) {
                    advance();

                    if (atVal("(")) {
                        pos = save;
                        return;
                    }
                }

                pos = save;
            }

            advance();
        }
    }

    /*void syncToSemicolon() {
        while (!atVal(";") && !at(TOK_EOF)) advance();
        if (atVal(";")) advance();
    }*/
};

//int runParser() {
//    setlocale(LC_ALL, "Russian");
//
//    vector<Token> tokens = loadTokens("lexems.txt");
//    if (tokens.empty() || (tokens.size() == 1 && tokens[0].type == TOK_EOF)) {
//        cerr << "Ошибка: файл lexems.txt пуст или не найден.\n";
//        return 1;
//    }
//
//    // Синтаксический анализ
//    Parser parser(tokens);
//    NodePtr ast = parser.parse();
//
//    // Вывод AST
//    cout << "Абстрактное синтаксическое дерево (AST)\n";
//    printAST(ast, "", true);
//
//    // Вывод ошибок
//    if (!parser.errors.empty()) {
//        cout << "\nСинтаксические ошибки\n";
//        for (auto& e : parser.errors) {
//            cout << "Строка " << e.line << ": [" << e.message << "]";
//            if (!e.expected.empty()) cout << " — ожидалось: " << e.expected;
//            /*if (!e.got.empty())      cout << ", получено: " << e.got;*/
//            cout << "\n";
//        }
//    }
//
//    if (parser.errors.empty())
//        cout << "Синтаксический анализ завершён успешно. Ошибок не найдено.\n";
//    else
//        cout << "Синтаксический анализ завершён с ошибками: "
//        << parser.errors.size() << "\n";
//
//    return parser.errors.empty() ? 0 : 1;
//}

NodePtr runParser() {
    setlocale(LC_ALL, "Russian");

    vector<Token> tokens = loadTokens("lexems.txt");
    if (tokens.empty() || (tokens.size() == 1 && tokens[0].type == TOK_EOF)) {
        cerr << "Ошибка: файл lexems.txt пуст или не найден.\n";
        return nullptr;
    }

    // Синтаксический анализ
    Parser parser(tokens);
    NodePtr ast = parser.parse();

    // Вывод AST
    cout << "Абстрактное синтаксическое дерево (AST)\n";
    printAST(ast, "", true);

    // Вывод ошибок
    if (!parser.errors.empty()) {
        cout << "\nСинтаксические ошибки\n";
        for (auto& e : parser.errors) {
            cout << "Строка " << e.line << ": [" << e.message << "]";
            if (!e.expected.empty()) cout << " — ожидалось: " << e.expected;
            /*if (!e.got.empty())      cout << ", получено: " << e.got;*/
            cout << "\n";
        }
    }

    if (parser.errors.empty())
        cout << "Синтаксический анализ завершён успешно. Ошибок не найдено.\n";
    else
        cout << "Синтаксический анализ завершён с ошибками: "
        << parser.errors.size() << "\n";

    return ast;
}