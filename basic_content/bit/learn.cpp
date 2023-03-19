#include<iostream>

using namespace std;
struct stuff 
{
    unsigned int field1: 30;
    unsigned int       : 2;
    unsigned int field2: 4;
    //unsigned int       : 0; //插入0字段会促使位域自动对齐，即变成3*32字节而非64
    unsigned int field3: 25; 
};
union u1
{
    stuff s;
    unsigned long long a;
};

int main()
{
    u1 u={.s={11111,3,8}};
    cout<<u.s.field1<<endl;
    cout<<u.s.field2<<endl;
    cout<<u.s.field3<<endl;
    cout<<sizeof(u)<<endl;

    u.a = 0;
    cout<<u.s.field1<<endl;
    cout<<u.s.field2<<endl;
    cout<<u.s.field3<<endl;
    cout<<sizeof(u)<<endl;
}
