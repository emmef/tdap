#ifndef TDAP_BOUNDARIES_HPP
#define TDAP_BOUNDARIES_HPP
/*
 * tdap/boundaries.hpp
 *
 * Part of Time-domain Audio Processing (TDAP)
 * Copyright (C) 2015-2019 Michel Fleur.
 * Source https://github.com/emmef/tdap
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

#include <tdap/impl/boundaries-helper.hpp>

namespace tdap::boundaries {

    template<typename V1, typename V2>
    using Comparison = helper::ComparisonHelper<
            V1, V2,
            helper::numeric_type_classification<V1>(),
            helper::numeric_type_classification<V2>()>;

    template<typename V, typename L, typename H>
    static constexpr bool is_between(V value, L minimum, H maximum)
    {
        return Comparison<V, L>::gte(value, minimum) &&
               Comparison<V, H>::lte(value, maximum);
    }

    template<typename V, typename L, typename H>
    static constexpr V force_between(V value, L minimum, H maximum)
    {
        return Comparison<V, L>::lt(value, minimum) ? minimum : Comparison<V,
                                                                           H>::gt(
                value, maximum) ? maximum : value;
    }

    template<typename V1, typename V2>
    static constexpr V1 minimum(const V1 &value1, const V2 &value2)
    {
        return Comparison<V1, V2>::lte(value1, value2) ? value1 : value2;
    }

    template<typename V1, typename V2>
    static constexpr V1 maximum(const V1 &value1, const V2 &value2)
    {
        return Comparison<V1, V2>::gte(value1, value2) ? value1 : value2;
    }

#if defined(TDAP_INDEX_POLICY_METHODS_CHECKED) && TDAP_INDEX_POLICY_METHODS_CHECKED < 1
    static constexpr bool defaultMethodIndexPolicy = false;
#else
    static constexpr bool defaultMethodIndexPolicy = true;
#endif

#if defined(TDAP_INDEX_POLICY_OPERATORS_CHECKED) && TDAP_INDEX_POLICY_OPERATORS_CHECKED < 1
    static constexpr bool defaultOperatorIndexPolicy = true;
#elif TDAP_INDEX_POLICY_OPERATORS_CHECKED == 0
    static constexpr bool defaultOperatorIndexPolicy = false;
#else
    static constexpr bool defaultOperatorIndexPolicy = true;
#endif

    struct IndexPolicy
    {
        static inline size_t force(size_t index, size_t size)
        {
            if (index < size) {
                return index;
            }
            throw std::out_of_range("IndexPolicy: Index out of range");
        }

        static inline size_t array(size_t index, size_t size)
        {
            return defaultOperatorIndexPolicy ? force(index, size) : index;
        }

        static inline size_t method(size_t index, size_t size)
        {
            return defaultMethodIndexPolicy ? force(index, size) : index;
        }

        struct NotGreater
        {
            static inline size_t force(size_t index, size_t high_value)
            {
                if (index <= high_value) {
                    return index;
                }
                throw std::out_of_range("Index out of range");
            }

            static inline size_t array(size_t index, size_t high_value)
            {
                return defaultOperatorIndexPolicy ? force(index, high_value)
                                                  : index;
            }

            static inline size_t method(size_t index, size_t high_value)
            {
                return defaultMethodIndexPolicy ? force(index, high_value)
                                                : index;
            }
        };
    };
}

#ifndef TDAP_INCLUDE_NO_IMPLEMENTATION
#include <tdap/impl/boundaries-helper.hpp>
#endif

#endif //TDAP_BOUNDARIES_HPP
