#ifndef TDAP_AVERAGE_HPP
#define TDAP_AVERAGE_HPP
/*
 * tdap/average.hpp
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
 *
 * The implementation of a moving average is straightforward for types without
 * precision loss at each calculation: keep the sum of N elements, add a new
 * sample and subtract the old sample, then return the sum divided by the window
 * size in samples. This is indeed what the PreciseWindowAverage class does.
 * With floating point errors, this approach does not work. This can be amended
 * by re-calculating the complete window average each time we add a new sample.
 * For large windows, this is computationally expensive. Instead, a modified
 * algorithm is used that lets each sample decay with a (very long) time
 * constant. Subtraction of old samples is corrected for this decay. The decay
 * time constant in number of samples is called the Error Mitigating Decay: EMD.
 * If the EMD approaches the window size, this puts relatively more weight on
 * recent samples. This makes the average less a true moving window average.
 * So the EMD should be as big as possible. However, imprecision in the sample
 * type also forces an upper boundary or the average looses correlation with the
 * input samples.
 */
#include <tdap/impl/average-helper.hpp>

namespace tdap::average
{
    using namespace std;
    using namespace tdap::boundaries;

    template<typename S, size_t SNR_BITS = 20, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO=10>
    class TrueFloatingPointWeightedMovingAverage
    {
        using History =
        helper::HistoryAndEmdForTrueFloatingPointMovingAverage
                <S, SNR_BITS, MIN_ERROR_DECAY_TO_WINDOW_RATIO>;
        using Window = helper::WindowForTrueFloatingPointMovingAverage<S>;

        History history;
        Window window;

        void optimiseForMaximumSamples()
        {
            if (history.optimiseForMaximumWindowSamples(window.windowSamples())) {
                window.setReadPtr();
            }
        }

    public:
        using Metrics =
                helper::HelperForMovingAverageMetricsForInaccurateTypes<
                        S, SNR_BITS, MIN_ERROR_DECAY_TO_WINDOW_RATIO>;
        TrueFloatingPointWeightedMovingAverage(
                const size_t maxWindowSize,
                const size_t emdSamples);

        void setAverage(const double average);

        void setWindowSize(const size_t windowSamples);

        void addInput(const double input);

        const S getAverage() const { return window.getAverage(); }

        const size_t getReadPtr() const { return window.getReadPtr(); }
        const size_t getWritePtr() const { return history.writePtr(); }
        const S getNextHistoryValue() const { return history.history()[window.getReadPtr()]; }
    };


    /**
     * Implements a true windowed average. This is obtained by adding a new
     * sample to a running average and subtracting the value of exactly the
     * window size in the past - kept in history.
     *
     * This algorithm is efficient and it is easy to combine an array of
     * different window sizes. However, the efficiency comes with an inherent
     * problem of addition/subtraction errors as a result of limited
     * floating-point precision. To mitigate this, both the running average and
     * all history values have an appropriate "natural decay" applied to them,
     * effectively zeroing vaues that are much older than the window size.
     *
     * This mitigating decay also suffers from imprecision and causes a
     * measurement "noise". As a rule of thumb, this noise should stay
     * approximately three orders of magnitude below average input.
     * In order to
     * @tparam S the type of samples used, normally "double"
     * @tparam MAX_SAMPLE_HISTORY the maximum sample history, determining the maximum RMS window size
     * @tparam MAX_RCS the maximum number of characteristic times in this array
     */
    template<typename S, size_t SNR_BITS = 20, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO=10>
    class TrueFloatingPointWeightedMovingAverageSet
    {
        using History =
        helper::HistoryAndEmdForTrueFloatingPointMovingAverage
                <S, SNR_BITS, MIN_ERROR_DECAY_TO_WINDOW_RATIO>;
        using Window =
        helper::ScaledWindowForTrueFloatingPointMovingAverage<S>;

        static constexpr size_t MINIMUM_TIME_CONSTANTS = 1;
        static constexpr size_t MAXIMUM_TIME_CONSTANTS = 32;
        static constexpr const char * TIME_CONSTANT_MESSAGE =
                "The (maximum) number of time-constants must lie between "
        TDAP_QUOTE(MINIMUM_TIME_CONSTANTS) " and "
        TDAP_QUOTE(MAXIMUM_TIME_CONSTANTS) ".";

        using Metrics =
        helper::HelperForMovingAverageMetricsForInaccurateTypes<
                S, SNR_BITS, MIN_ERROR_DECAY_TO_WINDOW_RATIO>;

        const size_t entries_;
        Window *entry_;
        size_t usedWindows_;
        History history_;

        static size_t validMaxTimeConstants(size_t constants);

        size_t checkWindowIndex(size_t index) const;

        void optimiseForMaximumSamples();

    public:

        TrueFloatingPointWeightedMovingAverageSet(
                size_t maxWindowSamples, size_t errorMitigatingTimeConstant, size_t maxTimeConstants, S average);

        size_t getMaxWindows() const { return entries_; }
        size_t getUsedWindows() const { return usedWindows_; }
        size_t getMaxWindowSamples() const { return history_.historySize(); }

        void setUsedWindows(size_t windows);

        void setWindowSizeAndScale(size_t index, size_t windowSamples, S scale);

        void setAverages(S average);

        S getAverage(size_t index) const;

        size_t getWindowSize(size_t index) const;

        S getWindowScale(size_t index) const;

        const S get() const { return history_.get(); }

        void addInput(S input);

        S addInputGetMax(S const input, S minimumValue);

        size_t getWritePtr() const { return history_.writePtr(); }

        size_t getReadPtr(size_t i) const { return entry_[i].getReadPtr(); }

        ~TrueFloatingPointWeightedMovingAverageSet();
    };

}

#ifndef TDAP_INCLUDE_NO_IMPLEMENTATION
#include <tdap/impl/average-impl.hpp>
#endif

}
#endif //TDAP_AVERAGE_HPP
