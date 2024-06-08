#include <string.h>
#include <stdio.h>
#include <time.h>

void Func1();
void Func2();

int main(void)
{
    puts("-- Func1 --");
    Func1();
    puts("-- Func2 --");
    Func2();
    return 0;
}

void Func1()
{
    /* strtok参数1为需要分解的字符串，传入的字符串会被分割，传入NULL表示继续分割最近一次传入的字符串；参数2为包含分割符的字符串，以其中的每一个字符来分割参数1中的字符串。
    分割一次意味着将参数1中的字符串首次出现的参数2中的任意分隔符变为\0(只改变首次出现的一个分隔符)，而后的分割会继续使用分隔符后面的子字符串作为源
    但有一特殊情况，如果字符串一开始就是分隔符，那它会跳过前面连续的分隔符，将第一个不连续的分隔符作为真正的分隔符来处理
    字符串的分割操作是直接作用于源字符串的，所以分割的结果是永久存在且过程只能一次性完成。可以改变参数2的内容，以指定不同的分隔符
    返回分割后首字符串的地址，如果字符串中已经没有分割符，则依旧返回此字符串，但下一次调用将返回NULL(因为已经没有字符串了) */
    char input[] = "abc /d,,,,|e,,2f,| g";
    char *p;
    p = strtok(input, " /");
    if (p)
        printf("%s\n", p);
    p = strtok(NULL, ",");
    if (p)
        printf("%s\n", p);
    p = strtok(NULL, ",");
    if (p)
        printf("%s\n", p);
    p = strtok(NULL, " ");
    if (p)
        printf("%s\n", p);
    
    printf("%s\n", input);
    for (int i = 0; i < sizeof(input)-1; i++)
    {
        if (!input[i])
            printf("%d ", i+1);
    }
    putchar('\n');
}

void Func2()
{
    char s1[100] = "string", s2[10];
    snprintf(s1, sizeof(s1), "%s,%d,%s,%s", "a", 11, s1, "b");
    puts(s1);
    memset(s1, 0, sizeof(s1));
    strcpy(s2, "string");
    sprintf(s1, "%s,%d,%s,%s", "a", 11, s2, "b");
    puts(s1);

    time_t t;
    time(&t);
    char *s01 = ctime(&t);
    printf("%p, %s", s01, s01);
    s01 = ctime(&t);
    *strstr(s01, "\n") = 0;
    printf("%p, %s\n", s01, s01);

    printf("aa"
        "bb\
        cc\n");

    char *buf = "hello world 133";
	char a1[10], a2[10];
	int a3;
	sscanf(buf, "%s %s %d", a1, a2, &a3);
    printf("%s, %s, %d\n", a1, a2, a3);
    memset(a1, 0, sizeof(a1));
    sscanf(buf, "%s ", a1);
    printf("%s\n", a1);
}
