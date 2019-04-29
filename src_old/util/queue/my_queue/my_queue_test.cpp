#include "my_queue.hpp"
#include <iostream>
#include <queue>

using namespace opencle;
using std::cout;
using std::endl;
using std::thread;

queue<int, 4> theQueue;

void f1(int n) {
    theQueue.push(n + 4);
    theQueue.push(n + 5);
    // theQueue.push(n + 6);
    // theQueue.push(n + 7);
    // theQueue.push(n + 8);
}

void f2() {
    cout << "FRONT: " << theQueue.front() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << "FRONT: " << theQueue.front() << endl;
    cout << theQueue.pop() << endl;
    cout << theQueue.pop() << endl;
    cout << "FRONT: " << theQueue.front() << endl;
    cout << theQueue.pop() << endl;
    cout << "FRONT: " << theQueue.front() << endl;
    // cout << theQueue.pop() << endl;
    // cout << theQueue.pop() << endl;
    // cout << "FRONT: " << theQueue.front() << endl;
    // cout << theQueue.pop() << endl;
    // cout << theQueue.pop() << endl;
}

int main() {
    theQueue.push(0);
    theQueue.push(1);
    theQueue.push(2);
    theQueue.push(3);

    thread t2(f2);
    thread t1(f1, 0);
    // thread t4(f2);
    // thread t3(f1, 1000);

    t1.join();
    t2.join();
    // t3.join();
    // t4.join();
}