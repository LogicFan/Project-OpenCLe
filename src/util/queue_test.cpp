#include "queue.hpp"
#include <iostream>

using namespace opencle;
using std::cout;
using std::endl;
using std::thread;

queue<int, 4> theQueue;

void f1() {
    theQueue.push(4);
    theQueue.push(5);
    theQueue.push(6);
    theQueue.push(7);
    theQueue.push(8);
}

void f2() {
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
}

int main() {
    theQueue.push(0);
    theQueue.push(1);
    theQueue.push(2);
    theQueue.push(3);

    thread t1(f1);
    thread t2(f2);

    t1.join();
    t2.join();
}