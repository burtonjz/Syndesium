#ifndef __ATOMIC_FLOAT_HPP_
#define __ATOMIC_FLOAT_HPP_

#include <atomic>

struct AtomicFloat {
    std::atomic<float> value;

    AtomicFloat() : value(0.0f) {}
    AtomicFloat(float v) : value(v) {}

    AtomicFloat(const AtomicFloat& other) {
        value.store(other.value.load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
    }

    AtomicFloat& operator=(const AtomicFloat& other) {
        if (this != &other) {
            value.store(other.value.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
        }
        return *this;
    }

    float get() const { return value.load(std::memory_order_relaxed); }
    void set(float v) { value.store(v, std::memory_order_relaxed); }

    operator float() const { return get(); }  // Allows implicit float usage
};


#endif // __ATOMIC_FLOAT_HPP_