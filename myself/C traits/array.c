#include <stdio.h>
#include <stdlib.h>

void Func1();

int main(void)
{
    puts("-- Func1 --");
    Func1();
    return 0;
}

void Func1()
{
    int a[4][3] = {{1,2,3}, [2] = {11}, {21,22,23}};
    for (int i = 0; i < sizeof(a)/sizeof(a[0]); i++)
        for (int j = 0; j < sizeof(a[0])/sizeof(a[0][0]); j++)
            printf("%d ", a[i][j]);
    putchar('\n');
    int b1 = 1/0;
    printf("%d %d\n", b1, 2/0);
}
