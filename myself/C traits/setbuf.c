#include <stdio.h>
char outbuf[BUFSIZ];

int main(void)
{
    setbuf(stdout, outbuf); //把缓冲区与流相连
    puts("This is a test of buffered output.");
    puts(outbuf);
    puts("***");
    fflush(stdout);
    puts(outbuf);
    return 0;
}