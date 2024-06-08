#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE_1 10
#define SIZE_2 10

int cmp(const void *a, const void *b)
{
    return strcmp((char *)a, (char *)b); //升序
}

int main()
{
    srand(time(NULL));
    char s[SIZE_1][SIZE_2] = {0};
    for (int i = 0; i < SIZE_1; i++)
        s[i][0] = rand()%26+'a';
    qsort(s, SIZE_1, sizeof(s[0]), cmp);
    for (int i = 0; i < SIZE_1; i++)
        printf("%d: %s\n", i, s[i]);
    return 0;
}
