#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <pthread.h>

void *F1(void *)
{
    for (int i = 0; i<20; i++)
    {
        Sleep(500);
        puts("test");
    }
    return NULL;
}

int main()
{
    int a = 11;
    char s[10];
    sprintf(s, "aaaaaaaaaaaaaaa");
    puts(s);
    pthread_t pth;
    pthread_create(&pth, NULL, F1, NULL);
    scanf("%d", &a);
    printf("%d", a);
}