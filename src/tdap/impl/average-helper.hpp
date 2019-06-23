#ifndef TDAP_AVERAGE_HELPER_HPP
#define TDAP_AVERAGE_HELPER_HPP
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
 */

#include <limits>
#include <tdap/boundaries.hpp>

namespace tdap::average::helper {

    using namespace tdap::boundaries;
    using namespace std;

    template<
            typename S, size_t SNR_BITS = 20, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO = 10>
    struct HelperForMovingAverageMetricsForInaccurateTypes
    {
        static_assert(
                std::is_floating_point<S>::value,
                "Sample type must be floating point");

        static constexpr size_t MIN_SNR_BITS = 4;
        static constexpr size_t MAX_SNR_BITS = 44;
        static_assert(
                SNR_BITS >= MIN_SNR_BITS && SNR_BITS <= MAX_SNR_BITS,
                "Number of signal-noise-ratio in bits must lie between"
        TDAP_QUOTE(MIN_SNR_BITS)
        " and "
        TDAP_QUOTE(MAX_SNR_BITS)
        ".");

        static constexpr size_t MIN_MIN_ERROR_DECAY_TO_WINDOW_RATIO = 1;
        static constexpr size_t MAX_MIN_ERROR_DECAY_TO_WINDOW_RATIO = 1000;
        static_assert(
                is_between(
                        MIN_ERROR_DECAY_TO_WINDOW_RATIO,
                        MIN_MIN_ERROR_DECAY_TO_WINDOW_RATIO,
                        MAX_MIN_ERROR_DECAY_TO_WINDOW_RATIO
                ),
                "Minimum error decay to window size ratio must lie between "
        TDAP_QUOTE(MIN_MIN_ERROR_DECAY_TO_WINDOW_RATIO)
        " and "
        TDAP_QUOTE(MAX_MIN_ERROR_DECAY_TO_WINDOW_RATIO));

        static constexpr size_t MAX_ERR_MITIGATING_DECAY_SAMPLES =
                minimum(
                        0.01 / numeric_limits<S>::epsilon(),
                        numeric_limits<size_t>::max());
        static constexpr size_t MAX_WINDOWS_SIZE_BOUNDARY =
                MAX_ERR_MITIGATING_DECAY_SAMPLES /
                MIN_ERROR_DECAY_TO_WINDOW_RATIO;
        static constexpr const char *
        getErrorMitigaticDecayLimitExceededMessage();
        static constexpr size_t MIN_MAX_WINDOW_SAMPLES = 64;
        static constexpr size_t MAX_MAX_WINDOW_SAMPLES = minimum(
                1.0 / ((static_cast<size_t>(1) << SNR_BITS) *
                       std::numeric_limits<S>::epsilon()),
                MAX_WINDOWS_SIZE_BOUNDARY);
        static constexpr size_t MIN_ERR_MITIGATING_DECAY_SAMPLES =
                MIN_ERROR_DECAY_TO_WINDOW_RATIO * MIN_MAX_WINDOW_SAMPLES;

        static constexpr const char * getWindowSizeOutOfBoundsMessage();

        static constexpr const char * getErrorMitigatingDecaySamplesOutOfRangeMessage();

        static bool isValidWindowSizeInSamples(size_t samples);
        static size_t validWindowSizeInSamples(size_t samples);
        static bool isValidErrorMitigatingDecaySamples(size_t samples);

        static size_t validErrorMitigatingDecaySamples(size_t samples);
    };

    template<typename S>
    class BaseHistoryAndEmdForTrueFloatingPointMovingAverage
    {
        const size_t historySamples_;
        S * const history_;
        const size_t emdSamples_;
        const S emdFactor_;
        size_t historyEndPtr_;
        size_t writePtr_ = 0;

    protected:
        BaseHistoryAndEmdForTrueFloatingPointMovingAverage(
                const size_t historySamples, const size_t emdSamples);
        inline void setNextPtr(size_t &ptr) const;

    public:

        size_t historySize() const { return historySamples_; }
        size_t emdSamples() const { return emdSamples_; }
        size_t writePtr() const { return writePtr_; }
        size_t maxWindowSamples() const { return historyEndPtr_ + 1; }
        S emdFactor() const { return emdFactor_; }

        inline size_t getRelative(size_t delta) const;
        const S getHistoryValue(size_t &readPtr) const;
        const S get(size_t index) const;
        const S get() const;
        const S operator[](size_t index) const;
        void set(size_t index, S value);
        void write(S value);
        S &operator[](size_t index);
        void fillWithAverage(const S average);
        const S * const history() const { return history_; }
        S * const history() { return history_; }

        bool optimiseForMaximumWindowSamples(size_t samples);

        ~BaseHistoryAndEmdForTrueFloatingPointMovingAverage();
    };

    template<typename S>
    class WindowForTrueFloatingPointMovingAverage
    {
        const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S> * history_ = nullptr;
        size_t windowSamples_ = 1;
        S inputFactor_ = 1;
        S historyFactor_ = 1;
        size_t readPtr_ = 1;
        S average_ = 0;

    public:
        WindowForTrueFloatingPointMovingAverage() {}
        WindowForTrueFloatingPointMovingAverage(
                const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S> *history);

        void setOwner(const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S> *history);

        bool isOwnedBy(const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S> *owner) const;

        S getAverage() const { return average_; }

        size_t windowSamples() const { return windowSamples_; }

        size_t getReadPtr() const { return readPtr_; }

        const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S> * owner() const { return history_; }

        void setAverage(S average);
        void setWindowSamples(size_t windowSamples);
        void setReadPtr();

        void addInput(S input);
    };

    template<typename S>
    class ScaledWindowForTrueFloatingPointMovingAverage :
            public WindowForTrueFloatingPointMovingAverage<S>
    {
        using Super = WindowForTrueFloatingPointMovingAverage<S>;
        S scale_ = 1;
    public:
        ScaledWindowForTrueFloatingPointMovingAverage() { }

        ScaledWindowForTrueFloatingPointMovingAverage(
                const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S> &history);

        S setScale(S scale);

        const S scale() const { return scale_; }

        void setWindowSamplesAndScale(size_t windowSamples, S scale);

        const S getAverage() const { return scale_ * Super::getAverage(); }

        void setOutput(S outputValue);
    };
    template<
            typename S, size_t SNR_BITS = 20,
            size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO = 10>
    class HistoryAndEmdForTrueFloatingPointMovingAverage :
            public BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>
    {
        using Metrics_ = HelperForMovingAverageMetricsForInaccurateTypes
                <S, SNR_BITS, MIN_ERROR_DECAY_TO_WINDOW_RATIO>;
        using Super = BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>;

        static size_t validWindowSize(size_t emdSamples, size_t windowSize);

    public:
        HistoryAndEmdForTrueFloatingPointMovingAverage(
                const size_t historySamples, const size_t emdSamples);
    };

}

#endif //TDAP_AVERAGE_HELPER_HPP
