/*
 * Copyright (C) 2026 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ATOMIC_DOUBLE_HPP_
#define ATOMIC_DOUBLE_HPP_

#include <atomic>

struct AtomicDouble {
    std::atomic<double> value;

    AtomicDouble() : value(0.0f) {}
    AtomicDouble(double v) : value(v) {}

    AtomicDouble(const AtomicDouble& other) {
        value.store(other.value.load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
    }

    AtomicDouble& operator=(const AtomicDouble& other) {
        if (this != &other) {
            value.store(other.value.load(std::memory_order_relaxed),
                        std::memory_order_relaxed);
        }
        return *this;
    }

    double get() const { return value.load(std::memory_order_relaxed); }
    void set(double v) { value.store(v, std::memory_order_relaxed); }

    operator double() const { return get(); }  // Allows implicit double usage
};


#endif // ATOMIC_DOUBLE_HPP_