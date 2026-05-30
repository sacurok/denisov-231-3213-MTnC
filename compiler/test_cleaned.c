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
овопвдаплврв
if (result > 25) {
globalCounter = globalCounter + 1;
}
else {
globalCounter = globalCounter - 1;
}
while (globalCounter < 5) {
globalCounter = globalCounter + 1;
}
printf("Строка с // однострочным комментарием внутри\n");
printf("Строка с /* многострочным комментарием */ внутри\n");
return 0;
}