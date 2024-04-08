#include <iostream>
#include <vector>

using namespace std;

class Test
{
public:
    Test(void);
    Test(const Test &t);
    ~Test(void);
};

Test::Test(void)
{
    cout << "object created\n";
}
Test::Test(const Test &t)
{
    cout << "copy\n";
}
Test::~Test(void)
{
    cout << "-->object deleted\n";
}

int main()
{
    vector<Test> vec(5);
    cout << "after create, before clear ... " << endl;
    vec.clear();
    cout << "after clear ... " << endl;
    cout << vec.capacity() << endl;
    vector<Test>(vec).swap(vec); //复制构造一个临时vector来swap掉原来的vector，而后自动析构，这样vec就不会再有以前的空间大小而是临时量的空间大小
    cout << vec.capacity() << endl;
    vector<Test> v1;
    cout << v1.capacity() << endl;

    return 0;
}