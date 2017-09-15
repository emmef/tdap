/*
 * tdap/delay.hpp
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

#ifndef TDAP_DELAY_HPP
#define TDAP_DELAY_HPP

#include <tdap/buffer.hpp>
#include <tdap/array.hpp>

namespace tdap {

    enum class DelayType
    {
        FromOneToWrapSize, FromZeroToWrapSizeMinusOne
    };

    template<typename T, class Array>
    const T delay_read_then_write(const T &input, size_t delay, size_t wrap_size, size_t &read, Array &array)
    {
        read %= wrap_size;
        T result = array[read];
        array[(read + delay) % wrap_size] = input;
        read++;

        return result;
    }

    template<typename T, class Array>
    const T delay_write_then_read(const T &input, size_t delay, size_t wrap_size, size_t &read, Array &array)
    {
        read %= wrap_size;
        array[(read + delay) % wrap_size] = input;
        return array[read++];
    }

    bool is_delay_valid_read_then_write(size_t delay, size_t wrap_size)
    {
        return delay > 0 && delay <= wrap_size;
    }

    bool is_delay_valid_write_then_read(size_t delay, size_t wrap_size)
    {
        return delay >= 0 && delay < wrap_size;
    }

    template<typename T, class Array, size_t N>
    class BaseDelay
    {
        static_assert(std::is_arithmetic<T>::value, "Delay is designed for arithmetic types only");

        size_t read_ = 0;

    public:
        constexpr size_t get_delay()
        { return N; }

        const T get_and_set(const T &input)
        {
            return delay_read_then_write(input, 0, N, read_, static_cast<Array *>(this)->__trait_get_array());
        }

        void zero()
        { static_cast<Array *>(this)->__trait_get_array().zero(); }
    };

    template<typename T, class Array>
    class BaseDelay<T, Array, 0>
    {
        static_assert(std::is_arithmetic<T>::value, "Delay is designed for arithmetic types only");

        using DelayFunction = const T(*)(const T &input, size_t delay, size_t wrap_size, size_t &read,
                                         Array &array);
        Array array_;
        size_t delay_;
        size_t read_;
        size_t wrap_;
        DelayFunction delay_function_;
        DelayFunction rd_then_wr = delay_read_then_write<T, Array>;
        DelayFunction wr_then_rd = delay_write_then_read<T, Array>;

    protected:
        Array &array()
        {
            return array_;
        }

        const Array &array() const
        {
            return array_;
        }

    public:
        template<typename ...A>
        BaseDelay(A... args) : array_(args...)
        {
            set_delay(0);
        }

        template<typename ...A>
        BaseDelay(size_t delay, A... args) : array_(args...)
        {
            if (!set_delay(delay)) {
                throw std::invalid_argument("Invalid initial delay");
            }
        }

        T get_and_set(const T &input)
        {
            return delay_function_(input, delay_, wrap_, read_, array_);
        }

        size_t get_delay() const
        { return delay_function_ == rd_then_wr ? array_.range_size() : delay_; }


        void zero()
        {
            array_.zero();
            read_ = 0;
        }

        bool set_delay(size_t new_delay)
        {
            return set_delay_in_frames(new_delay, 1);
        }

        bool set_delay_in_frames(size_t new_delay_in_frames, size_t frame_size)
        {
            size_t new_delay = new_delay_in_frames * frame_size;
            size_t capacity = array_.range_size();
            if (new_delay < capacity) {
                delay_function_ = wr_then_rd;
                delay_ = new_delay;
            }
            else if (new_delay == capacity) {
                delay_function_ = rd_then_wr;
                delay_ = 0;
            }
            else {
                return false;
            }

            wrap_ = new_delay != 0 ? new_delay : 1;
            this->zero();
            return true;
        }

        size_t capacity() const
        {
            return array().range_size();
        }
    };

    template<typename T>
    class BufferDelay : public BaseDelay<T, tdap::Buffer<T, false>, 0>
    {
        using BaseDelay<T, tdap::Buffer<T, false>, 0>::array;
    public:
        template<typename ...A>
        BufferDelay(A... args) : BaseDelay<T, tdap::Buffer<T, false>, 0>(args...)
        {}

        template<typename ...A>
        BufferDelay(size_t delay, A... args) : BaseDelay<T, tdap::Buffer<T, false>, 0>(delay, delay, args...)
        {}

        template<typename ...A>
        BufferDelay(size_t delay_in_frames, size_t frame_size, A... args) : BaseDelay<T, tdap::Buffer<T, false>, 0>(
                Count<T>::validated_product(delay_in_frames, frame_size), delay_in_frames * frame_size, args...)
        {}

        bool set_delay_in_frames(size_t new_delay_in_frames, size_t frame_size)
        {
            size_t new_delay = new_delay_in_frames * frame_size;
            if (array().capacity() < new_delay) {
                if (!array().change_capacity(new_delay)) {
                    std::cerr <<
                              "BufferDelay::set_delay_in_frames(new_delay_in_frames=" << new_delay_in_frames <<
                              ",frame_size=" << frame_size <<
                              ") [capacity=" << array().capacity() <<
                              "]: reallocated failed" << std::endl;
                    return false;
                }
            }
            return BaseDelay<T, tdap::Buffer<T, false>, 0>::set_delay_in_frames(new_delay_in_frames, frame_size);
        }

        bool change_capacity(size_t new_capacity)
        {
            return
                    Count<T>::valid_positive(new_capacity) &&
                    new_capacity >= BaseDelay<T, tdap::Buffer<T, false>, 0>::get_delay() &&
                    array().change_capacity(new_capacity, false);
        }
    };

    template<typename T, size_t MAX_DELAY>
    class FixedSizeArrayDelay : public BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, MAX_DELAY>
    {
    public:
        template<typename ...A>
        FixedSizeArrayDelay() : BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, MAX_DELAY>()
        {}

        template<typename ...A>
        FixedSizeArrayDelay(size_t delay, A... args) : BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, MAX_DELAY>(delay)
        {}

        template<typename ...A>
        FixedSizeArrayDelay(size_t delay_in_frames, size_t frame_size, A... args)
                : BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, MAX_DELAY>(
                delay_in_frames, frame_size)
        {}
    };

    template<typename T, size_t MAX_DELAY>
    class ArrayDelay : public BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, 0>
    {
    public:
        template<typename ...A>
        ArrayDelay() : BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, 0>()
        {}

        template<typename ...A>
        ArrayDelay(size_t delay, A... args) : BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, 0>(delay)
        {}

        template<typename ...A>
        ArrayDelay(size_t delay_in_frames, size_t frame_size, A... args)
                : BaseDelay<T, tdap::Array<T, MAX_DELAY, false>, 0>(
                delay_in_frames, frame_size)
        {}
    };


}

#endif //TDAP_DELAY_HPP
