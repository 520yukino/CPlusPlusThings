#include <stdio.h>
#include <sys/time.h> //更多的时间组件
#define SIZE 1024
//使用标准io的速度要远快于Linux的io函数，原因在于标准io额外拥有输入输出缓冲支持，配合套接字缓冲可加快数据传输
#if 1
int main()
{
    FILE *fp1, *fp2;
    size_t len;
    char buf[SIZE];
    fp1 = fopen("/mnt/d/YX/wuppo/Wup1.0.43.exe", "rb");
    fp2 = fopen("test", "w");
    printf("%p, %p\n", fp1, fp2);

    struct timeval tv1, tv2; //储存当前的秒和微秒
    gettimeofday(&tv1, NULL);
    while ((len = fread(buf, SIZE, 1, fp1))>0)
        fwrite(buf, SIZE, 1, fp2);
    gettimeofday(&tv2, NULL);
    if (tv2.tv_usec>=tv1.tv_usec) //判断微秒的大小
        printf("duration: %lds, %ldus\n", tv2.tv_sec-tv1.tv_sec, tv2.tv_usec-tv1.tv_usec);
    else
        printf("duration: %lds, %ldus\n", tv2.tv_sec-tv1.tv_sec-1, tv1.tv_usec-tv2.tv_usec);

    fclose(fp1);
    fclose(fp2);
}

//网上的测试代码
#else
#include <stdio.h>
#include <string.h>

int main()
{
    FILE *fp;
    char c[] = "This is runoob\n";
    char buffer[20];
 
    /* 打开文件用于读写 */
    fp = fopen("file.txt", "w+");
 
    /* 写入数据到文件 */
    fwrite(c, strlen(c) + 1, 1, fp);
 
    /* 查找文件的开头 */
    fseek(fp, 0, SEEK_SET);
 
    /* 读取并显示数据 */
    fread(buffer, strlen(c)+1, 1, fp);
    printf("%s\n", buffer);
    fclose(fp);
    
    return(0);
}

#endif