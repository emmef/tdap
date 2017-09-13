/*
 * tdap/bounds.hpp
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

#ifndef TDAP_BOUNDS_HEADER_GUARD
#define TDAP_BOUNDS_HEADER_GUARD

#include <cstddef>
#include <limits>
#include <type_traits>

namespace tdap {

    /**
     * Defines validation and transformation of indices/positions in types.
     */
    template<class T>
    struct __RangeCheckTraits
    {
        /**
         * Returns whether the given index is valid, given the range-size.
         *
         * @param index The index to verify against the range-size.
         * @param range_size The range-size, for instance the buffer capacity or array length.
         * @return true if the index is valid.
         */
        static constexpr bool verify(size_t index, size_t range_size)
        {
            return T::__range_verify(index, range_size);
        }

        /**
         * Gets the transformed index, given the range-size. In most cases the
         * transformed value is equal to the index.
         *
         * @param index The index to transform
         * @param range_size The range-size, for instance the buffer capacity or array length.
         * @return the transformed index.
         */
        static constexpr bool transform(size_t index, size_t range_size)
        {
            return T::__range_transform(index, range_size);
        }
    };

    template<class T>
    static constexpr bool valid_range_check()
    {
        return std::is_base_of<__RangeCheckTraits<T>, T>::value;
    }

    /**
     * Defines that a data type that can be addressed by indices should check if
     * the index is smaller than the range - capacity or size - of the s
     */
    struct RangeCheckEnabled : public __RangeCheckTraits<RangeCheckEnabled>
    {
#ifdef TDAP_RANGE_CHECK_FORCE_DISABLE
        static constexpr bool __range_verify(size_t, size_t)
        { return true; }
#else

        static constexpr bool __range_verify(size_t index, size_t range_size)
        { return index < range_size; }

#endif

        static constexpr size_t __range_transform(size_t index, size_t)
        { return index; }
    };

    struct RangeCheckDisabled : public __RangeCheckTraits<RangeCheckDisabled>
    {
#ifdef TDAP_RANGE_CHECK_FORCE_ENABLE
        static constexpr bool __range_verify(size_t, size_t)
        { return true; }
#else

        static constexpr bool __range_verify(size_t index, size_t range_size)
        { return index < range_size; }

#endif

        static constexpr size_t __range_transform(size_t index, size_t)
        { return index; }
    };

    template<size_t SIZEOF>
    struct __Sized_Count
    {
        static_assert(SIZEOF > 0, "Type size must be positive");

        static constexpr size_t max()
        { return std::numeric_limits<size_t>::max() / SIZEOF; }

        static constexpr bool valid(size_t cnt)
        { return cnt <= max(); }

        static constexpr bool valid_positive(size_t cnt)
        { return cnt > 0 && valid(cnt); }

        /**
         * Returns the product of the counts if that product is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t product(size_t cnt1, size_t cnt2)
        {
            return cnt1 * cnt1 > 0 && max() / cnt1 >= cnt2 ? cnt1 * cnt2 : 0;
        }

        /**
         * Returns the product of the counts if that product is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t product(size_t cnt1, size_t cnt2, size_t cnt3)
        {
            return product(cnt1, product(cnt2, cnt3));
        }

        /**
         * Returns the product of the counts if that product is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t product(
                size_t cnt1, size_t cnt2, size_t cnt3, size_t cnt4)
        {
            return product(cnt1, product(cnt2, cnt3, cnt4));
        }

        /**
         * Returns the sum of the counts if that sum is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t sum(size_t cnt1, size_t cnt2)
        {
            return cnt1 <= max() && cnt2 <= max() && (max() - cnt1) >= cnt2 ? cnt1 + cnt2 : 0;
        }

        /**
         * Returns the sum of the counts if that sum is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t sum(size_t cnt1, size_t cnt2, size_t cnt3)
        {
            return sum(cnt1, sum(cnt2, cnt3));
        }

        /**
         * Returns the sum of the counts if that sum is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t sum(size_t cnt1, size_t cnt2, size_t cnt3, size_t cnt4)
        {
            return sum(cnt1, sum(cnt2, cnt3, cnt4));
        }

        /**
         * Returns whether the sum of the counts is less than or equal to max()
         */
        static constexpr size_t is_valid_sum(size_t cnt1, size_t cnt2)
        {
            return cnt1 <= max() && cnt2 <= max() && cnt1 + cnt2 <= max();
        }

        /**
         * Returns the sum of the counts if that sum is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t is_valid_sum(size_t cnt1, size_t cnt2, size_t cnt3)
        {
            return is_valid_sum(cnt1, cnt2) && is_valid_sum(cnt1 + cnt2, cnt3);
        }

        /**
         * Returns the sum of the counts if that sum is less than or equal
         * to max() and zero otherwise.
         */
        static constexpr size_t is_valid_sum(size_t cnt1, size_t cnt2, size_t cnt3, size_t cnt4)
        {
            return is_valid_sum(cnt1, cnt2) && is_valid_sum(cnt3, cnt4) && is_valid_sum(cnt1 + cnt2, cnt3 + cnt4);
        }

        /**
         * Returns the first value that is both equal or bigger than value and
         * a multiple of alignment.
         *
         * @param value The value to align.
         * @param alignment The value to align with.
         * @return The aligned value.
         */
        static constexpr size_t aligned_with(size_t value, size_t alignment)
        {
            return alignment != 0 ? value % alignment != 0 ? value + alignment - (value % alignment) : value : 0;
        }

    };

    template<typename T>
    struct Count : public __Sized_Count<sizeof(T)>
    {
    };


    template<typename SIZE_T, bool constExpr>
    struct __PowerOf2__Helper_FillBitsToRight
    {
    };

    template<typename SIZE_T>
    class __PowerOf2__Helper_FillBitsToRight<SIZE_T, true>
    {
        template<size_t N>
        static constexpr SIZE_T fillN(const SIZE_T n)
        {
            return N < 2 ? n : fillN<N / 2>(n) | (fillN<N / 2>(n) >> (N / 2));
        };
    public:
        static constexpr SIZE_T fill(const SIZE_T n)
        {
            return fillN<8 * sizeof(SIZE_T)>(n);
        };
    };

    template<typename SIZE_T>
    struct __PowerOf2__Helper_FillBitsToRight<SIZE_T, false>
    {
        static constexpr int BITS = sizeof(SIZE_T) * 8;
        static inline SIZE_T fill(const SIZE_T x)
        {
            SIZE_T n = x;
            int N = 1;
            for (int N = 1; N < BITS; N *= 2) {
                n |= (n >> N);
            }
            return n;
        }
    };

    template<bool constExpr, typename SIZE_T = size_t>
    class __PowerOf2_Helper : public __PowerOf2__Helper_FillBitsToRight<SIZE_T, constExpr>
    {
        using __PowerOf2__Helper_FillBitsToRight<SIZE_T, constExpr>::fill;

        static constexpr SIZE_T unchecked_aligned(SIZE_T value, SIZE_T alignment)
        {
            return (value | (alignment - 1)) + 1;
        }


    public:
        /**
         * Returns whether the value is a power of two minus one.
         */
        static constexpr bool minus_one(const SIZE_T value)
        {
            return fill(value) == value;
        }

        /**
         * Returns whether the value is a power of two.
         */
        static constexpr bool is(const SIZE_T value)
        {
            return value >= 2 ? minus_one(value - 1) : false;
        }

        /**
         * Returns value if it is a power of two or else the next power of two that is greater.
         */
        static constexpr SIZE_T next(const SIZE_T value)
        {
            return fill(value - 1) + 1;
        }

        /**
         * Returns value if it is a power of two or else the next power of two that is smaller.
         */
        static constexpr SIZE_T previous(const SIZE_T value)
        {
            return next(value / 2 + 1);
        }

        /**
         * Returns value if it is smaller than the power and else the power of two.
         */
        static constexpr SIZE_T within(const SIZE_T value, const SIZE_T powerOfTwo)
        {
            return (fill(value & ((powerOfTwo - 1) ^ -1)) | value) & (powerOfTwo - 1);
        }

        /**
         * Returns the value if it is aligned to power_of_two, the first higher
         * value that is aligned to power_of_two or zero if the provided power of two
         * is not actually a power of two.
         *
         * @param value Value to be aligned
         * @param power_of_two The power of two to align to
         * @return the aligned value
         */
        static constexpr SIZE_T aligned_with(const SIZE_T value, const SIZE_T power_of_two)
        {
            return is(power_of_two) ? unchecked_aligned(value, power_of_two) : 0;
        }

        /**
         * Returns the pointer value if it is aligned to power_of_two, the first higher
         * pointer value that is aligned to power_of_two or NULL if the provided power of two
         * is not actually a power of two.
         *
         * @param pointer Pointer to be aligned
         * @param power_of_two The power of two to align to
         * @return the aligned value
         */
        template<typename T>
        static constexpr T *ptr_aligned_with(T *pointer, const SIZE_T power_of_two)
        {
            return is(power_of_two) ? static_cast<T *>(unchecked_aligned(static_cast<SIZE_T>(pointer),
                                                                         power_of_two)) : 0;
        }

    };

    struct Power2 : public __PowerOf2_Helper<false>
    {
        using constant = __PowerOf2_Helper<true>;
    };

} /* End of name space tdap */

#endif /* TDAP_BOUNDS_HEADER_GUARD */