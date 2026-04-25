//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include <regex>
//#include <map>
//#include <set>
//
//using namespace std;
//
//// Типы лексем
//enum TokenType {
//    KEYWORD,
//    IDENTIFIER,
//    CONSTANT_INT,
//    CONSTANT_FLOAT,
//    CONSTANT_STRING,
//    CONSTANT_BOOL,
//    OPERATOR,
//    DELIMITER,
//    UNKNOWN,
//    END_OF_FILE
//};
//
//// Структура токена
//struct Token {
//    TokenType type;
//    string value;
//    int line;
//    int column;
//
//    Token(TokenType t, const string& v, int l, int c)
//        : type(t), value(v), line(l), column(c) {
//    }
//};
//
//// Класс лексического анализатора
//class LexicalAnalyzer {
//private:
//    string source;
//    size_t position;
//    int currentLine;
//    int currentColumn;
//    vector<Token> tokens;
//
//    // Таблицы лексем
//    set<string> keywords = {
//        "int", "return", "if", "else", "for", "while", "include"
//    };
//
//    set<string> operators = {
//        "=", "+", "-", "*", "/", ">", "<", ">=", "<=", "==", "!=",
//        "&&", "||", "!", "++", "--", "+=", "-=", "*=", "/="
//    };
//
//    set<char> delimiters = {
//        ';', ',', '(', ')', '{', '}', '#', '<', '>', '[', ']', ':'
//    };
//
//    map<TokenType, string> typeNames = {
//        {KEYWORD, "KEYWORD"},
//        {IDENTIFIER, "IDENTIFIER"},
//        {CONSTANT_INT, "CONSTANT_INT"},
//        {CONSTANT_FLOAT, "CONSTANT_FLOAT"},
//        {CONSTANT_STRING, "CONSTANT_STRING"},
//        {CONSTANT_BOOL, "CONSTANT_BOOL"},
//        {OPERATOR, "OPERATOR"},
//        {DELIMITER, "DELIMITER"},
//        {UNKNOWN, "UNKNOWN"},
//        {END_OF_FILE, "END_OF_FILE"}
//    };
//
//    // Проверка на допустимый символ
//    bool isValidChar(char c) {
//        return isalnum(c) || c == '_' || c == '.' || c == '"' ||
//            operators.find(string(1, c)) != operators.end() ||
//            delimiters.find(c) != delimiters.end() ||
//            c == ' ' || c == '\t' || c == '\n' || c == '\r';
//    }
//
//    // Проверка начала идентификатора
//    bool isIdentifierStart(char c) {
//        return isalpha(c) || c == '_';
//    }
//
//    // Проверка символа идентификатора
//    bool isIdentifierChar(char c) {
//        return isalnum(c) || c == '_';
//    }
//
//    // Проверка цифры
//    bool isDigit(char c) {
//        return isdigit(c);
//    }
//
//    // Пропуск пробельных символов
//    void skipWhitespace() {
//        while (position < source.length() &&
//            (source[position] == ' ' || source[position] == '\t' ||
//                source[position] == '\n' || source[position] == '\r')) {
//            if (source[position] == '\n') {
//                currentLine++;
//                currentColumn = 1;
//            }
//            else {
//                currentColumn++;
//            }
//            position++;
//        }
//    }
//
//    // Чтение идентификатора или ключевого слова
//    Token readIdentifier() {
//        int startLine = currentLine;
//        int startColumn = currentColumn;
//        string value;
//
//        while (position < source.length() && isIdentifierChar(source[position])) {
//            value += source[position];
//            position++;
//            currentColumn++;
//        }
//
//        // Проверка на ключевое слово
//        if (keywords.find(value) != keywords.end()) {
//            return Token(KEYWORD, value, startLine, startColumn);
//        }
//
//        return Token(IDENTIFIER, value, startLine, startColumn);
//    }
//
//    // Чтение числа
//    Token readNumber() {
//        int startLine = currentLine;
//        int startColumn = currentColumn;
//        string value;
//        bool hasDot = false;
//
//        while (position < source.length() && (isDigit(source[position]) || source[position] == '.')) {
//            if (source[position] == '.') {
//                if (hasDot) {
//                    // Вторая точка подряд - ошибка
//                    cout << "Ошибка: некорректное число (две точки подряд) в строке "
//                        << currentLine << ", позиция " << currentColumn << endl;
//                    return Token(UNKNOWN, value + ".", startLine, startColumn);
//                }
//                hasDot = true;
//            }
//            value += source[position];
//            position++;
//            currentColumn++;
//        }
//
//        // Проверка, что число не заканчивается на точку
//        if (hasDot && value.back() == '.') {
//            cout << "Ошибка: число не может заканчиваться точкой в строке "
//                << currentLine << ", позиция " << currentColumn << endl;
//            return Token(UNKNOWN, value, startLine, startColumn);
//        }
//
//        // Проверка, что после числа не идёт буква
//        if (position < source.length() && isalpha(source[position])) {
//            cout << "Ошибка: некорректное число (буква в цифровой константе) в строке "
//                << currentLine << ", позиция " << currentColumn << endl;
//            // Пропускаем ошибочные символы
//            while (position < source.length() && isIdentifierChar(source[position])) {
//                value += source[position];
//                position++;
//                currentColumn++;
//            }
//            return Token(UNKNOWN, value, startLine, startColumn);
//        }
//
//        if (hasDot) {
//            return Token(CONSTANT_FLOAT, value, startLine, startColumn);
//        }
//        return Token(CONSTANT_INT, value, startLine, startColumn);
//    }
//
//    // Чтение строковой константы
//    Token readString() {
//        int startLine = currentLine;
//        int startColumn = currentColumn;
//        string value;
//
//        // Добавляем открывающую кавычку
//        value += source[position];
//        position++;
//        currentColumn++;
//
//        bool closed = false;
//        while (position < source.length()) {
//            if (source[position] == '"') {
//                value += source[position];
//                position++;
//                currentColumn++;
//                closed = true;
//                break;
//            }
//            else if (source[position] == '\n') {
//                cout << "Ошибка: незакрытая строковая константа в строке "
//                    << currentLine << ", позиция " << startColumn << endl;
//                break;
//            }
//            value += source[position];
//            position++;
//            currentColumn++;
//        }
//
//        if (!closed) {
//            cout << "Ошибка: достигнут конец файла, строковая константа не закрыта" << endl;
//        }
//
//        return Token(CONSTANT_STRING, value, startLine, startColumn);
//    }
//
//    // Чтение оператора или разделителя
//    Token readOperatorOrDelimiter() {
//        int startLine = currentLine;
//        int startColumn = currentColumn;
//        string value;
//
//        // Пробуем прочитать двухсимвольный оператор
//        if (position + 1 < source.length()) {
//            string twoChars = source.substr(position, 2);
//            if (operators.find(twoChars) != operators.end()) {
//                value = twoChars;
//                position += 2;
//                currentColumn += 2;
//                return Token(OPERATOR, value, startLine, startColumn);
//            }
//        }
//
//        // Читаем односимвольный оператор или разделитель
//        value = source[position];
//        if (operators.find(value) != operators.end()) {
//            position++;
//            currentColumn++;
//            return Token(OPERATOR, value, startLine, startColumn);
//        }
//        else if (delimiters.find(value[0]) != delimiters.end()) {
//            position++;
//            currentColumn++;
//            return Token(DELIMITER, value, startLine, startColumn);
//        }
//
//        // Неизвестный символ
//        cout << "Ошибка: недопустимый символ '" << value
//            << "' в строке " << currentLine << ", позиция " << currentColumn << endl;
//        position++;
//        currentColumn++;
//        return Token(UNKNOWN, value, startLine, startColumn);
//    }
//
//public:
//    LexicalAnalyzer(const string& code) : source(code), position(0), currentLine(1), currentColumn(1) {}
//
//    // Основной метод анализа
//    vector<Token> analyze() {
//        tokens.clear();
//
//        while (position < source.length()) {
//            skipWhitespace();
//
//            if (position >= source.length()) {
//                break;
//            }
//
//            char current = source[position];
//            Token token(UNKNOWN, "", currentLine, currentColumn);
//
//            if (isIdentifierStart(current)) {
//                token = readIdentifier();
//            }
//            else if (isDigit(current)) {
//                token = readNumber();
//            }
//            else if (current == '"') {
//                token = readString();
//            }
//            else {
//                token = readOperatorOrDelimiter();
//            }
//
//            // Добавляем только значимые токены (не пробелы)
//            if (token.type != UNKNOWN || token.value.empty()) {
//                tokens.push_back(token);
//            }
//        }
//
//        // Добавляем маркер конца файла
//        tokens.push_back(Token(END_OF_FILE, "EOF", currentLine, currentColumn));
//
//        return tokens;
//    }
//
//    // Вывод результатов
//    void printResults() {
//        cout << "\nЛексема" << string(40, ' ') << "| Тип" << endl;
//        cout << string(80, '-') << endl;
//
//        for (const auto& token : tokens) {
//            if (token.type != END_OF_FILE) {
//                cout << token.value;
//                // Выравнивание
//                if (token.value.length() < 50) {
//                    cout << string(50 - token.value.length(), ' ');
//                }
//                cout << "| " << typeNames[token.type] << endl;
//            }
//        }
//
//        cout << "\nПоследовательность токенов:\n[";
//        for (size_t i = 0; i < tokens.size(); i++) {
//            if (tokens[i].type != END_OF_FILE) {
//                cout << "(" << typeNames[tokens[i].type] << ", " << tokens[i].value << ")";
//                if (i < tokens.size() - 2) cout << ", ";
//            }
//        }
//        cout << "]\n";
//
//        cout << "\nЛексический анализ завершён. Обнаружено " << tokens.size() - 1 << " токенов.\n";
//
//        // Проверка на наличие ошибок
//        bool hasErrors = false;
//        for (const auto& token : tokens) {
//            if (token.type == UNKNOWN) {
//                hasErrors = true;
//                break;
//            }
//        }
//
//        if (!hasErrors) {
//            cout << "Ошибок не найдено.\n";
//        }
//        else {
//            cout << "Обнаружены лексические ошибки.\n";
//        }
//    }
//
//    // Сохранение токенов в файл
//    void saveTokensToFile(const string& filename) {
//        ofstream out(filename);
//        for (const auto& token : tokens) {
//            if (token.type != END_OF_FILE) {
//                out << typeNames[token.type] << " : " << token.value << endl;
//            }
//        }
//        out.close();
//    }
//};
//
//int main() {
//    setlocale(LC_ALL, "Russian");
//
//    // Читаем очищенный код из ЛР1
//    ifstream input("test_cleaned.c");
//    if (!input.is_open()) {
//        cout << "Ошибка: не удалось открыть очищенный файл test_cleaned.c" << endl;
//        cout << "Сначала запустите препроцессор из лабораторной работы 1" << endl;
//        return 1;
//    }
//
//    string code((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
//    input.close();
//
//    cout << "=== ЛЕКСИЧЕСКИЙ АНАЛИЗАТОР ===" << endl;
//    cout << "Анализируемый код:\n" << code << endl;
//    cout << string(80, '=') << endl;
//
//    // Запуск лексического анализатора
//    LexicalAnalyzer analyzer(code);
//    vector<Token> tokens = analyzer.analyze();
//
//    // Вывод результатов
//    analyzer.printResults();
//
//    // Сохранение результатов
//    analyzer.saveTokensToFile("tokens.txt");
//    cout << "\nРезультаты сохранены в файл tokens.txt" << endl;
//
//    return 0;
//}