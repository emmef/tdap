/*
 * tdap/circular.hpp
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

#ifndef TDAP_CIRCULAR_HPP
#define TDAP_CIRCULAR_HPP

#include <tdap/buffer.hpp>
#include <tdap/array.hpp>

namespace tdap {

    template<typename T, class Implementation>
    class _CircularTraits
    {
        size_t read_ = 0;
        size_t count_ = 0;

        void move_writes(size_t new_size)
        {
            if (count_ > 0) {
                if (read_ + count_ > new_size) {
                    size_t src_end = size() + read_;
                    size_t dst_start = size() + count_ - 1;
                    for (size_t src = src_end + count_, dst = dst_start; --src >= src_end; dst--) {
                        ref(dst % size()) = get(src % size());
                    }
                }
            }
            static_cast<Implementation *>(this)->_trait_set_size(new_size);
        }

        T &ref(size_t i)
        { return static_cast<Implementation *>(this)->_trait_ref(i); }

        const T &get(size_t i) const
        { return static_cast<const Implementation *>(this)->_trait_get(i); }

    public:
        const size_t capacity() const
        { return static_cast<const Implementation *>(this)->_trait_capacity(); }

        size_t count() const
        { return count_; }

        const size_t size() const
        { return static_cast<const Implementation *>(this)->_trait_size(); }

        void zero()
        { static_cast<Implementation *>(this)->_trait_zero(); }

        bool set_size(size_t new_size)
        {
            if (new_size < 1 || new_size < count_ || new_size > capacity()) {
                return false;
            }
            move_writes(new_size);
            return true;
        }

        bool set_size_read_up(size_t new_size)
        {
            if (new_size < 1 || new_size > capacity()) {
                return false;
            }
            if (new_size < count_) {
                size_t reads = count_ - new_size;
                count_ = new_size;
                read_ += reads;
                read_ %= size();
            }
            move_writes(new_size);
            return true;
        }

        bool set_size_write_off(size_t new_size)
        {
            if (new_size < 1 || new_size > capacity()) {
                return false;
            }
            if (new_size < count_) {
                count_ = new_size;
            }
            move_writes(new_size);
            return true;
        }

        bool write(const T &input)
        {
            if (count_ < size()) {
                ref((read_ + count_) % size()) = input;
                count_++;
                return true;
            }
            return false;
        }

        bool read(T &output)
        {
            if (count_) {
                output = get(read_);
                read_++;
                read_ %= size();
                count_--;
                return true;
            }
            return false;
        }

        bool write_and_read(const T &input, T &output)
        {
            if (count_ < size()) {
                ref((read_ + count_) % size()) = input;
                output = get(read_);
                read_++;
                read_ %= size();
                return true;
            }
            return false;
        }

        bool set_count(size_t new_count)
        {
            if (new_count >= size()) {
                return false;
            }
            if (new_count <= count_) {
                count_ = new_count;
                return true;
            }
            while (count_ < new_count) {
                ref((read_ + count_) % size()) = 0;
                count_++;
            }
            return true;
        }

        void reset()
        {
            read_ = count_ = 0;
            zero();
        }
    };

    template<typename T, size_t CAPACITY>
    class FixedCapCircularBuffer
            : public _CircularTraits<T, FixedCapCircularBuffer<T, CAPACITY>>
    {
        friend class _CircularTraits<T, FixedCapCircularBuffer<T, CAPACITY>>;

        Array<T, CAPACITY, false> data_;
        size_t size_;

        T &_trait_ref(size_t i)
        { return data_[i]; }

        const T &_trait_get(size_t i) const
        { return data_[i]; }

        const size_t _trait_capacity() const
        { return data_.capacity(); }

        const size_t _trait_size() const
        { return size_; }

        void _trait_set_size(size_t new_size)
        { size_ = new_size; }

        void _trait_zero()
        { data_.zero(); }

    public:
        FixedCapCircularBuffer() : size_(data_.capacity())
        {}
    };

    template<typename T>
    class CircularBuffer
            : public _CircularTraits<T, CircularBuffer<T>>
    {
        friend class _CircularTraits<T, CircularBuffer<T>>;

        Buffer<T, false> data_;
        size_t size_;

        T &_trait_ref(size_t i)
        { return data_[i]; }

        const T &_trait_get(size_t i) const
        { return data_[i]; }

        const size_t _trait_capacity() const
        { return data_.capacity(); }

        const size_t _trait_size() const
        { return size_; }

        void _trait_set_size(size_t new_size)
        { size_ = new_size; }

        void _trait_zero()
        { data_.zero(); }

    public:
        CircularBuffer(size_t initial_capacity) : data_(initial_capacity), size_(data_.capacity())
        {}

        CircularBuffer(size_t initial_capacity, const T &fill_value) : data_(initial_capacity, fill_value),
                                                                       size_(data_.capacity())
        {}

        CircularBuffer(size_t initial_capacity, bool zero_all) : data_(initial_capacity, zero_all), size_(data_.capacity())
        {}

        bool change_capacity(size_t new_capacity, bool do_throw = true)
        {
            if (new_capacity < size_) {
                return false;
            }
            return data_.change_capacity(new_capacity, do_throw);
        }

        bool change_capacity_read_up(size_t new_capacity, bool do_throw = true)
        {
            if (new_capacity < size_) {
                if (!_CircularTraits<T, CircularBuffer<T>>::set_size_read_up(new_capacity)) {
                    return false;
                }
            }
            return data_.change_capacity(new_capacity, do_throw);
        }

        bool change_capacity_write_off(size_t new_capacity, bool do_throw = true)
        {
            if (new_capacity < size_) {
                if (!_CircularTraits<T, CircularBuffer<T>>::set_size_write_off(new_capacity)) {
                    return false;
                }
            }
            return data_.change_capacity(new_capacity, do_throw);
        }

    };


}
#endif //TDAP_CIRCULAR_HPP
