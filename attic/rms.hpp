/*
 * tdap/Rms.hpp
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

#ifndef TDAP_RMS_HEADER_GUARD
#define TDAP_RMS_HEADER_GUARD

#include <cstddef>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <tdap/bounds.hpp>
#include <tdap/integration.hpp>

namespace tdap {
    template<typename T, size_t MAX_BUCKETS>
    struct BucketAverage
    {
        static constexpr size_t MAX_MAX_BUCKETS = 64;
        static constexpr size_t MIN_BUCKETS = 2;
        static_assert(is_between(MAX_BUCKETS, MIN_BUCKETS, MAX_MAX_BUCKETS),
                      "Number of buckets must be between 2 and 64");

        BucketAverage() = default;

        size_t get_window_size() const
        { return bucket_count_ * bucket_size_; }

        size_t get_bucket_count() const
        { return bucket_count_; }

        size_t get_bucket_size() const
        { return bucket_size_; }


        size_t
        set_approximate_window_size(size_t window_samples,
                                    double max_relative_error = 0.01,
                                    size_t minimum_preferred_bucket_count = MIN_BUCKETS)
        {
            double errors[MAX_BUCKETS + 1 - MIN_BUCKETS];
            size_t max_error = window_samples;
            size_t min_buckets = std::max(MIN_BUCKETS,
                                          minimum_preferred_bucket_count);

            double min_error = 1;

            for (size_t bucket_count = MAX_BUCKETS;
                 bucket_count >= min_buckets; bucket_count--) {

                size_t bucket_size = window_samples / bucket_count;
                size_t window_size = bucket_size * bucket_count;
                double error =
                        fabs(window_samples - window_size) / window_samples;
                if (error < max_relative_error) {
                    set_bucket_size_and_count(bucket_size, bucket_count);
                    return window_size;
                }
                errors[MAX_BUCKETS - bucket_count] = error;
                if (error < min_error) {
                    min_error = error;
                }
            }

            for (size_t bucket_count = min_buckets - 1;
                 bucket_count >= MIN_BUCKETS; bucket_count--) {

                size_t bucket_size = window_samples / bucket_count;
                size_t window_size = bucket_size * bucket_count;
                double error =
                        fabs(window_samples - window_size) / window_samples;
                if (error < max_relative_error) {
                    set_bucket_size_and_count(bucket_size, bucket_count);
                    return window_size;
                }
                errors[MAX_BUCKETS - bucket_count] = error;
                if (error < min_error) {
                    min_error = error;
                }
            }
            for (size_t bucket_count = MAX_BUCKETS;
                 bucket_count >= MIN_BUCKETS; bucket_count--) {

                if (errors[MAX_BUCKETS - bucket_count] == min_error) {
                    // There is always a bucket count that meets this condition
                    size_t bucket_size = window_samples / bucket_count;
                    size_t window_size = bucket_size * bucket_count;

                    set_bucket_size_and_count(bucket_size, bucket_count);
                    return window_size;
                }
            }
            // There is always a bucket count that meets the break condition
            // in the previous loop, so we never end up here.
            return 0;
        }

        bool set_bucket_count(size_t count)
        {
            return set_bucket_size_and_count(bucket_size_, count);
        }

        bool set_bucket_size(size_t size)
        {
            return set_bucket_size_and_count(size, bucket_count_);
        }

        bool set_bucket_size_and_count(size_t size, size_t count)
        {
            if (size == 0) {
                return false;
            }
            if (!is_between(count, MIN_BUCKETS, MAX_BUCKETS)) {
                return false;
            }
            bucket_count_ = count;
            bucket_size_ = size;
            size_t window_size = get_window_size();
            scale_ = user_scale_ / window_size;
            set_average(sum_ / window_size);
            return true;
        }
        
        bool set_output_scale(const T scale)
        {
            if (scale <= std::numeric_limits<T>::epsilon()) {
                return false;
            }
            user_scale_ = scale;
            scale_ = user_scale_ / get_window_size(); 
            return true;
        }

        void set_average(T average)
        {
            current_sample_ = bucket_size_;
            current_bucket_ = 0;
            T bucket_value = average * bucket_size_;
            for (size_t i = 0; i < bucket_count_; i++) {
                bucket_[i] = bucket_value;
            }
            sum_ = bucket_value * bucket_count_;
            old_bucket_sum_ = sum_ - bucket_value;
            new_bucket_sum = 0;
            average_square_sample_value = average;
        }

        T add_sample_get_average(const T sample)
        {
            if (current_sample_ > 0) {
                // happy path
                new_bucket_sum += sample;
                sum_ = old_bucket_sum_ + new_bucket_sum +
                       current_sample_ * average_square_sample_value;
                current_sample_--;
                return get_average();
            }
            // darn: bucket full
            sum_ = old_bucket_sum_ + new_bucket_sum;
            bucket_[current_bucket_] = new_bucket_sum;

            current_bucket_++;
            current_bucket_ %= bucket_count_;
            current_sample_ = bucket_size_;

            new_bucket_sum = 0;

            old_bucket_sum_ = 0;
            for (size_t bucket = current_bucket_ + 1;
                 bucket < bucket_count_; bucket++) {
                old_bucket_sum_ += bucket_[bucket];
            }
            for (size_t bucket = 0; bucket <= current_bucket_; bucket++) {
                old_bucket_sum_ += bucket_[bucket];
            }
            average_square_sample_value = get_average();
            return average_square_sample_value;
        }

        T get_average() const
        {
            return sum_ * scale_;
        }

        std::ostream &operator>>(std::ostream &out)
        {
            out
                    << "BucketAverage<T," << MAX_BUCKETS
                    << ">(bucket_size=" << get_bucket_size()
                    << ",bucket_count=" << get_bucket_count()
                    << ",window_size=" << get_window_size();

            return out;
        }
    private:
        T bucket_[MAX_BUCKETS];
        T old_bucket_sum_ = 0;
        T new_bucket_sum = 0;
        T average_square_sample_value = 0;
        T sum_ = 0;
        T user_scale_ = 1.0;
        T scale_ = 1.0 / MAX_BUCKETS;
        size_t bucket_size_ = 1;
        size_t bucket_count_ = MAX_BUCKETS;
        size_t current_bucket_ = MAX_BUCKETS;
        size_t current_sample_ = 0;
    };

    template<typename T, size_t MAX_BUCKETS>
    class BucketRms
    {
        BucketAverage<T, MAX_BUCKETS> average_;
    public:
        const BucketAverage<T, MAX_BUCKETS> &average() const
        { return average_; }

        size_t
        set_approximate_window_size(size_t window_samples,
                                    double max_relative_error = 0.01,
                                    size_t minimum_preferred_bucket_count = BucketAverage<T, MAX_BUCKETS>::MIN_BUCKETS)
        {
            return average_.set_approximate_window_size(window_samples,
                                                        max_relative_error,
                                                        minimum_preferred_bucket_count);
        }

        bool set_bucket_count(size_t count)
        {
            return average_.set_bucket_size_and_count(
                    average_.get_bucket_size(), count);
        }

        bool set_bucket_size(size_t size)
        {
            return average_.set_bucket_size_and_count(size,
                                                      average_.get_bucket_count());
        }

        bool set_bucket_size_and_count(size_t size, size_t count)
        {
            return average_.set_bucket_size_and_count(size, count);
        }

        void set_average(T average)
        {
            return average_.set_average(average * average);
        }

        bool set_scale(T scale)
        {
            return average_.set_output_scale(scale * scale);
        }

        T add_sample_get_rms(const T sample)
        {
            return sqrt(average_.add_sample_get_average(sample * sample));
        }

    };

    struct PerceptiveMetrics
    {
        static constexpr double PERCEPTIVE_SECONDS = 0.400;
        static constexpr double PEAK_SECONDS = 0.0004;
        static constexpr double PEAK_HOLD_SECONDS = 0.0050;
        static constexpr double PEAK_RELEASE_SECONDS = 0.0100;
        static constexpr double MAX_SECONDS = 10.0000;
        static constexpr double PEAK_PERCEPTIVE_RATIO =
                PEAK_SECONDS / PERCEPTIVE_SECONDS;
    };

    template<typename S, size_t BUCKETS, size_t LEVELS>
    class PerceptiveRms : protected PerceptiveMetrics
    {
        static_assert(is_between(LEVELS, (size_t) 3, (size_t) 16),
                      "Levels must be between 3 and 16");

        static constexpr double INTEGRATOR_WINDOW_SIZE_RATIO = 0.2;

        using Rms = BucketRms<S, BUCKETS>;
        Array <Rms, LEVELS> rms_;
        size_t used_levels_ = LEVELS;
        SmoothHoldMaxAttackReleaseIntegrator<S> follower_;

        S get_biggest_window_size(S biggest_window) const
        {
            S limited_window = std::min(MAX_SECONDS, limited_window);

            if (limited_window < PERCEPTIVE_SECONDS) {
                return PERCEPTIVE_SECONDS;
            }
            double big_to_perceptive_log =
                    (log(limited_window) - log(PERCEPTIVE_SECONDS)) /
                    M_LN2;
            if (big_to_perceptive_log < 0.5) {
                return PERCEPTIVE_SECONDS;
            }
            return limited_window;
        }

        void determine_number_of_levels(double biggest_window, size_t levels,
                                        size_t &bigger_levels,
                                        size_t &smaller_levels)
        {
            double bigger_weight =
                    log(biggest_window) - log(PERCEPTIVE_SECONDS);
            double smaller_weight =
                    log(PERCEPTIVE_SECONDS) - log(PEAK_SECONDS);
            // apart frm perceptive window, we always have used_levels - 1 available windows to divide for
            bigger_levels = bigger_weight * (LEVELS - 1) /
                            (smaller_weight + bigger_weight);
            smaller_levels = std::max(1.0, smaller_weight * (LEVELS - 1) /
                                           (smaller_weight +
                                            bigger_weight));
            size_t extra_levels = bigger_levels + smaller_levels;
            while (extra_levels < LEVELS - 1) {
                if (biggest_window > PERCEPTIVE_SECONDS) {
                    bigger_levels++;
                }
                else {
                    smaller_levels++;
                }
                extra_levels = bigger_levels + smaller_levels;
            }
        }

    public:
        PerceptiveRms() : follower_(1, 1, 1, 1) {};

        void configure(size_t sample_rate, S biggest_window, S peak_to_rms,
                       S integration_to_window_size = INTEGRATOR_WINDOW_SIZE_RATIO,
                       size_t levels = LEVELS)
        {
            used_levels_ = Value<size_t>::force_between(levels, 3, LEVELS);
            double peak_scale = 1.0 / Value<S>::force_between(peak_to_rms, 2, 10);
            double biggest = get_biggest_window_size(biggest_window);
            double integration_factor = Value<S>::force_between(
                    integration_to_window_size, 0.1, 1);
            size_t bigger_levels;
            size_t smaller_levels;
            determine_number_of_levels(biggest, levels, bigger_levels,
                                       smaller_levels);

            for (size_t level = 0; level < smaller_levels; level++) {
                double exponent =
                        1.0 * (smaller_levels - level) / smaller_levels;
                double window_size = PERCEPTIVE_SECONDS *
                                     pow(PEAK_PERCEPTIVE_RATIO, exponent);
                double scale = level == 0 ?
                               peak_scale :
                               pow(PEAK_PERCEPTIVE_RATIO, exponent * 0.25);
                double rc = window_size * integration_factor;
                rms_[level].setWindowSizeAndRc(window_size * sample_rate,
                                               rc * sample_rate);
                rms_[level].set_output_scale(scale);
            }

            rms_[smaller_levels].setWindowSizeAndRc(
                    PERCEPTIVE_SECONDS * sample_rate,
                    PERCEPTIVE_SECONDS * sample_rate * integration_factor);
            rms_[smaller_levels].set_output_scale(1.0);

            for (size_t level = smaller_levels + 1;
                 level < used_levels_; level++) {
                double exponent = 1.0 * (level - smaller_levels) /
                                  (used_levels_ - 1 - smaller_levels);
                double window_size = biggest *
                                     pow(biggest / PERCEPTIVE_SECONDS,
                                         exponent);
                double rc = window_size * integration_factor;
                rms_[level].setWindowSizeAndRc(window_size * sample_rate,
                                               rc * sample_rate);
            }

            for (size_t level = 0; level < used_levels_; level++) {
                double window_size =
                        1.0 * rms_[level].get_window_size() / sample_rate;
                double scale = rms_[level].get_output_scale();
            }

            follower_ = SmoothHoldMaxAttackRelease<S>(
                    PEAK_HOLD_SECONDS * sample_rate,
                    0.5 + 0.5 * PEAK_SECONDS * sample_rate,
                    PEAK_RELEASE_SECONDS * sample_rate,
                    10);
        }

        S add_square_get_detection(S square, S minimum = 0)
        {
            S value = minimum;
            for (int level = used_levels_; --level > 0; ) {
                value = rms_[level].addSquareCompareAndGet(square, value);
            }
            return follower_.apply(value);
        }

        const FixedSizeArray <BucketIntegratedRms<S, BUCKETS>, LEVELS> &
        rms() const
        {
            return rms_;
        };


    };



} /* End of name space tdap */

#endif /* TDAP_RMS_HEADER_GUARD */
