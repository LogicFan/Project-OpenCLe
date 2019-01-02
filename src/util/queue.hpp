#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

#include <cstddef>
#include <type_traits>

/** We will have two versions of opencle::queue, by using
 *      template meta programing, both need to:
 *  1. Be multithread-safe for first thread call push();
 *      second thread call pop(), get()
 *  2. Have no mutex lock, no atomic operation.
 *     Note: malloc/operator new has global mutex lock,
 *      and should not be used after initialization.
 *  3. The size of queue is per-determined at compile time,
 *      CAP is the maximum number of T that can contain.
 *  4. Provide a method called clear() to clean all resources.
 *      Must be multi-thread-safe with one call clear(),
 *      other threads call push(), pop(), get().
 *  5. Not be copy / move constructable / assignable, and
 *      corresponding methods explicitly deleted.
 */

// Assigned to Brad Huang.

namespace opencle {

template <typename T, size_t CAP>
class queue final {
    T _mem[];
    size_t _read;
    size_t _write;
    size_t _size;

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

    // Enable if T's constructor is_nothrow_copy_constructible
    std::conditional_t<std::is_nothrow_copy_constructible<T>::value, T, void> pop();

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
};

}  // namespace opencle

#endif