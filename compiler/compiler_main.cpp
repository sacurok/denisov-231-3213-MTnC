#include <iostream>

#include "preprocessor.cpp"
#include "lexical_analyzer.cpp"
#include "syntax_analyzer.cpp"
#include "semantic_analyzer.cpp"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    cout << "1: Препроцессор\n\n";

    if (runPreprocessor() != 0) {
        cout << "\nОшибка на этапе препроцессора\n";
        return 1;
    }

    cout << "\n2: Лексический анализ\n\n";

    if (runLexer() != 0) {
        cout << "\nОшибка на этапе лексического анализа\n";
        return 1;
    }

    cout << "\n3: Синтаксический анализ\n\n";

    /*if (runParser() != 0) {
        cout << "\nОшибка на этапе синтаксического анализа\n";
        return 1;
    }*/

    NodePtr ast = runParser();

    if (!ast) {
        cout << "\nОшибка на этапе синтаксического анализа\n";
        return 1;
    }

    cout << "\n4: Семантический анализ\n\n";

    runSemanticAnalyzer(ast);

    //cout << "\nКомпиляция завершена успешно\n";

    return 0;
}