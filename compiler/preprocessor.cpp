#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

using namespace std;

// Сохранение строк
vector<string> saveStrings(string& code) {
    vector<string> strings;
    regex strRegex(R"("(\\.|[^"\\])*")");

    string newCode;
    size_t lastPos = 0;
    int index = 0;

    auto begin = sregex_iterator(code.begin(), code.end(), strRegex);
    auto end = sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        smatch match = *it;
        newCode += code.substr(lastPos, match.position() - lastPos);
        strings.push_back(match.str());
        newCode += "__STR" + to_string(index++) + "__";
        lastPos = match.position() + match.length();
    }

    newCode += code.substr(lastPos);
    code = newCode;
    return strings;
}

// Восстанавление строк
void restoreStrings(string& code, const vector<string>& strings) {
    for (size_t i = 0; i < strings.size(); i++) {
        code = regex_replace(code, regex("__STR" + to_string(i) + "__"), strings[i]);
    }
}

// Проверка незакрытых многострочных комментариев
bool checkUnclosedComments(const string& code) {
    int openCount = 0;
    int closeCount = 0;

    for (size_t i = 0; i < code.length() - 1; i++) {
        if (code[i] == '/' && code[i + 1] == '*') {
            openCount++;
            i++;
        }
        else if (code[i] == '*' && code[i + 1] == '/') {
            closeCount++;
            i++;
        }
    }

    if (openCount > closeCount) {
        cout << "Ошибка: незакрытый многострочный комментарий\n";
        return false;
    }
    else if (openCount < closeCount)
    {
        cout << "Ошибка: у многострочного комментария нет открывающего символа\n";
        return false;
    }
    return true;
}


// Удаление многострочных комментариев
void removeMultiLineComments(string& code) {
    code = regex_replace(code, regex("/\\*[\\s\\S]*?\\*/"), "");
}

// Удаление однострочных комментариев
void removeSingleLineComments(string& code) {
    code = regex_replace(code, regex("//.*"), "");
}

// Очистка пробельных символов
void cleanWhitespace(string& code) {
    // Удаление пробелов и табуляции в начале всего текста
    code = regex_replace(code, regex("^[ \\t]+"), "");

    // Удаление пробелов и табуляции в конце всего текста
    code = regex_replace(code, regex("[ \\t]+$"), "");

    // Замена последовательностей пробелов на один пробел
    code = regex_replace(code, regex("[ ]{2,}"), " ");
}

// Удаление пустых строк
void removeEmptyLines(string& code) {
    code = regex_replace(code, regex("\n\\s*\n"), "\n");
}

int runPreprocessor() {
    setlocale(LC_ALL, "Russian");

    ifstream input("test.c");
    if (!input.is_open()) {
        cout << "Ошибка: не удалось открыть файл с программой\n";
        return 1;
    }

    //cout << "Файл с программой успешно открыт\n";

    string code((istreambuf_iterator<char>(input)),
        istreambuf_iterator<char>());
    input.close();

    vector<string> strings = saveStrings(code);

    if (!checkUnclosedComments(code)) {
        return 1;
    }

    // Удаление комментариев
    removeMultiLineComments(code);
    removeSingleLineComments(code);
    //cout << "Комментарии удалены\n";

    // Очистка пробелов и пустых строк
    cleanWhitespace(code);
    removeEmptyLines(code);
    //cout << "Пробелы и пустые строки очищены\n";

    // Восстанавление строк
    restoreStrings(code, strings);
    //cout << "Строки восстановлены\n";

    // Запись результата
    ofstream output("test_cleaned.c");
    output << code;
    output.close();

    cout << "Стадия препроцессора выполнена успешно\n";
    cout << "Результат записан в файл test_cleaned.c\n";
    
    /*cout << "\nРезультат:\n\n";
    cout << code;*/

    return 0;
}