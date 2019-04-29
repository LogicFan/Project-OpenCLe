#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

// #define UTIL_QUEUE_DEBUG
// #define UTIL_QUEUE_PRIORITY_FIX

#include <chrono>
#include <cstddef>
#include <mutex>
#include <thread>
#include <type_traits>

#ifdef UTIL_QUEUE_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

/** We will have two versions of opencle::queue, by using
 *      template meta programing, both need to:
 *  1. Be multithread-safe for first thread call push();
 *      second thread call pop(), front(), back(), operator[],
 *      at.
 *  2. Have no mutex lock, no atomic operation.
 *     NOTE: This implementation contains mutex lock for the
 *      moment. Will improve in the future.
 *     Note: malloc/operator new has global mutex lock,
 *      and should not be used after initialization.
 *  3. The size of queue is per-determined at compile time,
 *      CAP is the maximum number of T that can contain.
 *  4. Provide a method called clear() to clean all resources.
 *      Must be multi-thread-safe with one call clear(),
 *      other threads call push(), pop(), front(), back(),
 *      operator[], at.
 *  5. Not be copy / move constructable / assignable, and
 *      corresponding methods explicitly deleted.
 */

// Assigned to Brad Huang.

using std::conditional_t;
using std::enable_if_t;
using std::is_nothrow_copy_constructible;
using std::mutex;

namespace opencle {

template <typename T, size_t CAP>
class queue final {
    struct dummy {};

    T* _mem;
    size_t _read;
    size_t _write;
    size_t _size;

    bool _valid[CAP];

    mutex _m_mutex;
    mutex _n_mutex;
    mutex _l_mutex;

   public:
    ////// CONSTRUCTOR AND DESTRUCTOR //////
    queue();
    ~queue();

    // Delete copy/move constructor/assignments
    queue(const queue& other) = delete;
    queue(queue&& other) = delete;
    queue& operator=(const queue& other) = delete;
    queue& operator=(queue&& other) = delete;

    ////// MODIFIERS //////
    void push(T const& value);
    void push(T&& value);

    // return T if constructor is_nothrow_copy_constructible, else return void
    conditional_t<is_nothrow_copy_constructible<T>::value, T, void> pop() {
        return pop_top(dummy());
    }

    void clear();

    ////// ELEMENT ACCESS //////
    T& front();
    T const& front() const;
    T& back();
    T const& back() const;
    T& operator[](size_t pos);
    T const& operator[](size_t pos) const;
    T& at(size_t pos);
    T const& at(size_t pos) const;

    ////// CAPACITY //////
    bool empty() const;
    size_t size() const;

   private:
    inline size_t mod_addr(size_t addr) { return addr % CAP; }

    // Prioritized mutex locking mechanism implemented by triple mutex
    // The triple mutex guarantees that high- and low-priority locks are locked
    //   alternately, with a high lock at first.
    void low_lock();
    void low_unlock();
    void high_lock();
    void high_unlock();

    T pop_top(enable_if_t<is_nothrow_copy_constructible<T>::value, dummy> dum);
    // void pop_top(enable_if_t<!is_nothrow_copy_constructible<T>::value, dummy> dum);
};

////// DEFAULT CONSTRUCTOR //////
template <typename T, size_t CAP>
queue<T, CAP>::queue() : _mem(static_cast<T*>(operator new(CAP * sizeof(T)))) {}

////// DESTRUCTOR //////
template <typename T, size_t CAP>
queue<T, CAP>::~queue() {
    clear();
    operator delete(_mem);
}

////// COPY PUSH //////
template <typename T, size_t CAP>
void queue<T, CAP>::push(T const& value) {
    low_lock();

#ifdef UTIL_QUEUE_DEBUG
    cout << "- Trying to push lvalue " << value << endl;
#endif

#ifndef UTIL_QUEUE_PRIORITY_FIX
    if (_size < CAP) {
        _mem[_write] = value;
        _write = mod_addr(_write + 1);
        ++_size;

#ifdef UTIL_QUEUE_DEBUG
        cout << "- Pushed." << endl;
#endif

    } else {
        throw std::out_of_range("Push failed: size exceeds capacity.");
    }
#endif

    low_unlock();
}

////// MOVE PUSH //////
template <typename T, size_t CAP>
void queue<T, CAP>::push(T&& value) {
    low_lock();

#ifdef UTIL_QUEUE_DEBUG
    cout << "- Trying to push rvalue " << value << endl;
#endif

#ifndef UTIL_QUEUE_PRIORITY_FIX
    if (_size < CAP) {
        _mem[_write] = std::move(value);
        _write = mod_addr(_write + 1);
        ++_size;

#ifdef UTIL_QUEUE_DEBUG
        cout << "- Pushed." << endl;
#endif

    } else {
        std::cout << "Failed: Pushing " << value << std::endl;
        throw std::out_of_range("Push failed: size exceeds capacity");
    }
#endif

    low_unlock();
}

////// POP //////
template <typename T, size_t CAP>
T queue<T, CAP>::pop_top(enable_if_t<is_nothrow_copy_constructible<T>::value, dummy> dum) {
    high_lock();

#ifdef UTIL_QUEUE_DEBUG
    cout << "- Trying to pop. " << endl;
#endif

#ifndef UTIL_QUEUE_PRIORITY_FIX
    if (_size > 0) {
        T* ret = new T(_mem[_read]);
        _mem[_read].~T();
        _read = mod_addr(_read + 1);
        --_size;

#ifdef UTIL_QUEUE_DEBUG
        cout << "- Popped " << *ret << endl;
#endif

        high_unlock();

        return *ret;
    } else {
        high_unlock();
        throw std::out_of_range("Pop failed: queue is empty");
    }
#else
    high_unlock();
#endif
}

// template <typename T, size_t CAP>
// void queue<T, CAP>::pop_top(enable_if_t<!is_nothrow_copy_constructible<T>::value, dummy> dum)
// {
//     high_lock();

// #ifdef UTIL_QUEUE_DEBUG
//     cout << "- Trying to pop. " << endl;
// #endif

// #ifdef UTIL_QUEUE_PRIORITY_FIX
//     if (_size > 0) {
//         _mem[_read].~T();
//         _read = mod_addr(_read + 1);
//         --_size;

// #ifdef UTIL_QUEUE_DEBUG
//         cout << "- Popped. " << endl;
// #endif

//         high_unlock();
//     } else {
//         high_unlock();
//         throw std::out_of_range("Pop failed: queue is empty");
//     }
// #else
//     high_unlock();
// #endif
// }

////// CLEAR //////
template <typename T, size_t CAP>
void queue<T, CAP>::clear() {
    low_lock();
    for (size_t i = 0; i < CAP; ++i) {
        if (_valid[i]) {
            (_mem[i]).~T();
        }
    }
    low_unlock();
}

////// FRONT //////
template <typename T, size_t CAP>
T& queue<T, CAP>::front() {
    return _mem[_read];
}

template <typename T, size_t CAP>
T const& queue<T, CAP>::front() const {
    return _mem[_read];
}

////// BACK //////
template <typename T, size_t CAP>
T& queue<T, CAP>::back() {
    return _mem[_write];
}

template <typename T, size_t CAP>
T const& queue<T, CAP>::back() const {
    return _mem[_write];
}

////// RAMDOM ACCESS //////
template <typename T, size_t CAP>
T& queue<T, CAP>::operator[](size_t pos) {
    return _mem[mod_addr(pos + _read)];
}

template <typename T, size_t CAP>
T const& queue<T, CAP>::operator[](size_t pos) const {
    return _mem[mod_addr(pos + _read)];
}

template <typename T, size_t CAP>
T& queue<T, CAP>::at(size_t pos) {
    return _mem[mod_addr(pos + _write)];
}

template <typename T, size_t CAP>
T const& queue<T, CAP>::at(size_t pos) const {
    return _mem[mod_addr(pos + _write)];
}

////// CAPACITY //////
template <typename T, size_t CAP>
bool queue<T, CAP>::empty() const {
    return _size == 0;
}

template <typename T, size_t CAP>
size_t queue<T, CAP>::size() const {
    return _size;
}

template <typename T, size_t CAP>
void queue<T, CAP>::low_lock() {
    _l_mutex.lock();
    _n_mutex.lock();
    _m_mutex.lock();
    _n_mutex.unlock();
}

template <typename T, size_t CAP>
void queue<T, CAP>::low_unlock() {
    _m_mutex.unlock();
    _l_mutex.unlock();
}

template <typename T, size_t CAP>
void queue<T, CAP>::high_lock() {
    _n_mutex.lock();
    _m_mutex.lock();
    _n_mutex.unlock();
}

template <typename T, size_t CAP>
void queue<T, CAP>::high_unlock() {
    _m_mutex.unlock();
}

}  // namespace opencle

#endif