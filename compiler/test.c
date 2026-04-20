#include <stdio.h>

// Глобальная переменная - однострочный комментарий
int globalCounter = 0;

/* Функция для сложения двух чисел
   Многострочный комментарий */
int addNumbers(int x, int y) {
    return x + y;  // Возвращаем сумму
}

int main() {
    int a = 10;      // Первая переменная
    int b = 20;      // Вторая переменная
    int result;      // Результат операции

    /* Выполняем
       арифметические
       операции */
    result = a + b;

    // Условный оператор if-else
    if (result > 25) {
        globalCounter = globalCounter + 1;
    }
    else {
        globalCounter = globalCounter - 1;
    }

    // Цикл for для демонстрации
    for (int i = 0; i < 3; i++) {
        /* Внутри цикла
           вызываем функцию */
        int temp = addNumbers(i, globalCounter);
        result = result + temp;
    }

    // Цикл while
    while (globalCounter < 5) {
        globalCounter = globalCounter + 1;
    }

    /* Логическое выражение
       и возврат результата */
    int isValid = (result > 0) && (globalCounter < 10);

    printf("Строка с // однострочным комментарием внутри\n");
    printf("Строка с /* многострочным комментарием */ внутри\n");

    return 0;
}