//
// Created by light on 19-11-5.
//

#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

void hello() {
    cout << "hello world\n" << endl;
    sleep(1);
}

int main() {
    // vector<thread> t(2, thread(hello)); //vector需要元素的复制构造函数可用，但thread把它delete了
    thread t[2] = {thread(hello), thread(hello)};
    // t[0].join(); //C++中结束main之前需要响应各个线程，否则return后会出现“无错terminal”
    // 需要注意的是线程对象执行了join后就不再joinable了，所以只能调用join一次。
    t[0].detach(); t[1].detach();
    return 0;
}