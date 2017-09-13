/*
 * TestPower2.hpp
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

#ifndef TEST__TDAP_POWEROF2_HEADER_GUARD
#define TEST__TDAP_POWEROF2_HEADER_GUARD

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

/*
 * Do NOT include <tdap/power2.hpp> here as it will
 * slow down the build process considerably.
 * The file
 *   test_power2.cpp
 * that implements this test fixture will include the header.
 */
using namespace CppUnit;

class TestPowerOf2 : public TestFixture
{
public:
	void constantVariantastemplateArgument();
	void testWithinPowerOfTwo();
	void testAroundPowersOfTwo();
	static void testValueWithinPowerOfTwo(size_t value, size_t powerOfTwo);

	static TestSuite *createSuite()
	{
		CppUnit::TestSuite *suite = new CppUnit::TestSuite("Test suite for 'tdap/PowerOf2.hpp'");

		suite->addTest(
				new TestCaller<TestPowerOf2>("TestPowerOf2 test 'testWithinPowerOfTwo'",
						&TestPowerOf2::testWithinPowerOfTwo));
		suite->addTest(
				new TestCaller<TestPowerOf2>("TestPowerOf2 test 'testAroundPowersOfTwo'",
						&TestPowerOf2::testAroundPowersOfTwo));
		suite->addTest(
				new TestCaller<TestPowerOf2>("TestPowerOf2 test 'constantVariantastemplateArgument'",
						&TestPowerOf2::constantVariantastemplateArgument));

		return suite;
	}

	void setUp();
	void tearDown();
	virtual ~TestPowerOf2();
};

#endif /* TEST__TDAP_POWEROF2_HEADER_GUARD */
