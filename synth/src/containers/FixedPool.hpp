/*
 * Copyright (C) 2025 Jared Burton
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

#include <cstddef>
#include <functional>
#include <array>
#include <bitset>
#include <type_traits>

#include "modules/BaseModule.hpp"

template<typename T, size_t N>
class FixedPool {
    static_assert(std::is_base_of<BaseModule, T>::value, "FixedPool<T>: T must derive from Module");
private:
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N> storage_ ;
    std::bitset<N> inUse_ ;

public:
    template<typename... Args>
    void initializeAll(Args&&... args) {
        for (size_t i = 0; i < N; ++i) {
            void* place = static_cast<void*>(&storage_[i]);
            new (place) T(std::forward<Args>(args)...);
            inUse_.reset(i);
        }
    }

    T* allocate() {
        for (size_t i = 0; i < N; ++i) {
            if (!inUse_.test(i)) {
                inUse_.set(i);
                return reinterpret_cast<T*>(&storage_[i]);
            }
        }
        return nullptr; // Pool exhausted
    }

    void release(T* ptr) {
        size_t index = static_cast<size_t>(ptr - reinterpret_cast<T*>(&storage_[0]));
        if (index < N && inUse_.test(index)) {
            inUse_.reset(index);
        }
    }

    template <typename Func, typename... Args>
    void forEachActive(Func&& func, Args&&... args){
        for ( std::size_t i = 0; i < N; ++i ){
            if (inUse_.test(i)){
                T& obj = *reinterpret_cast<T*>(&storage_[i]);
                std::invoke(std::forward<Func>(func), obj, std::forward<Args>(args)...);
            }
        }
    }

    std::size_t countActiveVoices() const {
        return inUse_.count() ;
    }

    ~FixedPool() {
        for (size_t i = 0; i < N; ++i) {
            if (inUse_.test(i)) {
                reinterpret_cast<T*>(&storage_[i])->~T();
            }
        }
    }
};
