#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

/** we will have two versions of opencle::queue, by using
 * template meta programing, both of then need to achieve
 *  1. multithread-safe for fist thread call push(); second
 * thread call pop(), get()
 *  2. no mutex lock, no automic operation.
 *  3. the size of queue is per-determine at compile time,
 * SIZE is the number of T that can contian.
 *  4. notice: malloc/operator new has global mutex lock,
 * and should not be used after initialization.
 *  5. should provide a method called clear() to clean all
 * resources. Must be multi-thread-safe with one call clear(),
 * other threads call push(), pop(), get().
 *  4. queue is not copy / move constructable / assignable, and
 * you must explicitly delete those methods.
 */

/* assign to Brad Huang, due on Jan 1 0:00 AM (EST time). */

namespace opencle {
/* if T's ctor is no throw */
template <typename T, int SIZE> class queue final {
  public:
    queue();

    void push(T const &other);
    void push(T &&other);

    T pop();

    void clear();
};

/* general version */
template <typename T, int SIZE> class queue final {
  public:
    queue();

    void push(T const &other);
    void push(T &&other);

    T &get();

    void pop();

    void clear();
};

} // namespace opencle

#endif