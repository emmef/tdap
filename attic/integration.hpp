/*
 * tdap/Integration.hpp
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

#ifndef TDAP_INTEGRATION_HEADER_GUARD
#define TDAP_INTEGRATION_HEADER_GUARD

#include <type_traits>
#include <cmath>
#include <limits>

#include <tdap/bounds.hpp>

namespace tdap {

    struct Integration
    {
        template<typename F>
        static constexpr F min_samples()
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return std::numeric_limits<F>::epsilon();
        }

        template<typename F>
        static constexpr F max_samples()
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return 1.0 / min_samples<F>();
        }

        template<typename F>
        static const Range<F> &range()
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            static const Range<F> range_(min_samples<F>(), max_samples<F>());

            return range_;
        }

        template<typename F>
        static constexpr F limited_samples(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return between(samples, min_samples<F>(), max_samples<F>());
        }

        template<typename F>
        static F checked_samples(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return range<F>().get_valid(samples);
        }

        template<typename F>
        static constexpr F get_unchecked_history_multiplier(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return exp(-1.0 / samples);
        }

        template<typename F>
        static constexpr F get_history_multiplier(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return samples < min_samples<F>()
                   ? 0.0
                   : get_unchecked_history_multiplier(
                            samples > max_samples<F>() ? max_samples<F>() : samples);
        }

        template<typename F>
        static constexpr F get_history_multiplier_limited(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return get_unchecked_history_multiplier(limited_samples(samples));
        }

        template<typename F>
        static F get_history_multiplier_checked(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return get_unchecked_history_multiplier(checked_samples(samples));
        }

        template<typename F>
        static constexpr F get_other_multiplier(F history_multiplier)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return 1.0 - history_multiplier;
        }

        template<typename F>
        static constexpr F get_input_multiplier(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return samples < min_samples<F>()
                   ? 1.0
                   : get_other_multiplier(get_unchecked_history_multiplier(
                            samples > max_samples<F>() ? max_samples<F>() : samples));
        }

        template<typename F>
        static constexpr F get_input_multiplier_limited(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return get_other_multiplier(get_unchecked_history_multiplier(limited_samples(samples)));
        }

        template<typename F>
        static F get_input_multiplier_checked(F samples)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return get_other_multiplier(get_unchecked_history_multiplier(checked_samples(samples)));
        }

        template<typename F>
        static constexpr F get_samples_from_history_multiply(double history_multiply)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return -1.0 / log(history_multiply);
        }

        template<typename F>
        static F constexpr get_samples_from_input_multiply(double input_multiply)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return -1.0 / log(1.0 - input_multiply);
        }

        template<typename F, typename S>
        static constexpr F integrate(F history_multiply, F input_multiply, S input, F history)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return input_multiply * input + history_multiply * history;
        }

        template<typename F, typename S>
        static constexpr F integrate(F history_multiply, S input, F history)
        {
            static_assert(std::is_floating_point<F>::value, "Type parameter F should be floating point");
            return (1.0 - history_multiply) * input + history_multiply * history;
        }

        template<typename F>
        static F valid_samples(double sampleRate, double seconds)
        {
            double samples = Count<double>::valid_positive(sampleRate) * Count<double>::valid_positive(seconds);
            if (samples < max_samples<F>() && samples < Count<char>::max()) {
                return samples;
            }
            throw std::invalid_argument(
                    "Average:: Combination of sampleRate_ and seconds yields too large sample count");
        }
    };

    template<typename F>
    class IntegrationCoefficients
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        F history_multiply_ = 0;
        F input_multiply_ = 1.0;
    public:
        IntegrationCoefficients()
        {}

        IntegrationCoefficients(double characteristicSamples) :
                history_multiply_(Integration::get_history_multiplier(characteristicSamples)),
                input_multiply_(1.0 - history_multiply_)
        {}

        IntegrationCoefficients(double sampleRate, double seconds) :
                IntegrationCoefficients(Integration::valid_samples<double>(sampleRate, seconds))
        {}

        F history_multiply() const
        { return history_multiply_; }

        F input_multiply() const
        { return input_multiply_; }

        void set_integration_samples(double value)
        {
            history_multiply_ = Integration::get_history_multiplier(value);
            input_multiply_ = 1.0 - history_multiply_;
        }

        void set_integration_time_and_rate(double seconds, double sampleRate)
        {
            set_integration_samples(
                    Integration::valid_samples<double>(sampleRate, seconds));
        }

        F get_integration_samples() const
        {
            return Integration::get_samples_from_history_multiply<F>(history_multiply_);
        }

        template<typename V>
        V getIntegrated(const V input, const V previousOutput) const
        {
            static_assert(std::is_floating_point<V>::value,
                          "V must be a floating-point type (stability condition)");
            return Integration::integrate(history_multiply_, input_multiply_, input, previousOutput);
        }

        template<typename V>
        V integrate(const V input, V &output) const
        {
            return (output = getIntegrated(input, output));
        }

        template<typename V>
        V get_decay(const V value) const
        {
            return value * history_multiply_;
        }

        template<typename V>
        V decay(V &value) const
        {
            return (value = get_decay(value));
        }
    };

    template <typename F>
    struct HoldMax
    {
        F max_ = 0;
        size_t hold_count_ = 0;
        size_t count_down_ = 0;

        template<typename V>
        F get_value(const V input, const V integrated_value)
        {
            if (input > max_) {
                count_down_ = hold_count_;
                max_ = input;
                return input;
            }
            if (count_down_ > 0) {
                count_down_--;
                return max_;
            }
            max_ = integrated_value;
            return input;
        }

        void reset()
        {
            max_ = 0;
            count_down_ = 0;
        }
    };

    template<typename F>
    struct Integrator
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        IntegrationCoefficients<F> coefficients_;
        F output_ = 0;

        template<typename V>
        V integrate(const V input)
        {
            return coefficients_.integrate(input, output_);
        }

        template<typename V>
        V integrate(const V input, V &output) const
        {
            return coefficients_.integrate(input, output);
        }

        template<typename V>
        void set_output(const V new_output)
        {
            output_ = new_output;
        }
    };

    template<typename F>
    struct SmoothIntegrator
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        Integrator<F> filter_;
        F output_ = 0;

        template<typename V>
        V integrate(const V input, V &pre_smooth_output, V &post_smooth_output) const
        {
            return filter_.integrate(filter_.integrate(input, pre_smooth_output), post_smooth_output);
        }

        template<typename V>
        V integrate(const V input)
        {
            return filter_.integrate(filter_.integrate(input), output_);
        }

        template<typename V>
        void set_output(V new_output)
        {
            filter_.set_output(new_output);
            output_ = new_output;
        }
    };

    template<typename F>
    struct SmoothHoldMaxIntegrator
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        SmoothIntegrator<F> filter_;
        HoldMax<F> hold_max_;

        template<typename V>
        V integrate(const V input)
        {
            return filter_.integrate(hold_max_.get_value(input, filter_.output_));follower_.hold_max_.
        }

        template<typename V>
        void set_output(V new_output)
        {
            filter_.set_output(new_output);
            hold_max_.reset();
        }

        void set_hold_count(size_t hold_count)
        {
            hold_max_.hold_count_ = hold_count;
        }
    };

    template<typename F>
    struct AttackReleaseIntegrator
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        IntegrationCoefficients<F> attack_;
        IntegrationCoefficients<F> release_;
        F output_;

        template<typename V>
        V integrate(const V input, V &output)
        {
            return input > output ? attack_.integrate(input, output) : release_.integrate(input, output);
        }

        template<typename V>
        V integrate(const V input)
        {
            return integrate(output_);
        }

        template<typename V>
        void setOutput(V new_output)
        {
            output_ = new_output;
        }
    };

    template<typename F>
    struct SmoothAttackReleaseIntegrator
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        AttackReleaseIntegrator<F> filter_;
        F output_;

        template<typename V>
        V integrate(const V input, V &pre_smooth_output, V &post_smooth_output)
        {
            return filter_.integrate(filter_.integrate(input, pre_smooth_output), post_smooth_output);
        }

        template<typename V>
        V integrate(const V input)
        {
            return filter_.integrate(filter_.integrate(input), output_);
        }

        template<typename V>
        void setOutput(const V new_output)
        {
            filter_.setOutput(new_output);
            output_ = new_output;
        }
    };

    template<typename F>
    struct SmoothHoldMaxAttackReleaseIntegrator
    {
        static_assert(std::is_floating_point<F>::value, "F must be a floating-point type");

        SmoothAttackReleaseIntegrator<F> filter_;
        HoldMax<F> hold_max_;

        template<typename V>
        V integrate(const V input)
        {
            return filter_.integrate(hold_max_.get_value(input, filter_.output_));
        }

        template<typename V>
        void setOutput(const V new_output)
        {
            filter_.setOutput(new_output);
            hold_max_.reset();
        }

        void set_hold_count(size_t hold_count)
        {
            hold_max_.hold_count_ = hold_count;
        }
    };


} /* End of name space tdap */

#endif /* TDAP_INTEGRATION_HEADER_GUARD */
