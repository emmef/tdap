#ifndef TDAP_FILTER_HPP
#define TDAP_FILTER_HPP
/*
 * tdap/filter.hpp
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

namespace tdap::filter
{
    template <typename T>
    struct Filter
    {
        static_assert(std::is_arithmetic<T>::value);

        virtual T filter(const T input) { return input; }

        virtual void reset() { }

        virtual ~Filter() = default;

        static Filter &identity()
        {
            static Filter IDENTITY;
            return IDENTITY;
        }

        /**
         * Returns the length of the impulse response. This is defined as the
         * first window where the total energy (RMS) of the impulse response,
         * as weighted through a weighting function, is below a certain threshold,
         * relative to the total energy of the impulse response so far.
         *
         * The measurement is limited to a maximum number of samples: if the
         * energy condition is not met by that number of samples, the
         * measurement fails.
         *
         * @param filter The filter to measure the impulse-response-length for.
         * @param maxLength Maximum length to measure until
         * @param windowSize The size of the window.
         * @param threshold The maximum relative energy that the window can have.
         * @param weighting The weighting used (before RMS) to determine energy.
         *      Thi defaults to the identity filter.
         * @return The effective length of the impulse response or -1 on failure.
         */
        static ssize_t getEffectiveImpulseResponseLength(Filter &filter,
                                                         size_t maxLength,
                                                         double threshold,
                                                         size_t windowSize,
                                                         Filter &weighting = identity());
    };

    template <typename T>
    struct ChannelFilter
    {
        static_assert(std::is_arithmetic<T>::value);

        virtual size_t getChannels() const;

        virtual T filter(const size_t channel, const T input) { return input; }

        virtual void reset() { }

        virtual ~ChannelFilter() = default;

        /**
         * Returns a Filter that uses channel 0 of this ChannelFilter and
         * ignores resets.
         *
         * @return the Filter.
         */
        Filter<T> single();
    };

}

#ifndef TDAP_INCLUDE_NO_IMPLEMENTATION
#include <tdap/impl/filter-impl.hpp>
#endif

#endif //TDAP_FILTER_HPP
