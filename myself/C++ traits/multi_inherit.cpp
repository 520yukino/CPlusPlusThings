#include<iostream>
using namespace std;
//菱形继承需要处理基类的重复性问题，使用虚继承来使得中间类只会实例化1个基类而非2个，但同时子类的构造列表需要添加基类的构造，否则基类默认调用默认构造函数而非中间类指定的构造函数
//如果基类通过继承手段在子类中出现了多个，那这是有歧义的，比如C继承A和B，B继承A这种情况，C中含有2个A，那么在使用C对象中存在于A的非静态成员时编译器会不知道使用哪一个A而导致歧义；包括这里的菱形继承，如果不使用虚继承则也会出错
class Person {
public:
    Person(int x) {
        cout << "Person::Person(int) called" << endl;
    }
    Person() {
        cout << "Person::Person() called" << endl;
    }
    void f1() {
        cout << "Person::f1() called" << endl;
    }
};

class Faculty: virtual public Person {
public:
    Faculty(int x): Person(x) {
       cout << "Faculty::Faculty(int) called" << endl;
    }
};

class Student: virtual public Person {
public:
    Student(int x): Person(x) {
        cout << "Student::Student(int) called" << endl;
    }
};

class Hero: public Faculty, public Student {
public:
    Hero(int x): Student(x), Faculty(x)/* , Person(x) */ {
        cout << "TA::TA(int) called" << endl;
    }
};

int main()
{
    Hero h1(30);
    h1.f1();
}