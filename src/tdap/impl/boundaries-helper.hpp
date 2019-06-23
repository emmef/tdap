#ifndef TDAP_BOUNDARIES_HELPER_HPP
#define TDAP_BOUNDARIES_HELPER_HPP
/*
 * tdap/boundaries-declare.hpp
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

#include <type_traits>
#include <cstddef>
#include <stdexcept>
#include <tdap/macros.hpp>

namespace tdap::boundaries::helper {

    template <typename V1, typename V2, int V1TYPE, int V2TYPE>
    struct ComparisonHelper;

    template<typename V>
    constexpr const int numeric_type_classification()
    {
        static_assert(std::is_floating_point<V>::value || std::is_integral<V>::value);
        return std::is_floating_point<V>::value ? 0 : std::is_signed<V>::value ? 1 : 2;
    }

    template <typename V1, typename V2>
    struct ComparisonHelperAutoWidenedTypes
    {
        static constexpr bool lt(const V1 &v1, const V2 &v2)
        { return v1 < v2; }

        static constexpr bool lte(const V1 &v1, const V2 &v2)
        { return v1 <= v2; }

        static constexpr bool gt(const V1 &v1, const V2 &v2)
        { return v1 > v2; }

        static constexpr bool gte(const V1 &v1, const V2 &v2)
        { return v1 >= v2; }
    };


    template <typename V1, typename V2, int V2TYPE>
    struct ComparisonHelper<V1, V2, 0, V2TYPE> :
            public ComparisonHelperAutoWidenedTypes<V1, V2>
    {
        static_assert(std::is_floating_point<V1>::value);
    };

    template <typename V1, typename V2>
    struct ComparisonHelper<V1, V2, 1, 1> :
            public ComparisonHelperAutoWidenedTypes<V1, V2>
    {
        static_assert(std::is_signed<V1>::value);
        static_assert(std::is_signed<V2>::value);
    };

    template <typename V1, typename V2>
    struct ComparisonHelper<V1, V2, 2, 2> :
            public ComparisonHelperAutoWidenedTypes<V1, V2>
    {
        static_assert(std::is_integral<V1>::value && !std::is_signed<V1>::value);
        static_assert(std::is_integral<V2>::value && !std::is_signed<V2>::value);
    };

    template <typename V1, typename V2>
    struct ComparisonHelper<V1, V2, 1, 2>
    {
        static_assert(std::is_integral<V1>::value && std::is_signed<V1>::value);
        static_assert(std::is_integral<V2>::value && !std::is_signed<V2>::value);

        static constexpr bool lt(const V1 &v1, const V2 &v2)
        { return v1 < 0 || static_cast<V2>(v1) < v2; }

        static constexpr bool lte(const V1 &v1, const V2 &v2)
        { return v1 < 0 || static_cast<V2>(v1) <= v2; }

        static constexpr bool gt(const V1 &v1, const V2 &v2)
        { return v1 >= 0 && static_cast<V2>(v1) > v2; }

        static constexpr bool gte(const V1 &v1, const V2 &v2)
        { return v1 >= 0 && static_cast<V2>(v1) >= v2; }

    };
    template <typename V1, typename V2>
    struct ComparisonHelper<V1, V2, 2, 1>
    {
        static_assert(std::is_integral<V1>::value && !std::is_signed<V1>::value);
        static_assert(std::is_integral<V2>::value && std::is_signed<V2>::value);

        static constexpr bool lt(const V1 &v1, const V2 &v2)
        { return v2 >= 0 && v1 < static_cast<V1>(v2); }

        static constexpr bool lte(const V1 &v1, const V2 &v2)
        { return v2 >= 0 && v1 <= static_cast<V1>(v2); }

        static constexpr bool gt(const V1 &v1, const V2 &v2)
        { return v2 < 0 || v1 > static_cast<V1>(v2); }

        static constexpr bool gte(const V1 &v1, const V2 &v2)
        { return v2 < 0 || v1 >= static_cast<V1>(v2); }
    };
}

#endif //TDAP_BOUNDARIES_HELPER_HPP
