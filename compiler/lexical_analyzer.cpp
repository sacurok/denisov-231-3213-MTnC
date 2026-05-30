#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <regex>
#include <iomanip>
#include "common.cpp"

using namespace std;

// Таблицы лексем
const set<string> KEYWORDS = {
    "int", "return", "if", "else", "while", "for", "break", "case", "char", "const", "continue",
    "double", "float", "switch", "void"
};

const set<string> OPERATORS = {
    "+", "-", "*", "/", "%", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=", "<", ">", "<=", ">=",
    "&&", "||", "!", "&", "|", "^", "~", "<<", ">>", "++", "--", "->", "."
};

const set<char> DELIMITERS = {
    ';', ',', '(', ')', '{', '}', '[', ']', ':'
};

// Типы лексем
//enum TokenType {
//    KEYWORD,
//    IDENTIFIER,
//    CONSTANT_INT,
//    CONSTANT_FLOAT,
//    CONSTANT_STRING,
//    CONSTANT_CHAR,
//    OPERATOR,
//    DELIMITER,
//    PREPROCESSOR,
//    UNKNOWN
//};

string tokenTypeName(TokenType type) {
    switch (type) {
    case KEYWORD: return "KEYWORD";
    case IDENTIFIER: return "IDENTIFIER";
    case CONSTANT_INT: return "CONSTANT_INT";
    case CONSTANT_FLOAT: return "CONSTANT_FLOAT";
    case CONSTANT_STRING: return "CONSTANT_STRING";
    case CONSTANT_CHAR: return "CONSTANT_CHAR";
    case OPERATOR: return "OPERATOR";
    case DELIMITER: return "DELIMITER";
    case PREPROCESSOR: return "PREPROCESSOR";
    default: return "UNKNOWN";
    }
}

//struct Token {
//    TokenType type;
//    string value;
//    int line;
//};

struct LexError {
    string message;
    string details;
    int line;
};

// Лексический анализатор
class Lexer {
public:
    vector<Token> tokens;
    vector<LexError> errors;

    void analyze(const string& code) {
        tokens.clear();
        errors.clear();

        size_t i = 0;
        int line = 1;
        size_t n = code.size();

        while (i < n) {
            // Перевод строки
            if (code[i] == '\n') {
                line++;
                i++;
                continue;
            }

            // Пробельные символы
            if (isspace((unsigned char)code[i])) {
                i++;
                continue;
            }

            // Препроцессорные директивы (строки начинающиеся с #)
            /*if (code[i] == '#') {
                size_t start = i;
                while (i < n && code[i] != '\n') i++;
                tokens.push_back({ PREPROCESSOR, code.substr(start, i - start), line });
                continue;
            }*/

            // Препроцессорные директивы (строки начинающиеся с #)
            if (code[i] == '#') {
                size_t start = i;
                i++;

                while (i < n && isspace((unsigned char)code[i])) {
                    i++;
                }

                size_t directiveStart = i;
                while (i < n && isalpha((unsigned char)code[i])) {
                    i++;
                }

                string directive = code.substr(directiveStart, i - directiveStart);

                // Проверяем, является ли директива "include"
                if (directive != "include") {
                    errors.push_back({ "Ошибка в препроцессорной директиве include",
                        "Директива '" + directive + "' не распознана. Возможно, вы имели в виду 'include'", line });
                }

                while (i < n && code[i] != '\n') {
                    i++;
                }
                tokens.push_back({ PREPROCESSOR, code.substr(start, i - start), line });
                continue;
            }

            // Строковые константы
            if (code[i] == '"') {
                size_t start = i;
                i++;
                bool closed = false;
                while (i < n) {
                    if (code[i] == '\\' && i + 1 < n) {
                        i += 2;
                        continue;
                    }
                    if (code[i] == '"') {
                        i++;
                        closed = true;
                        break;
                    }
                    if (code[i] == '\n') break;
                    i++;
                }
                if (!closed) {
                    errors.push_back({ "Незакрытый строковый литерал", 
                        "Строковый литерал не имеет закрывающей кавычки", line});
                }
                else {
                    tokens.push_back({ CONSTANT_STRING, code.substr(start, i - start), line });
                }
                continue;
            }

            // Символьные константы
            if (code[i] == '\'') {
                size_t start = i;
                i++;
                bool closed = false;
                while (i < n) {
                    if (code[i] == '\\' && i + 1 < n) {
                        i += 2;
                        continue;
                    }
                    if (code[i] == '\'') {
                        i++;
                        closed = true;
                        break;
                    }
                    if (code[i] == '\n') break;
                    i++;
                }
                if (!closed) {
                    errors.push_back({ "Незакрытый символьный литерал",
                        "Символьный литерал не имеет закрывающей кавычки", line});
                }
                else {
                    tokens.push_back({ CONSTANT_CHAR, code.substr(start, i - start), line });
                }
                continue;
            }

            // Числовые константы
            if (isdigit((unsigned char)code[i])) {
                size_t start = i;
                bool isFloat = false;
                bool hasTwoDots = false;
                bool hasLetters = false;
                bool hasComma = false;
                int dotCount = 0;

                // Читаем число
                while (i < n && (isdigit((unsigned char)code[i]) || code[i] == '.' ||
                    (i > start && isalpha((unsigned char)code[i])))) {

                    if (code[i] == '.') {
                        dotCount++;
                        if (dotCount == 1) {
                            isFloat = true;
                        }
                        if (dotCount > 1) {
                            hasTwoDots = true;
                        }
                    }
                    else if (isalpha((unsigned char)code[i])) {
                        hasLetters = true;
                    }
                    i++;
                }

                if (i < n && code[i] == ',') {
                    // Проверяем, что перед запятой была цифра и после запятой тоже цифра
                    size_t tempPos = i + 1;
                    if (tempPos < n && isdigit((unsigned char)code[tempPos])) {
                        hasComma = true;
                        // Читаем дальше, чтобы включить в лексему всё число с запятой
                        while (tempPos < n && (isdigit((unsigned char)code[tempPos]) || code[tempPos] == ',')) {
                            if (code[tempPos] == ',') {
                                tempPos++;
                                continue;
                            }
                            tempPos++;
                        }
                        i = tempPos;
                    }
                }

                string lexeme = code.substr(start, i - start);

                if (hasTwoDots) {
                    errors.push_back({ "Ошибка в числовой константе",
                        "Число '" + lexeme + "' содержит две точки подряд", line });
                }
                else if (hasLetters) {
                    errors.push_back({ "Ошибка в числовой константе",
                        "Число '" + lexeme + "' содержит недопустимые символы", line });
                }
                else if (hasComma) {
                    errors.push_back({ "Ошибка в числовой константе",
                        "Число '" + lexeme + "' содержит запятую.", line });
                }
                else if (isFloat) {
                    tokens.push_back({ CONSTANT_FLOAT, lexeme, line });
                }
                else {
                    tokens.push_back({ CONSTANT_INT, lexeme, line });
                }
                continue;
            }

            // Идентификатор или ключевое слово
            if (isalpha((unsigned char)code[i]) || code[i] == '_') {
                size_t start = i;
                while (i < n && (isalnum((unsigned char)code[i]) || code[i] == '_')) i++;
                string lexeme = code.substr(start, i - start);
                if (KEYWORDS.count(lexeme)) {
                    tokens.push_back({ KEYWORD, lexeme, line });
                }
                else {
                    tokens.push_back({ IDENTIFIER, lexeme, line });
                }
                continue;
            }

            // Операторы (двухсимвольные и односимвольные)
            {
                // Двухсимвольный оператор
                if (i + 1 < n) {
                    string two = code.substr(i, 2);
                    if (OPERATORS.count(two)) {
                        tokens.push_back({ OPERATOR, two, line });
                        i += 2;
                        continue;
                    }
                }

                // Односимвольный оператор
                string one = code.substr(i, 1);
                if (OPERATORS.count(one)) {
                    tokens.push_back({ OPERATOR, one, line });
                    i++;
                    continue;
                }
            }

            // Разделители
            if (DELIMITERS.count(code[i])) {
                tokens.push_back({ DELIMITER, string(1, code[i]), line });
                i++;
                continue;
            }

            // Неизвестный оператор
            if (!isalnum((unsigned char)code[i]) && !isspace((unsigned char)code[i]) &&
                code[i] != '\n' && code[i] != '\"' && code[i] != '\'') {
                errors.push_back({ "Неизвестный оператор",
                    "Символ '" + string(1, code[i]) + "' не является допустимым оператором", line });
            }
            else {
                // Недопустимый символ
                errors.push_back({ "Недопустимый символ",
                    "Символ '" + string(1, code[i]) + "' (код " + to_string((int)(unsigned char)code[i]) +
                    ") не распознан в исходном коде", line });
            }
            i++;
        }
    }
};

// Функции вывода
void printTable(const vector<Token>& tokens) {
    cout << "\n";
    cout << "|" << left << setw(30) << "Лексема"
         << "|" << left << setw(20) << "Тип"
         << "|" << left << "Строка\n";

    cout << "+" << string(30, '-') << "+"
        << string(20, '-') << "+"
        << string(8, '-') << "+\n";

    for (size_t i = 0; i < tokens.size(); i++) {
        cout << "|" << left << setw(30) << tokens[i].value
             << "|" << left << setw(20) << tokenTypeName(tokens[i].type)
             << "|" << left << tokens[i].line << "\n";
    }
    cout << string(60, '-') << "\n";
}

void printSequence(const vector<Token>& tokens) {
    cout << "\nПоследовательность лексем:\n[";
    for (size_t i = 0; i < tokens.size(); i++) {
        cout << "(" << tokenTypeName(tokens[i].type) << ", " << tokens[i].value << ")";
        if (i + 1 < tokens.size()) cout << ", ";
    }
    cout << "]\n";
}

void printErrors(const vector<LexError>& errors) {
    if (errors.empty()) return;
    cout << "\n";
    cout << "Лексические ошибки\n";
    for (const auto& e : errors) {
        cout << "Строка " << e.line << ": [" << e.message << "] - " << e.details << "\n";
    }
}

int runLexer() {
    setlocale(LC_ALL, "Russian");

    ifstream input("test_cleaned.c");
    if (!input.is_open()) {
        cerr << "Ошибка: не удалось открыть файл с очищенной программой\n";
        return 1;
    }

    string code((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
    input.close();

    Lexer lexer;
    lexer.analyze(code);

    //printTable(lexer.tokens);
    //printSequence(lexer.tokens);
    printErrors(lexer.errors);

    cout << "\n";
    if (lexer.errors.empty()) {
        cout << "Лексический анализ завершён успешно.\n";
        cout << "Обнаружено " << lexer.tokens.size() << " токенов\n";
        // Запись результата в файл
        ofstream out("lexems.txt");
        /*for (size_t i = 0; i < lexer.tokens.size(); i++) {
            out << tokenTypeName(lexer.tokens[i].type) << "\t" << lexer.tokens[i].value << "\n";
        }*/
         for (size_t i = 0; i < lexer.tokens.size(); i++) {
             out << "(" << tokenTypeName(lexer.tokens[i].type) << ", " << lexer.tokens[i].value << ", " << lexer.tokens[i].line << ")";
             if (i + 1 < lexer.tokens.size()) out << ", ";
         }
        out.close();
        cout << "Последовательность лексем записана в lexems.txt\n";
    }
    else {
        cout << "Лексический анализ завершён с ошибками.\n";
        cout << "Лексемы не были записаны в файл.\n";
    }

    return 0;
}