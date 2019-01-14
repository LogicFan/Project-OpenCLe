#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

#include "readerwriterqueue/readerwriterqueue.h"

/** We will have two versions of opencle::queue, by using
 *      template meta programing, both need to:
 *  1. Be multithread-safe for first thread calling push();
 *      second thread calling pop() or peek().
 *  2. Have no mutex lock, no atomic operation.
 *     Note: malloc/operator new has global mutex lock,
 *      and should not be used after initialization.
 *  3. The size of queue is per-determined at compile time,
 *      CAP is the maximum number of T that can contain.
 *  4. Not be copy constructable / assignable, and
 *      corresponding methods be explicitly deleted.
 */

// Assigned to Brad Huang.

using moodycamel::ReaderWriterQueue;

namespace opencle {

template <typename T, size_t CAP>
class queue final {
    ReaderWriterQueue<T> rwQueue;

   public:
    ////// CONSTRUCTOR AND DESTRUCTOR //////

    //// Constructor ////
    // @NOT_THREAD_SAFE
    queue(): rwQueue(ReaderWriterQueue<T>(CAP)) {}

    //// Destructor ////
    // @NOT_THREAD_SAFE
    ~queue() {}


    // Delete copy constructor/assignment
    queue(const queue& other) = delete;
    queue& operator=(const queue& other) = delete;


    // Allow move copy constructor/assignment

    //// Move constructor ////
    // @NOT_THREAD_SAFE
    queue(queue&& other): rwQueue(ReaderWriterQueue<T>(std::move(other))) {};

    //// Move Assignment ////
    // @NOT_THREAD_SAFE
    queue& operator=(queue&& other) {
        rwQueue = std::move(other.rwQueue);
        return *this;
    };

    ////// MODIFIERS //////

    //// Copy Enqueue ////
    void push(T const& value) {
        if (!rwQueue.try_enqueue(value)) {
            throw std::out_of_range("Push failed: size exceeds capacity, or memory allocation failed.");
        }
    }

    //// Move Enqueue ////
    void push(T&& value) {
        if (!rwQueue.try_enqueue(std::move(value))) {
            throw std::out_of_range("Push failed: size exceeds capacity, or memory allocation failed.");
        }
    }


#ifdef MOODYCAMEL_HAS_EMPLACE
    //// Emplace ////
    // @Variadic Template: available for either a non-MS compiler or VS >= 2013
    // @return bool, true if operation succeeds, false otherwise
    template<typename... Args>
    bool emplace(Args&&... args) { return rwQueue.try_emplace(std::forward<Args>(args)...); }
#endif


    //// Dequeue ////
    // @precondition Requires T to be move assignable
    // @mutation if operation succeeds, moves front to result using operator=
    template<typename U>
    void pop(U& result) {
        if (!rwQueue.try_dequeue(result)) {
            throw std::out_of_range("Pop failed: queue is empty");
        }
    }


    //// Clear ////
    // @NOT_THREAD_SAFE
    void clear() {
        rwQueue = ReaderWriterQueue<T>(CAP);
    }


    ////// ELEMENT ACCESS //////

    //// Peek ////
    // @return T&, the element next to be removed
    T& front() { return *(rwQueue.peek()); }

    //// Const Peek ////
    // @return T const&, the element next to be removed
    T const& front() const { return *(rwQueue.peek()); }


    ////// CAPACITY //////

    //// Empty Approximation ////
    // @return bool, an approximation if a queue is empty
    // Safe to call from both the producer and consumer threads
    bool empty() const { return rwQueue.size_approx() == 0; }

    //// Size Approximation ////
    // @return bool, the approximate number of items currently in the queue
	// Safe to call from both the producer and consumer threads
    size_t size() const { return rwQueue.size_approx(); }
};

}  // namespace opencle

#endif