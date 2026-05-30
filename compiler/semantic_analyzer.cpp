#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <iomanip>

using namespace std;

struct Symbol {
    string name;
    string type;
    string scope;
    bool declared;
    bool initialized;
    string note;
};

struct SemanticError {
    string message;
    string details;
    int line;
};

struct Triad {
    int number;
    string op;
    string arg1;
    string arg2;
};

struct FunctionInfo {
    string returnType;
    vector<string> paramTypes;
};

class SemanticAnalyzer {
private:

    vector<Symbol> symbolTable;
    vector<SemanticError> errors;
    vector<Triad> triads;

    int triadCounter = 1;
    string currentScope = "global";
    string currentFunctionReturnType = "void";
    map<string, FunctionInfo> functions;

    int getNodeLine(NodePtr node) {
        if (!node) return 0;

        for (auto& child : node->children) {
            int line = getNodeLine(child);
            if (line != 0) return line;
        }
        return 0;
    }

    void collectFunctions(NodePtr node) {
        if (!node) return;

        if (node->kind == ASTKind::FuncDef) {
            string funcName = node->attr;
            string returnType = "void";

            if (!node->children.empty())
                returnType = node->children[0]->attr;

            FunctionInfo info;
            info.returnType = returnType;

            // Собираем типы параметров
            if (node->children.size() > 1) {
                auto params = node->children[1];
                if (params->kind == ASTKind::ParamList) {
                    for (auto& p : params->children) {
                        string paramType = "unknown";
                        if (!p->children.empty())
                            paramType = p->children[0]->attr;
                        info.paramTypes.push_back(paramType);
                    }
                }
            }

            if (!functions.count(funcName))
                functions[funcName] = info;
        }

        for (auto& child : node->children)
            collectFunctions(child);
    }

    string getExpressionType(NodePtr node) {
        if (!node) return "unknown";

        switch (node->kind) {

        case ASTKind::IntLiteral: return "int";
        case ASTKind::FloatLiteral: return "float";
        case ASTKind::StringLiteral: return "string";
        case ASTKind::CharLiteral: return "char";

        case ASTKind::Identifier: {
            Symbol* s = findSymbol(node->attr);
            if (!s) return "unknown";
            return s->type;
        }

        case ASTKind::BinaryExpr: {
            string left = getExpressionType(node->children[0]);
            string right = getExpressionType(node->children[1]);

            if (left != right) {
                /*errors.push_back({"Ошибка: несовместимые типы '" +
                    left + "' и '" + right + "' в выражении"});*/
                errors.push_back({
                    "Ошибка несовместимости типов",
                    "Тип '" + left + "' нельзя использовать вместе с типом '" + right + "'",
                    node->line
                });
            }

            return left;
        }

        case ASTKind::CallExpr: {
            string funcName = node->attr;
            if (functions.count(funcName)) return functions[funcName].returnType;
            return "unknown";
        }

        default:
            return "unknown";
        }
    }

public:
    void analyze(NodePtr root) {
        collectFunctions(root);
        visit(root);
    }

    const vector<Symbol>& getSymbolTable() const {
        return symbolTable;
    }
    const vector<SemanticError>& getErrors() const {
        return errors;
    }
    const vector<Triad>& getTriads() const {
        return triads;
    }

private:

    // Поиск символа
    Symbol* findSymbol(const string& name) {

        for (auto& s : symbolTable) {

            if (s.name == name &&
                (s.scope == currentScope || s.scope == "global"))
            {
                return &s;
            }
        }

        return nullptr;
    }

    bool existsInCurrentScope(const string& name) {

        for (auto& s : symbolTable) {

            if (s.name == name &&
                s.scope == currentScope)
            {
                return true;
            }
        }

        return false;
    }

    // Добавление триады
    int addTriad(
        const string& op,
        const string& arg1 = "",
        const string& arg2 = "")
    {
        triads.push_back({
            triadCounter,
            op,
            arg1,
            arg2
            });

        return triadCounter++;
    }

    // Обход AST
    void visit(NodePtr node) {

        if (!node)
            return;

        switch (node->kind) {

        case ASTKind::Program:
            visitProgram(node);
            break;

        case ASTKind::VarDecl:
            visitVarDecl(node);
            break;

        case ASTKind::FuncDef:
            visitFuncDef(node);
            break;

        case ASTKind::AssignStmt:
            visitAssign(node);
            break;

        case ASTKind::IfStmt:
            visitIf(node);
            break;

        case ASTKind::WhileStmt:
            visitWhile(node);
            break;

        case ASTKind::ExprStmt:
            visitExprStmt(node);
            break;

        case ASTKind::ReturnStmt:
            visitReturn(node);
            break;

        default:

            for (auto& c : node->children)
                visit(c);

            break;
        }
    }

    // Program
    void visitProgram(NodePtr node) {

        // printf из stdio.h
        symbolTable.push_back({
            "printf",
            "int(func)",
            "global",
            true,
            true,
            "Внешняя функция"
            });

        for (auto& child : node->children)
            visit(child);
    }

    // VarDecl
    void visitVarDecl(NodePtr node) {

        string varName = node->attr;
        string varType = "unknown";

        if (!node->children.empty()) {

            if (node->children[0]->kind == ASTKind::TypeSpec)
                varType = node->children[0]->attr;
        }

        // Проверка повторного объявления
        if (existsInCurrentScope(varName)) {
            /*errors.push_back({
                "Ошибка: повторное объявление переменной '" + varName + "'"
                });*/
            errors.push_back({
                "Повторное объявление переменной",
                "Переменная '" + varName + "' уже существует в текущей области видимости",
                node->line
            });
            return;
        }

        bool initialized = false;

        if (node->children.size() > 1)
            initialized = true;

        symbolTable.push_back({
            varName,
            varType,
            currentScope,
            true,
            initialized,
            "Переменная"
            });

        // Генерация триады инициализации
        if (initialized) {

            string expr = evaluateExpression(node->children[1]);

            addTriad(
                "assign",
                varName,
                expr
            );
        }
    }

    // FuncDef
    void visitFuncDef(NodePtr node) {
        string funcName = node->attr;

        string returnType = "void";

        if (!node->children.empty())
            returnType = node->children[0]->attr;

        // Проверка повторного объявления функции
       /* if (functions.count(funcName)) {
            errors.push_back({
                "Повторное объявление функции",
                "Функция '" + funcName + "' уже была объявлена",
                node->line
                });
            return;
        }*/

        FunctionInfo info;
        info.returnType = returnType;

        symbolTable.push_back({
            funcName,
            returnType + "(func)",
            "global",
            true,
            true,
            "Функция"
            });

        string oldScope = currentScope;
        currentScope = funcName;

        currentFunctionReturnType = returnType;

        // Параметры
        if (node->children.size() > 1) {
            auto params = node->children[1];

            if (params->kind == ASTKind::ParamList) {

                for (auto& p : params->children) {

                    string paramName = p->attr;
                    string paramType = "unknown";

                    if (!p->children.empty())
                        paramType = p->children[0]->attr;

                    info.paramTypes.push_back(paramType);

                    symbolTable.push_back({
                        paramName,
                        paramType,
                        currentScope,
                        true,
                        true,
                        "Параметр"
                        });
                }
            }
        }

        functions[funcName] = info;

        // Тело функции
        for (auto& c : node->children)
            visit(c);

        currentScope = oldScope;
    }

    // AssignStmt
    void visitAssign(NodePtr node) {

        if (node->children.size() < 2)
            return;

        auto left = node->children[0];
        auto right = node->children[1];

        string varName = left->attr;

        Symbol* s = findSymbol(varName);

        string leftType = s ? s->type : "unknown";
        string rightType = getExpressionType(right);

        if (!s) {
           /* errors.push_back({"Ошибка: использование необъявленной переменной '" + varName + "'"});*/
            errors.push_back({
                "Использование необъявленной переменной",
                "Переменная '" + varName + "' не была объявлена",
                node->line
            });
            return;
        }

        if (leftType != rightType && rightType != "unknown") {
            /*errors.push_back({"Ошибка: нельзя присвоить тип '" +
                rightType + "' переменной типа '" + leftType + "'"});*/
            errors.push_back({
                "Несовместимое присваивание",
                "Нельзя присвоить значение типа '" + rightType +
                "' переменной типа '" + leftType + "'",
                node->line
            });
        }

        string expr = evaluateExpression(right);

        s->initialized = true;

        addTriad("assign", varName, expr);
    }

    // IfStmt
    void visitIf(NodePtr node) {

        if (node->children.empty())
            return;

        string cond = evaluateExpression(node->children[0]);

        int condTriad = addTriad(
            "if",
            cond,
            ""
        );

        for (size_t i = 1; i < node->children.size(); i++)
            visit(node->children[i]);
    }

    // WhileStmt
    void visitWhile(NodePtr node) {

        if (node->children.size() < 2)
            return;

        int start = triadCounter;

        string cond = evaluateExpression(node->children[0]);

        addTriad(
            "while",
            cond,
            ""
        );

        visit(node->children[1]);

        addTriad(
            "goto",
            to_string(start),
            ""
        );
    }

    // ExprStmt
    void visitExprStmt(NodePtr node) {

        if (!node->children.empty())
            evaluateExpression(node->children[0]);
    }

    // ReturnStmt
   /* void visitReturn(NodePtr node) {

        if (node->children.empty()) {

            addTriad("ret");
            return;
        }

        string val = evaluateExpression(node->children[0]);

        addTriad(
            "ret",
            val,
            ""
        );
    }*/

    void visitReturn(NodePtr node) {

        if (node->children.empty()) {

            if (currentFunctionReturnType != "void") {
               /* errors.push_back({
                    "Ошибка: функция должна возвращать значение типа '" +
                    currentFunctionReturnType + "'"
                    });*/
                errors.push_back({
                    "Отсутствует возвращаемое значение",
                    "Функция должна возвращать значение типа '" +
                    currentFunctionReturnType + "'",
                    node->line
                });
            }

            addTriad("ret");
            return;
        }

        string returnType =
            getExpressionType(node->children[0]);

        if (returnType != currentFunctionReturnType &&
            returnType != "unknown")
        {
            /*errors.push_back({
                "Ошибка: return имеет тип '" +
                returnType +
                "', ожидался '" +
                currentFunctionReturnType + "'"
                });*/
            errors.push_back({
                "Неверный тип возвращаемого значения",
                "Функция должна возвращать '" +
                currentFunctionReturnType +
                "', но return имеет тип '" +
                returnType + "'",
                node->line
            });
        }

        string val = evaluateExpression(node->children[0]);

        addTriad( "ret", val, "" );
    }

    // Вычисление выражения
    string evaluateExpression(NodePtr node) {

        if (!node)
            return "";

        switch (node->kind) {

        case ASTKind::Identifier:
        {
            Symbol* s = findSymbol(node->attr);

            if (!s) {
               /* errors.push_back({
                    "Ошибка: использование необъявленной переменной '" + node->attr + "'"
                    });*/
                errors.push_back({
                    "Использование необъявленной переменной",
                    "Переменная '" + node->attr + "' не была объявлена",
                    node->line
                });
                return node->attr;
            }

            if (!s->initialized) {
                /*errors.push_back({
                    "Ошибка: использование неинициализированной переменной '" + node->attr + "'"
                    });*/
                errors.push_back({
                    "Использование неинициализированной переменной",
                    "Переменная '" + node->attr +
                    "' используется до инициализации",
                    node->line
                });
            }

            return node->attr;
        }

        case ASTKind::IntLiteral:
        case ASTKind::FloatLiteral:
        case ASTKind::StringLiteral:
        case ASTKind::CharLiteral:

            return node->attr;

        case ASTKind::BinaryExpr:
        {
            string left = evaluateExpression(node->children[0]);
            string right = evaluateExpression(node->children[1]);

            int num = addTriad(
                node->attr,
                left,
                right
            );

            return "^" + to_string(num);
        }

        case ASTKind::UnaryExpr:
        {
            string val = evaluateExpression(node->children[0]);

            int num = addTriad(
                node->attr,
                val,
                ""
            );

            return "^" + to_string(num);
        }


        case ASTKind::CallExpr:
        {
            string funcName = node->attr;

            if (!functions.count(funcName) &&
                funcName != "printf")
            {
               /* errors.push_back({
                    "Ошибка: вызов неизвестной функции '" +
                    funcName + "'"
                    });*/
                errors.push_back({
                    "Вызов неизвестной функции",
                    "Функция '" + funcName + "' не была объявлена",
                    node->line
                });
            }

            vector<string> argTypes;

            if (node->children.size() > 1) {

                auto args = node->children[1];

                for (auto& a : args->children) {

                    argTypes.push_back(
                        getExpressionType(a)
                    );

                    string val = evaluateExpression(a);

                    addTriad(
                        "push",
                        val,
                        ""
                    );
                }
            }

            if (functions.count(funcName)) {

                auto& params =
                    functions[funcName].paramTypes;

                if (params.size() != argTypes.size()) {
                   /* errors.push_back({
                        "Ошибка: неверное количество аргументов при вызове функции '" +
                        funcName + "'"
                        });*/
                    errors.push_back({
                        "Неверное количество аргументов",
                        "Функция '" + funcName + "' вызвана с неверным количеством аргументов",
                        node->line
                    });
                }
                else {

                    for (size_t i = 0; i < params.size(); i++) {

                        if (params[i] != argTypes[i] &&
                            argTypes[i] != "unknown")
                        {
                           /* errors.push_back({
                                "Ошибка: тип аргумента " + to_string(i + 1) +
                                " функции '" + funcName + "' не совпадает"
                                });*/
                            errors.push_back({
                                "Несовместимый тип аргумента",
                                "Аргумент " + to_string(i + 1) +
                                " функции '" + funcName + "' имеет неверный тип",
                                node->line
                            });
                        }
                    }
                }
            }

            int num = addTriad(
                "call",
                funcName,
                ""
            );

            return "^" + to_string(num);
        }

        default:
            return "";
        }
    }
};

// Функции вывода
void printSymbolTable(const vector<Symbol>& symbolTable) {
    cout << "\nТаблица символов\n\n";

    cout << "|" << left << setw(15) << "Имя"
        << "|" << left << setw(15) << "Тип"
        << "|" << left << setw(15) << "Область"
        << "|" << left << setw(15) << "Объявлена"
        << "|" << left << setw(15) << "Инициализирована"
        << "|" << left << setw(25) << "Примечание"
        << "|\n";

    cout << "+" << string(15, '-')
        << "+" << string(15, '-')
        << "+" << string(15, '-')
        << "+" << string(15, '-')
        << "+" << string(15, '-')
        << "+" << string(25, '-')
        << "+\n";

    for (const auto& s : symbolTable) {
        cout << "|" << left << setw(15) << s.name
            << "|" << left << setw(15) << s.type
            << "|" << left << setw(15) << s.scope
            << "|" << left << setw(15) << (s.declared ? "+" : "-")
            << "|" << left << setw(15) << (s.initialized ? "+" : "-")
            << "|" << left << setw(25) << s.note
            << "|\n";
    }
    cout << string(105, '-') << "\n";
}

void printTriads(const vector<Triad>& triads) {
    cout << "\nТриады\n\n";

    cout << "|" << left << setw(8) << "№"
        << "|" << left << setw(20) << "Операция"
        << "|" << left << setw(25) << "Операнд 1"
        << "|" << left << setw(25) << "Операнд 2"
        << "|\n";

    cout << "+" << string(8, '-')
        << "+" << string(20, '-')
        << "+" << string(25, '-')
        << "+" << string(25, '-')
        << "+\n";

    for (const auto& t : triads) {
        cout << "|" << left << setw(8) << t.number
            << "|" << left << setw(20) << t.op
            << "|" << left << setw(25) << t.arg1
            << "|" << left << setw(25) << t.arg2
            << "|\n";
    }
    cout << string(85, '-') << "\n";
}

void printSemanticErrors(const vector<SemanticError>& errors) {
    if (errors.empty()) {
        cout << "\nСемантических ошибок не найдено.\n";
        return;
    }

    cout << "\nСемантические ошибки\n\n";
    for (const auto& e : errors) {
        /*cout << "- " << e.message << "\n";*/
        cout << "Строка " << e.line << ": [" << e.message << "] - " << e.details << "\n";
    }
}

// Запуск семантического анализатора
int runSemanticAnalyzer(NodePtr ast) {
    SemanticAnalyzer analyzer;

    analyzer.analyze(ast);
    printSymbolTable(analyzer.getSymbolTable());
    printTriads(analyzer.getTriads());
    printSemanticErrors(analyzer.getErrors());

    return 0;
}