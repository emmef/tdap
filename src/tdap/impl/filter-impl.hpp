#ifndef TDAP_FILTER_IMPL_HPP
#define TDAP_FILTER_IMPL_HPP
/*
 * tdap/filter-declare.hpp
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

#include <algorithm>
#include <tdap/average.hpp>

namespace tdap::filter
{
    using namespace tdap::average;

    template<typename T>
    Filter<T> ChannelFilter<T>::single()
    {
        class SingleFilter : public Filter<T>
        {
            ChannelFilter<T> * const flt;
        public:
            SingleFilter(ChannelFilter * const original) : flt(original) {}

            virtual T filter(const T input) { return flt->filter(0, input);  }
        };
        return SingleFilter(this);
    }

    template<typename T>
    ssize_t Filter<T>::getEffectiveImpulseResponseLength(Filter &filter,
                                                         size_t maxLength,
                                                         double threshold,
                                                         size_t windowSize,
                                                         Filter &weighting)
    {
        using Average = TrueFloatingPointWeightedMovingAverage<T>;
        const double usedThreshold = std::clamp(threshold, 1e-24, 1.0);
        size_t usedMaximumLength = maximum(2, maxLength);
        size_t usedWindowSize = force_between(windowSize, 1, usedMaximumLength - 1);
        size_t usedMinumLength = usedWindowSize;
        Average average(usedWindowSize, Average::Metrics::MAX_ERR_MITIGATING_DECAY_SAMPLES);
        average.setWindowSize(usedWindowSize);
        average.setAverage(0);
        T input = std::is_floating_point<T>::value ? static_cast<T>(1) : std::numeric_limits<T>::max();
        double squareThreshold = usedThreshold * usedThreshold;

        double totalSum = 0.0;

        for (size_t sample = 0; sample < usedMaximumLength; sample++) {
            double value = sample == 0 ? input : 0;
            double filtered = filter.filter(value);
            double weighted = weighting.filter(filtered);
            double square = weighted * weighted;

            totalSum += square;
            average.addInput(square);
            double windowAverage = average.getAverage() *  windowSize;

            if (
                    sample >= usedMinumLength &&
                    windowAverage < totalSum * squareThreshold) {
                // TODO Check interpretation: identity has length 0 or 1?
                return sample + 1 - usedWindowSize;
            }
        }

        return -1;
    }

}

#endif //TDAP_FILTER_IMPL_HPP
