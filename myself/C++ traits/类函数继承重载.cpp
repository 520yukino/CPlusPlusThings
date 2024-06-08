#include<cstdlib>
#include<iostream>

using namespace std;
int n[2];

class Parent01
{
	void f1(){}
	int a1;
protected:
	void f2() {}
	int a2;
public:
	Parent01() { cout << "Parent01(), n = " << ++n[0] << endl; }
	~Parent01() { cout << "~Parent01(), n = " << --n[0] << endl; }
	virtual void Virtual()
	{
		cout << "Parent01类的虚函数: Virtual()" << endl;
	}
	void fun()
	{
		cout << "Parent01类的无参函数: fun()" << endl;
	}
	void fun(int i)
	{
		cout << "Parent01类的有参函数: fun(int i)" << endl;
	}
	virtual void fun(int i, int j)
	{
		cout << "Parent01类的虚函数: func(int i, int j)" << endl;
	}
};
 
class Children01: public Parent01
{
	Parent01 *p;
public:
	Children01() { cout << "Children01(), n = " << ++n[1] << endl; }
	~Children01() { cout << "~Children01(), n = " << --n[1] << endl; }
	void fun()
	{
		cout << "Parent01类的无参函数: fun()" << endl;
	}
	void fun(int i, int j)
	{
		cout << "Children01类的虚函数: fun(int i, int j)" << endl;
	}
	void fun(int i, int j, int k)
	{
		cout << "Children01的函数: fun(int i, int j, int k)" << endl;
		// f1(); //私有函数不能通过继承访问
		// p->f2(); //保护函数不能通过指针或对象访问
		f2(); //但直接使用是没问题的
	}
}; 
 
void run01(Parent01* parent) //外部调用此函数时如果子类是保护或私有继承，子类是无法传给基类引指的，因为这种继承方式会使得子类实例无法使用基类公有部分；但如果此函数在子类中被使用，则可以转换，因为在子类中可以使用基类公有部分
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
	parent.fun(1, 2);
	child.fun(1, 2);
	child.fun(1, 2, 3);
	child.Parent01::fun(1, 2);
	run01(&child);
}

int main()
{
	main01();
	// system("pause");
}

class Base {
public:
	int a;
};

void TakeBase(Base& base) {
	base.a = 1;
}

class Child : private Base {
 public:
  static void UseChildAsBase() {
    Child child;
    TakeBase(child);
  }
};
