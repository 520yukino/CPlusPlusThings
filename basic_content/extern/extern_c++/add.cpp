#include <iostream>
using namespace std;
extern "C" {
    #include "add.h"
}
/* int add(int x) {
    puts("C++\n");
    return x;
} */
int main() {
    add(2,3);
    getchar();
    return 0;
}
