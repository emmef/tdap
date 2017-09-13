/*
 * tdap/array_traits.hpp
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

#ifndef TDAP_ARRAY_TRAITS_HPP_HPP
#define TDAP_ARRAY_TRAITS_HPP_HPP

#include <tdap/bounds.hpp>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace tdap {

    template<typename T, class RangeCheck, class Implementation, bool has_trivial_addressing>
    class _ArrayTraits;

    template<typename T, class Sub, bool isScalar>
    struct __Scalar_ArrayTraits
    {
    };

    template<typename T, class Sub>
    struct __Scalar_ArrayTraits<T, Sub, false>
    {
        /**
         * Sets all elements to given value;
         */
        void fill(const T &value)
        {
            for (size_t i = 0; i < static_cast<const Sub *>(this)->range_size(); i++) {
                static_cast<Sub *>(this)->ref(i) = value;
            }
        }

    };

    template<typename T, class Implementation>
    struct __Scalar_ArrayTraits<T, Implementation, true>
    {
        /**
         * Zeroes all elements
         */
        void zero()
        {
            static_assert(std::is_scalar<T>::value, "Can only zero scalar type");
            if (Implementation::has_trivial_adressing()) {
                std::memset(&mutable_cast().ref(0), 0, sizeof(T) * immutable_cast().range_size());
            }
            else {
                for (size_t i = 0; i < immutable_cast().range_size(); i++) {
                    mutable_cast().operator[](i) = static_cast<T>(0);
                }
            }
        }

        /**
         * Sets all elements to given value;
         */
        void fill(const T &value)
        {
            if (Implementation::has_trivial_adressing() && value == static_cast<T>(0)) {
                std::memset(mutable_cast().ref_data(), 0, immutable_cast().range_size() * sizeof(T));
            }
            else {
                for (size_t i = 0; i < immutable_cast().range_size(); i++) {
                    mutable_cast().ref(i) = value;
                }
            }
        }

    private:
        Implementation &mutable_cast()
        { return *static_cast<Implementation *>(this); }

        const Implementation &immutable_cast() const
        { return *static_cast<const Implementation *>(this); }

    };

    template<typename T, class Sub, bool isTrivial_Addressing>
    struct __Trivial_Addressing_ArrayTraits
    {
    };

    template<typename T, class Implementation>
    struct __Trivial_Addressing_ArrayTraits<T, Implementation, false>
    {

        template<typename ...A>
        void copy(const _ArrayTraits<T, A...> &source)
        {
            if (source.range_size() != immutable_cast().range_size()) {
                throw std::invalid_argument("ArrayTraits::copy(): source has different size");
            }

            for (size_t i = 0; i < immutable_cast().range_size(); i++) {
                immutable_cast().operator[](i) = source.operator[](i);
            }
        }

        template<typename ...A>
        void copy(size_t offset, const _ArrayTraits<T, A...> &source, size_t sourceOffset, size_t length)
        {
            size_t end = immutable_cast().check_copy_parameters(offset, source, sourceOffset, length);

            for (size_t src = sourceOffset, dst = offset; dst < end; dst++, src++) {
                mutable_cast().operator[](dst) = source.operator[](src);
            }
        }

        void move(size_t destination, size_t source, size_t length)
        {
            if (source == destination) {
                return;
            }
            size_t src_end = immutable_cast().check_move_parameters(source, length, destination);
            if (source > destination || src_end <= destination) {
                for (size_t s = source, d = destination; s < src_end; s++, d++) {
                    mutable_cast().ref(d) = immutable_cast().get(s);
                }
            }
            else {
                for (size_t s = src_end - 1, d = destination + length; --d >= destination; s--) {
                    mutable_cast().ref(d) = immutable_cast().get(s);
                }
            }
        }

    private:
        Implementation &mutable_cast()
        { return *static_cast<Implementation *>(this); }

        const Implementation &immutable_cast() const
        { return *static_cast<const Implementation *>(this); }

    };

    template<typename T, class Implementation>
    struct __Trivial_Addressing_ArrayTraits<T, Implementation, true>
    {

        T *data_ref()
        { return &static_cast<Implementation *>(this)->_trait_ref_mutable(0); }

        const T *data_get() const
        { return &static_cast<const Implementation *>(this)->_trait_ref_immutable(0); }

        template<typename ...A>
        void copy(const _ArrayTraits<T, A...> &source)
        {
            if (source.range_size() != immutable_cast().range_size()) {
                throw std::invalid_argument("ArrayTraits::copy(): source has different size");
            }

            if (_ArrayTraits<T, A...>::has_trivial_adressing()) {
                const void *src = static_cast<const void *>(source.data_get());
                void *dst = data_ref();
                std::memmove(dst, src, sizeof(T) * immutable_cast().range_size());
            }
            else {
                T *dst = data_ref();
                for (size_t i = 0; i < immutable_cast().range_size(); i++) {
                    dst[i] = source.operator[](i);
                }
            }
        }

        template<typename ...A>
        void copy(size_t offset, const _ArrayTraits<T, A...> &source, size_t sourceOffset, size_t length)
        {
            size_t end = immutable_cast().check_copy_parameters(offset, source, sourceOffset, length);

            if (_ArrayTraits<T, A...>::has_trivial_adressing()) {
                const void *src = static_cast<const void *>(source.data_get() + sourceOffset);
                void *dst = static_cast<void *>(data_ref() + offset);
                std::memmove(dst, src, sizeof(T) * length);
            }
            else {
                T *ptr = data_ref();
                for (size_t src = sourceOffset, dst = offset; dst < end; dst++, src++) {
                    ptr[dst] = source.operator[](src);
                }
            }
        }

        void move(size_t destination, size_t source, size_t length)
        {
            if (source == destination) {
                return;
            }
            size_t src_end = immutable_cast().check_move_parameters(source, length, destination);
            if (Implementation::has_trivial_adressing()) {
                std::memmove(mutable_cast().operator+(destination), immutable_cast().operator+(source),
                             sizeof(T) * length);
                return;
            }
        }

    private:
        Implementation &mutable_cast()
        { return *static_cast<Implementation *>(this); }

        const Implementation &immutable_cast() const
        { return *static_cast<const Implementation *>(this); }

    };


    template<typename T, class RangeCheck, class Implementation, bool has_trivial_addressing>
    class _ArrayTraits :
            public __Scalar_ArrayTraits<T, _ArrayTraits<T, RangeCheck, Implementation, has_trivial_addressing>, std::is_scalar<T>::value>,
            public __Trivial_Addressing_ArrayTraits<T, _ArrayTraits<T, RangeCheck, Implementation, has_trivial_addressing>, has_trivial_addressing>
    {
        static_assert(std::is_trivially_copyable<T>::value, "Value type must be trivially to copy");
        static_assert(valid_range_check<RangeCheck>(), "Invalid range check class");
        friend class __Scalar_ArrayTraits<T, _ArrayTraits<T, RangeCheck, Implementation, has_trivial_addressing>, std::is_scalar<T>::value>;

        Implementation &mutable_cast()
        { return *static_cast<Implementation *>(this); }

        const Implementation &immutable_cast() const
        { return *static_cast<const Implementation *>(this); }

        T &mutable_ref(size_t idx)
        {
            if (RangeCheck::verify(idx, immutable_cast()._trait_range_size())) {
                return mutable_cast()._trait_ref_mutable(
                        RangeCheck::transform(idx, immutable_cast()._trait_range_size()));
            }
            throw std::out_of_range("_ArrayTraits::mutable_ref()");
        }

        const T &immutable_ref(size_t idx) const
        {
            if (RangeCheck::verify(idx, immutable_cast()._trait_range_size())) {
                return immutable_cast()._trait_ref_immutable(
                        RangeCheck::transform(idx, immutable_cast()._trait_range_size()));
            }
            throw std::out_of_range("_ArrayTraits::immutable_ref()");
        }

        template<typename ...A>
        size_t
        check_copy_parameters(size_t offset, const _ArrayTraits<T, A...> &source, size_t sourceOffset, size_t length)
        {
            if (!Count<T>::is_valid_sum(offset, length)) {
                throw std::invalid_argument("ArrayTraits::copy(): offset and length too big (numeric)");
            }
            size_t end = offset + length;
            if (end > range_size()) {
                throw std::invalid_argument("ArrayTraits::copy(): offset and length too big (size)");
            }
            if (!Count<T>::is_valid_sum(sourceOffset, length) || sourceOffset + length > source.range_size()) {
                throw std::invalid_argument("ArrayTraits::copy(): source offset and length too big");
            }
            return end;
        }

        size_t check_move_parameters(size_t source, size_t length, size_t destination)
        {
            if (length == 0) {
                return source;
            }
            size_t end = Count<T>::sum(source, length);
            if (end == 0 || end > range_size()) {
                throw std::invalid_argument("ArrayTraits::copy(): source offset and length too big (numeric)");
            }
            size_t dest_end = Count<T>::sum(destination, length);
            if (dest_end == 0 || dest_end > range_size()) {
                throw std::invalid_argument("ArrayTraits::copy(): destination offset and length too big (numeric)");
            }
            return end;
        }

    public:
        constexpr size_t range_size() const
        { return immutable_cast()._trait_range_size(); }

        T &ref(size_t i)
        { return mutable_ref(i); }

        T &operator[](size_t i)
        { return ref(i); }

        T *operator+(size_t i)
        { return &ref(i); }

        const T &get(size_t i) const
        { return immutable_ref(i); }

        const T &operator[](size_t i) const
        { return get(i); }

        const T *operator+(size_t i) const
        { return &get(i); }

        /**
         * Returns whether elements are stored consecutively in memory.
         */
        static constexpr bool has_trivial_adressing()
        { return has_trivial_addressing; }
    };

    template<typename T, size_t CAPACITY, class Implementation, class RangeCheck>
    class _FixedCapacityArrayTraits
            : public _ArrayTraits<T, RangeCheck, _FixedCapacityArrayTraits<T, CAPACITY, Implementation, RangeCheck>, true>
    {
        friend class _ArrayTraits<T, RangeCheck, _FixedCapacityArrayTraits<T, CAPACITY, Implementation, RangeCheck>, true>;

        static_assert(tdap::Count<T>::valid_positive(CAPACITY), "Invalid capacity for fixed-capacity array");

        constexpr size_t _trait_range_size() const
        {
            return std::min(CAPACITY, static_cast<const Implementation *>(this)->_trait_range_size());
        }

        T &_trait_ref_mutable(size_t i)
        { return static_cast<Implementation *>(this)->_trait_ref_mutable(i); }

        const T &_trait_ref_immutable(size_t i) const
        { return static_cast<const Implementation *>(this)->_trait_ref_immutable(i); }

    public:
        constexpr size_t capacity() const
        { return CAPACITY; }
    };

}

#endif //TDAP_ARRAY_TRAITS_HPP_HPP
