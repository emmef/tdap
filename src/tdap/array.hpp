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

#ifndef TDAP_ARRAY_HPP
#define TDAP_ARRAY_HPP

#include <tdap/array_traits.hpp>

namespace tdap {

    template<typename T, size_t CAPACITY, bool check_range = true>
    class Array
            : public _FixedCapacityArrayTraits<T, check_range, CAPACITY, Array<T, CAPACITY, check_range>>
    {
        friend class _FixedCapacityArrayTraits<T, check_range, CAPACITY, Array<T, CAPACITY, check_range>>;

        static_assert(Count<T>::valid_positive(CAPACITY), "Array: Invalid capacity");

        T data_[CAPACITY];

        T &_trait_ref_mutable(size_t i)
        { return data_[i]; }

        const T &_trait_ref_immutable(size_t i) const
        { return data_[i]; }

        constexpr size_t _trait_range_size() const
        { return CAPACITY; }

    public:
        Array() {}
        Array(const T &fill_value) {
            fill(fill_value);
        }
        template<bool __check_range, class ...A>
        explicit Array(const _FixedCapacityArrayTraits<T, __check_range, CAPACITY, A...> &source)
        {
            copy(source);
        }
    };

}

#endif //TDAP_ARRAY_HPP
