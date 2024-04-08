#include <stdio.h>
#include <iostream>
using namespace std;

class CBase
{
public:
	virtual int DoJob()
	{
		return 0;
	}
	virtual CBase *Clone(void)
	{
		return new CBase;
	}
	virtual CBase &GetStatic(void)
	{
		static CBase StaticObj;
		puts("in CBase::GetStatic");
		return StaticObj;
	}
	void invoke() //基类的非虚方法内部可以调用多态方法，原理在于this指针，子对象调用invoke时传入的this指针为子对象而非基对象
	{
		GetStatic();
	}
};

class CDerive1 : public CBase
{
public:
	virtual int DoJob() override //虚函数的覆盖，虚函数如果不覆盖则和普通函数一样直接继承
	{
		return 100;
	}
	virtual CDerive1 *Clone(void)
	{
		return new CDerive1;
	}
	// virtual CDerive1 &GetStatic(void)
	// {
	// 	static CDerive1 m_Static;
	// 	puts("in CDerive1::GetStatic");
	// 	return m_Static;
	// }
};

class CDerive2 : public CDerive1
{
public:
	// virtual float DoJob(){ //返回类型协变只能适用于返回值为某种继承类，不能用于这种基本类型转换
	// 	return 1.23f;
	// }
	virtual int DoJob(int) //而此处则是隐藏，因为参数不同，这种情况下CDerive2无法使用基类的DoJob()
	{
		return 200;
	}
	virtual CDerive2 *Clone(void)
	{
		return new CDerive2;
	}
	CDerive2 &GetStatic(void) //虚函数当然可以隔代覆盖
	{
		static CDerive2 m_Static;
		puts("in CDerive2::GetStatic");
		return m_Static;
	}
};

class CTestVirturalBase
{
public:
	virtual CBase *GetClone(void) = 0;
	virtual CBase &GetStatic(void) = 0;
	virtual CDerive1 *GetBase(void) = 0;
};

class CTestVirtualDev1 : public CTestVirturalBase
{
public:
	virtual CDerive1 *GetClone(void)
	{
		return new CDerive1;
	}
	virtual CDerive1 &GetStatic(void)
	{
		static CDerive1 StaticObj;
		return StaticObj;
	}
	// virtual CBase* GetBase(void){ //协变只能返回基类中对应的虚函数的返回类的子类，思考多态调用时使用基类对象获取返回值，必须这样规定防止bug
	// 	return new CBase;
	// }
	// virtual CDerive1 *GetBase(void) = delete; //子类不能删除继承于基类的虚函数，因为多态时会出错
};

// 隐藏hiding，下层类对上层同名函数进行隐藏，必须使用作用域解析才能访问隐藏函数；覆盖重写override，虚函数重写，产生函数多态
void func(const char *s)
{
	cout << "global function with name:" << s << endl;
}

class B;
class A
{
	A *pa;
	B *pb;
	void func()
	{
		cout << "member function of A" << endl;
	}

public:
	A() {pa = this;} //如果是创建B对象的过程，则此处的this指向B对象而非A对象，即创建谁就指向谁
	A(int) {A::~A();} //构造函数可以调用析构函数，因为析构函数只是执行函数体中的内容，不会删除对象
	~A() {}
	void useFunc()
	{
		// func("lvlv"); //A::func()将外部函数func(char*)隐藏
		func();
		::func("lvlv");
	}
	virtual void print()
	{
		cout << "A's print" << endl;
		pa->print(); //基类可以通过多态调用子类的虚函数
	}
};

class B : public A
{
public:
	B(): A() {}
	void useFunc()
	{ // 隐藏A::vodi useFunc()
		cout << "B's useFunc()" << endl;
	}
	int useFunc(int i)
	{ // 隐藏A::vodi useFunc()
		cout << "In B's useFunc(int i), i = " << i << endl;
		return 0;
	}
	virtual int print(const char *a)
	{
		cout << "B's print(const char *a): " << a << endl;
		return 1;
	}
	virtual void print() {
	    cout << "B's print()" << endl;
	}
};

int main()
{
	CBase c1;
	CDerive1 c2;
	CDerive2 c3;
	CBase &rc1 = c3;
	rc1.GetStatic();
	c3.invoke();
	cout << c2.DoJob() << endl;
	cout << c3.DoJob(1) << endl;
	cout << rc1.DoJob() << endl; //CDerive2的DoJob(int)无法多态调用，因为基类没有这样的函数

	// CTestVirtualDev1 ct1; //不允许使用抽象类类型CTestVirtualDev1的对象，原因在于CTestVirturalBase中的纯虚函数没有全部被覆盖，导致CTestVirtualDev1拥有纯虚函数进而判定为ABC
	A a;
	a.useFunc();
	B b;
	b.useFunc(); // A::useFunc()被B::useFunc()隐藏
	b.A::useFunc();
	b.useFunc(2);

	b.print();
	b.A::print();
	b.print("jf");
}