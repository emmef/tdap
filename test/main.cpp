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
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TextTestRunner.h>
#include "TestPower2.hpp"
#include <tdap/array.hpp>
#include <tdap/buffer.hpp>
#include <tdap/circular.hpp>
#include <tdap/delay.hpp>

using namespace CppUnit;
using namespace std;
using namespace tdap;

int main ()
{
    CppUnit::TextTestRunner runner;
    Array<float, 15> data;

    runner.addTest(TestPowerOf2::createSuite());

    cout << "Starting tests!" << endl;

    return runner.run() ? 0 : 1;
}
