# Project-OpenCLe

## Development guide

There will be mainly 3 classes, Memory, Task and Core. Every class will use forward-declaration technique.

Memory: similar to std::shared_ptr, but manage in both host-side memory and device-side memory. A Memory could be either a real memory (either on host-side or device-side, but not both) or a place holder (i.e.). If a Memory is a place holder, then it should point to the Memory that comes from. There also should be private method that transfer memory between different devices (or host). Also the kernel assign number (used for decide copy the memory or not).

Task: stores kernel codes, parameter Memory (non-const and const) and have a method returns the place holder of non-const memory.

Core: have two threads, one for user interface and another for schedule the task.
