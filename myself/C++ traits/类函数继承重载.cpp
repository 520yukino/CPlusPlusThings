#include<cstdlib>
#include<iostream>
 
using namespace std;
 
class Parent01
{
public:
	Parent01();
	~Parent01();
	virtual void Virtual()
	{
		cout << "Parent01类的虚函数:Virtual()" << endl;
	}
	void fun()
	{
		cout << "Parent01类的无参函数fun" << endl;
	}
	void fun(int i)
	{
		cout << "Parent01类的有参函数fun" << endl;
	}
	virtual void fun(int i, int j)
	{
		cout << "Parent01类的虚函数:void func(int i, int j)" << endl;
	}
protected:
	void f() {}
};
 
Parent01::Parent01()
{
	cout << "Parent01类的构造函数" << endl;
}
 
Parent01::~Parent01()
{
	cout << "Parent01类的析构函数" << endl;
}
 
class Children01: public Parent01
{
	Parent01 p;
public:
	Children01();
	~Children01();
	// void fun(int i,int j)
	// {
	// 	cout << "Children01类的虚函数fun(int i,int j)" << endl;
	// }
	void fun(int i,int j,int k)
	{
		cout << "Children01的函数void fun(int i,int j,int k)" << endl;
		// p.f(); //保护函数不能通过指针或对象访问
		f(); //但继承可以访问protected
	}
};

Children01::Children01()
{
	cout << "Children01类的构造函数" << endl;
}
 
Children01::~Children01()
{
	cout << "Children01类的析构函数" << endl;
}
 
 
void run01(Parent01* parent)
{
	parent->fun(0, 1);
	//parent->Virtual();
}
 
void main01()
{
	Children01 child;
	run01(&child);
	child.Parent01::fun();
	Children01(); //Parent构造而后Child构造然后析构Child析构最后Parent析构
	Parent01 parent;
	run01(&parent);
	parent.fun(1, 2);
	child.fun(1, 2, 3);
	child.Parent01::fun(1, 2);
}

int main()
{
	main01();
	system("pause");
}