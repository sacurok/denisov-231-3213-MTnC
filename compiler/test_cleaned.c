#include <stdio.h>
int globalCounter = 0;
int addNumbers(int x, int y) {
return x + y;
}
int main() {
int a = 10;
int b = 20;
int result;
result = a + b;
if (result > 25) {
globalCounter = globalCounter + 1;
}
else {
globalCounter = globalCounter - 1;
}
for (int i = 0; i < 3; i++) {
int temp = addNumbers(i, globalCounter);
result = result + temp;
}
while (globalCounter < 5) {
globalCounter = globalCounter + 1;
}
int isValid = (result > 0) && (globalCounter < 10);
printf("Строка с // однострочным комментарием внутри\n");
printf("Строка с /* многострочным комментарием */ внутри\n");
return 0;
}