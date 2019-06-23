/*
 * tdap/array.hpp
 *
 * Part of TDAP: Time-domain Audio Processing library
 * Copyright (C) 2015-2017 Michel Fleur.
 * Source https://github.com/emmef/tdap
 * (Moved from https://bitbucket.org/emmef/tdap)
 * Email  tdap@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TDAP_BUFFER_HPP
#define TDAP_BUFFER_HPP

#include <tdap/array_traits.hpp>

namespace tdap {

    template<typename T, bool check_range = true>
    class Buffer
            : public _ArrayTraits<T, check_range, true, Buffer<T, check_range>>
    {
        friend class _ArrayTraits<T, check_range, true, Buffer<T, check_range>>;

        T *data_;
        size_t capacity_;

        size_t _trait_range_size() const
        {
            return capacity_;
        }

        T &_trait_ref_mutable(size_t i)
        { return data_[i]; }

        const T &_trait_ref_immutable(size_t i) const
        { return data_[i]; }

        static size_t valid_capacity(size_t cap, bool init)
        {
            if (Count<T>::valid(cap)) {
                return cap;
            }
            throw std::invalid_argument(
                    init ? "Buffer::init: invalid capacity" : "Buffer::change_capacity: invalid capacity");
        }

        void free()
        {
            if (data_) {
                delete[]data_;
                data_ = nullptr;
            }
            capacity_ = 0;
        }

    public:
        Buffer(size_t initial_capacity) : data_(new T[valid_capacity(initial_capacity, true)]),
                                          capacity_(initial_capacity)
        {

        }

        Buffer() : Buffer(0)
        {}

        Buffer(size_t initial_capacity, const T &fill_value) : data_(new T[valid_capacity(initial_capacity, true)]),
                                                               capacity_(initial_capacity)
        {
            fill(fill_value);
        }

        template<bool __check_range>
        explicit Buffer(const Buffer<T, __check_range> &source) :
                data_(new T[valid_capacity(source.capacity_, true)]), // check as source can have capacity 0
                capacity_(source.capacity_)
        {
            copy(source);
        }

        template<bool __check_range, bool has_trivial_addressing, class ...A>
        explicit Buffer(const _ArrayTraits<T, __check_range, has_trivial_addressing, A...> &source) :
                data_(new T[valid_capacity(source.range_size(), true)]), // check as source can have capacity 0
                capacity_(source.range_size())
        {
            copy(source);
        }

        template<bool __check_range>
        Buffer(Buffer<T, __check_range> &&source) :
                data_(source.data_), // check as source can have capacity 0
                capacity_(source.capacity_)
        {
            source.free();
        }

        size_t capacity() const
        { return capacity_; }

        bool change_capacity(size_t new_capacity, bool do_throw = true)
        {
            if (do_throw) {
                valid_capacity(new_capacity, false);
            }
            else if (!Count<T>::valid_positive(new_capacity)) {
                return false;
            }
            T *new_data = new T[new_capacity];
            std::memmove(new_data, data_, std::min(capacity_, new_capacity));
            delete[] data_;
            data_ = new_data;
            capacity_ = new_capacity;
            return true;
        }

        ~Buffer()
        {
            free();
        }
    };
}
#endif //TDAP_BUFFER_HPP
