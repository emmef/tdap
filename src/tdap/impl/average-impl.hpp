#ifndef TDAP_AVERAGE_IMPL_HPP
#define TDAP_AVERAGE_IMPL_HPP
/*
 * tdap/average-declare.hpp
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


#include <stdexcept>
#include <cmath>

namespace tdap::average::helper
{
    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    bool HelperForMovingAverageMetricsForInaccurateTypes<S, SNR_BITS,
                                                         MIN_ERROR_DECAY_TO_WINDOW_RATIO>::isValidWindowSizeInSamples(
            size_t samples)
    {
        return is_between(
                samples,
                MIN_MAX_WINDOW_SAMPLES,
                MAX_MAX_WINDOW_SAMPLES);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    size_t HelperForMovingAverageMetricsForInaccurateTypes<S, SNR_BITS,
                                                           MIN_ERROR_DECAY_TO_WINDOW_RATIO>::validWindowSizeInSamples(
            size_t samples)
    {
        if (isValidWindowSizeInSamples(samples)) {
            return samples;
        }
        throw std::invalid_argument(getWindowSizeOutOfBoundsMessage());
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    bool HelperForMovingAverageMetricsForInaccurateTypes<S, SNR_BITS,
                                                         MIN_ERROR_DECAY_TO_WINDOW_RATIO>::isValidErrorMitigatingDecaySamples(
            size_t samples)
    {
        return is_between(
                samples,
                MIN_ERR_MITIGATING_DECAY_SAMPLES,
                MAX_ERR_MITIGATING_DECAY_SAMPLES);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    size_t HelperForMovingAverageMetricsForInaccurateTypes<S, SNR_BITS,
                                                           MIN_ERROR_DECAY_TO_WINDOW_RATIO>::validErrorMitigatingDecaySamples(
            size_t samples)
    {
        if (isValidErrorMitigatingDecaySamples(samples)) {
            return samples;
        }
        throw std::invalid_argument(
                getErrorMitigatingDecaySamplesOutOfRangeMessage());
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    constexpr const char *
    HelperForMovingAverageMetricsForInaccurateTypes<S, SNR_BITS,
                                                    MIN_ERROR_DECAY_TO_WINDOW_RATIO>::getWindowSizeOutOfBoundsMessage()
    {
        return "RMS window size in samples must lie between "
        TDAP_QUOTE(MIN_MAX_WINDOW_SAMPLES)
        " and "
        TDAP_QUOTE(MAX_MAX_WINDOW_SAMPLES)
        " for minimum of "
        TDAP_QUOTE(SNR_BITS)
        " bits of signal to error-noise ratio and sample type "
        TDAP_QUOTE(typename S);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    constexpr const char *
    HelperForMovingAverageMetricsForInaccurateTypes<S, SNR_BITS,
                                                    MIN_ERROR_DECAY_TO_WINDOW_RATIO>::getErrorMitigatingDecaySamplesOutOfRangeMessage()
    {
        return
                "Error mitigating decay samples must lie between "
        TDAP_QUOTE(MIN_ERR_MITIGATING_DECAY_SAMPLES)
        " and "
        TDAP_QUOTE(MAX_ERR_MITIGATING_DECAY_SAMPLES)
        " for sample type "
        TDAP_QUOTE(typename S) ".";
    }


    template<typename S>
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S>::BaseHistoryAndEmdForTrueFloatingPointMovingAverage(
            const size_t historySamples, const size_t emdSamples)
            :
            historySamples_(historySamples),
            history_(new S[historySamples]),
            emdSamples_(emdSamples),
            emdFactor_(exp( -1.0 / emdSamples)),
            historyEndPtr_(historySamples - 1),
            writePtr_(0)
    {}

    template<typename S>
    void BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::setNextPtr(
            size_t &ptr) const
    {
        if (ptr > 0) {
            ptr--;
        }
        else
            ptr = historyEndPtr_;
    }

    template<typename S>
    size_t
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::getRelative(
            size_t delta) const
    {
        return (writePtr_ + delta) % (historyEndPtr_ + 1);
    }

    template<typename S>
    const S
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::getHistoryValue(
            size_t &readPtr) const
    {
        S result = history_[readPtr];
        setNextPtr(readPtr);
        return result;
    }

    template<typename S>
    const S BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::get(
            size_t index) const
    {
        return history_[IndexPolicy::NotGreater::method(index, historyEndPtr_)];
    }

    template<typename S>
    const S
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::get() const
    {
        return get(writePtr_);
    }

    template<typename S>
    const S
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::operator[](
            size_t index) const
    {
        return history_[IndexPolicy::NotGreater::array(index, historyEndPtr_)];
    }

    template<typename S>
    void
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::set(size_t index,
                                                               S value)
    {
        history_[IndexPolicy::NotGreater::method(index, historyEndPtr_)] = value;
    }

    template<typename S>
    void
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::write(S value)
    {
        history_[writePtr_] = value;
        setNextPtr(writePtr_);
    }

    template<typename S>
    S &BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::operator[](
            size_t index)
    {
        return history_[IndexPolicy::NotGreater::array(index, historyEndPtr_)];
    }

    template<typename S>
    bool BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S>::optimiseForMaximumWindowSamples(size_t samples)
    {
        size_t newHistoryEnd =
                force_between(samples, 4, maxWindowSamples()) - 1;
        if (newHistoryEnd != historyEndPtr_) {
            historyEndPtr_ = newHistoryEnd;
            return true;
        }
        return false;
    }

    template<typename S>
    void
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<S>::fillWithAverage(
            const S average)
    {
        for (size_t i = 0; i <= historyEndPtr_; i++) {
            history_[i] = average;
        }
    }

    template<typename S>
    BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S>::~BaseHistoryAndEmdForTrueFloatingPointMovingAverage()
    {
        delete[] history_;
    }



    template<typename S>
    WindowForTrueFloatingPointMovingAverage<
            S>::WindowForTrueFloatingPointMovingAverage(
            const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S> *history)
            :
            history_(history)
    {
    }

    template<typename S>
    void WindowForTrueFloatingPointMovingAverage<S>::setOwner(
            const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S> *history)
    {
        if (history_ == nullptr) {
            history_ = history;
            return;
        }
        throw std::runtime_error("Window already owned by other history");
    }

    template<typename S>
    bool WindowForTrueFloatingPointMovingAverage<S>::isOwnedBy(
            const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S> *owner) const
    {
        bool b = owner == history_;
        return b;
    }

    template<typename S>
    void WindowForTrueFloatingPointMovingAverage<S>::setAverage(S average)
    {
        average_ = average;
    }

    template<typename S>
    void WindowForTrueFloatingPointMovingAverage<S>::setWindowSamples(
            size_t windowSamples)
    {
        if (history_ == nullptr) {
            throw std::runtime_error("WindowForTrueFloatingPointMovingAverage::setWindowSamples(): window not related to history data");
        }
        if (!is_between(windowSamples, 1, history_->maxWindowSamples())) {
            throw std::runtime_error("WindowForTrueFloatingPointMovingAverage: window samples must lie between 1 and history's maximum size");
        }
        windowSamples_ = windowSamples;
        const double unscaledHistoryDecayFactor =
                exp(-1.0 * this->windowSamples_ / history_->emdSamples());
        inputFactor_ = (1.0 - history_->emdFactor()) / (1.0 - unscaledHistoryDecayFactor);
        historyFactor_ = inputFactor_ * unscaledHistoryDecayFactor;
        setReadPtr();
    }

    template<typename S>
    void WindowForTrueFloatingPointMovingAverage<S>::setReadPtr()
    {
        if (windowSamples_ <= history_->maxWindowSamples()) {
            readPtr_ = history_->getRelative(windowSamples_);
            return;
        }
        throw std::runtime_error("RMS window size cannot be bigger than buffer");
    }

    template<typename S>
    void WindowForTrueFloatingPointMovingAverage<S>::addInput(S input)
    {
        S history = history_->getHistoryValue(readPtr_);
        average_ =
                history_->emdFactor() * average_ +
                inputFactor_ * input -
                historyFactor_ * history;
    }


    template<typename S>
    ScaledWindowForTrueFloatingPointMovingAverage<
            S>::ScaledWindowForTrueFloatingPointMovingAverage(
            const BaseHistoryAndEmdForTrueFloatingPointMovingAverage<
            S> &history)
            :
            Super(&history)
    {
    }

    template<typename S>
    S ScaledWindowForTrueFloatingPointMovingAverage<S>::setScale(S scale)
    {
        if (fabs(scale) < 1e-12) {
            scale_ = 0.0;
        }
        else if (scale > 1e12) {
            scale_ = scale;
        }
        else if (scale < -1e12) {
            scale_ = -1e12;
        }
        else {
            scale_ = scale;
        }
        return scale;
    }

    template<typename S>
    void ScaledWindowForTrueFloatingPointMovingAverage<
            S>::setWindowSamplesAndScale(size_t windowSamples, S scale)
    {
        Super::setWindowSamples(windowSamples);
        setScale(scale);
    }

    template<typename S>
    void ScaledWindowForTrueFloatingPointMovingAverage<S>::setOutput(
            S outputValue)
    {
        Super::setAverage(scale_ != 0.0 ? outputValue / scale_ : outputValue);
    }


    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    HistoryAndEmdForTrueFloatingPointMovingAverage<S, SNR_BITS,
                                                   MIN_ERROR_DECAY_TO_WINDOW_RATIO>::HistoryAndEmdForTrueFloatingPointMovingAverage(
            const size_t historySamples, const size_t emdSamples) :
            Super(validWindowSize(Metrics_::validErrorMitigatingDecaySamples(emdSamples), historySamples), emdSamples)
    {}

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    size_t HistoryAndEmdForTrueFloatingPointMovingAverage<S, SNR_BITS,
                                                          MIN_ERROR_DECAY_TO_WINDOW_RATIO>::validWindowSize(
            size_t emdSamples, size_t windowSize)
    {
        if (Metrics_::validWindowSizeInSamples(windowSize) < emdSamples / Metrics_::MIN_MIN_ERROR_DECAY_TO_WINDOW_RATIO) {
            return windowSize;
        }
        throw std::invalid_argument("Invalid combination of window size and ratio between that and error mitigating decay samples.");
    }
}

namespace tdap::average {


    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    TrueFloatingPointWeightedMovingAverage<S, SNR_BITS,
                                           MIN_ERROR_DECAY_TO_WINDOW_RATIO>::TrueFloatingPointWeightedMovingAverage(
            const size_t maxWindowSize, const size_t emdSamples)
            :
            history(maxWindowSize, emdSamples),
            window(&history)
    {
        window.setWindowSamples(maxWindowSize);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverage<S, SNR_BITS,
                                                MIN_ERROR_DECAY_TO_WINDOW_RATIO>::setAverage(
            const double average)
    {
        window.setAverage(average);
        history.fillWithAverage(average);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverage<S, SNR_BITS,
                                                MIN_ERROR_DECAY_TO_WINDOW_RATIO>::setWindowSize(
            const size_t windowSamples)
    {
        window.setWindowSamples(windowSamples);
        optimiseForMaximumSamples();
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverage<S, SNR_BITS,
                                                MIN_ERROR_DECAY_TO_WINDOW_RATIO>::addInput(
            const double input)
    {
        window.addInput(input);
        history.write(input);
    }




    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    size_t TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                     MIN_ERROR_DECAY_TO_WINDOW_RATIO>::validMaxTimeConstants(
            size_t constants)
    {
        if (is_between(constants, MINIMUM_TIME_CONSTANTS, MAXIMUM_TIME_CONSTANTS)) {
            return constants;
        }
        throw std::invalid_argument(TIME_CONSTANT_MESSAGE);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    size_t TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                     MIN_ERROR_DECAY_TO_WINDOW_RATIO>::checkWindowIndex(
            size_t index) const
    {
        if (index < getUsedWindows()) {
            return index;
        }
        throw std::out_of_range("Window index greater than configured windows to use");
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                   MIN_ERROR_DECAY_TO_WINDOW_RATIO>::optimiseForMaximumSamples()
    {
        size_t maximumSamples = 0;
        for (size_t i = 0; i < usedWindows_; i++) {
            maximumSamples = std::max(maximumSamples, entry_[i].windowSamples());
        }
        if (history_.optimiseForMaximumWindowSamples(maximumSamples)) {
            for (size_t i = 0; i < usedWindows_; i++) {
                entry_[i].setReadPtr();
            }
        }
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                              MIN_ERROR_DECAY_TO_WINDOW_RATIO>::TrueFloatingPointWeightedMovingAverageSet(
            size_t maxWindowSamples, size_t errorMitigatingTimeConstant,
            size_t maxTimeConstants, S average) :
            entries_(validMaxTimeConstants(maxTimeConstants)),
            entry_(new Window[entries_]),
            usedWindows_(entries_),
            history_(maxWindowSamples, errorMitigatingTimeConstant)
    {
        history_.fillWithAverage(average);
        for (size_t i = 0; i < entries_; i++) {
            entry_[i].setOwner(&history_);
            entry_[i].setAverage(0);
            entry_[i].setWindowSamplesAndScale((i + 1) * maxWindowSamples / entries_, 1.0);
        }
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                   MIN_ERROR_DECAY_TO_WINDOW_RATIO>::setUsedWindows(
            size_t windows)
    {
        if (windows > 0 && windows <= getMaxWindows()) {
            usedWindows_ = windows;
            optimiseForMaximumSamples();
        }
        else {
            throw std::out_of_range(
                    "Number of used windows zero or larger than condigured maximum at construction");
        }
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                   MIN_ERROR_DECAY_TO_WINDOW_RATIO>::setWindowSizeAndScale(
            size_t index, size_t windowSamples, S scale)
    {
        if (windowSamples > getMaxWindowSamples()) {
            throw std::out_of_range("Window size in samples is larger than configured maximum at construction.");
        }
        entry_[checkWindowIndex(index)].setWindowSamplesAndScale(windowSamples, scale);
        optimiseForMaximumSamples();
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                   MIN_ERROR_DECAY_TO_WINDOW_RATIO>::setAverages(
            S average)
    {
        for (size_t i = 0; i < entries_; i++) {
            entry_[i].setAverage(average);
        }
        for (size_t i = 0; i < history_.historySize(); i++) {
            history_.set(i, average);
        }
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    S TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                MIN_ERROR_DECAY_TO_WINDOW_RATIO>::getAverage(
            size_t index) const
    {
        return entry_[checkWindowIndex(index)].getAverage();
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    size_t TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                     MIN_ERROR_DECAY_TO_WINDOW_RATIO>::getWindowSize(
            size_t index) const
    {
        return entry_[checkWindowIndex(index)].windowSamples();
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    S TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                MIN_ERROR_DECAY_TO_WINDOW_RATIO>::getWindowScale(
            size_t index) const
    {
        checkWindowIndex(index);
        return entry_[index].scale();
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    void TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                   MIN_ERROR_DECAY_TO_WINDOW_RATIO>::addInput(
            S input)
    {
        for (size_t i = 0; i < getUsedWindows(); i++) {
            entry_[i].addInput(input);
        }
        history_.write(input);
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    S TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                                MIN_ERROR_DECAY_TO_WINDOW_RATIO>::addInputGetMax(
            const S input, S minimumValue)
    {
        S average = minimumValue;
        for (size_t i = 0; i < getUsedWindows(); i++) {
            Window &entry = entry_[i];
            entry.addInput(input);
            const S v1 = entry.getAverage();
            average = std::max(v1, average);
        }
        history_.write(input);
        return average;
    }

    template<
            typename S, size_t SNR_BITS, size_t MIN_ERROR_DECAY_TO_WINDOW_RATIO>
    TrueFloatingPointWeightedMovingAverageSet<S, SNR_BITS,
                                              MIN_ERROR_DECAY_TO_WINDOW_RATIO>::~TrueFloatingPointWeightedMovingAverageSet()
    {
        delete[] entry_;
    }


#endif //TDAP_AVERAGE_IMPL_HPP
