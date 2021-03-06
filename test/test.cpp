/*
 * Main.cpp
 *
 * Runs Unit-test suite.
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

#include <iostream>
#include <tdap/average.hpp>
#include <tdap/filter.hpp>
#include <tdap/boundaries.hpp>

using namespace std;

using DoubleFilter = tdap::filter::Filter<double>;
using DoubleChannelFilter = tdap::filter::ChannelFilter<double>;

int main ()
{
    cout << "Hello world!" << endl;

    cout
    <<
    "Effective length of identity filter is "
    << DoubleFilter::getEffectiveImpulseResponseLength(
            DoubleFilter::identity(), 1000, 1e-12, 100) << endl;

    return 0;
}
