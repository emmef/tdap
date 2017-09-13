/*
 * TestPower2.cpp
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

#include "TestPower2.hpp"

#include <tdap/bounds.hpp>

using namespace tdap;

typedef Power2 PowerOfTwo;

template<size_t N>
struct Dummyelement
{
	char data[N];

	size_t size() const { return N; }
};

template<size_t N>
struct Container
{
	Dummyelement<PowerOfTwo::constant::next(N)> element;
};

void TestPowerOf2::constantVariantastemplateArgument()
{
	Container<3> container;

	CPPUNIT_ASSERT_EQUAL_MESSAGE("Container3 size should be 4", (size_t)4, container.element.size());
}

static const char * toString(size_t value)
{
	static thread_local char ws[100];
	snprintf(ws, 100, "%zu", value);

	return ws;
}

static inline size_t
nextPowerOfTwoReference(size_t size)
{
    size_t maximumSize = Count<char>::max();

    if (size == 0 || size > maximumSize) {
        return 0;
    }

    size_t previousTest = 0;
    size_t test = 1;
    while (test < maximumSize && test < size && test > previousTest) {
        previousTest = test;
        test *= 2;
    }

    return test;
}
static inline size_t
previousPowerOfTwoReference(size_t size)
{
    size_t maximumSize = Count<char>::max();

    if (size == 0 || size > maximumSize) {
        return 0;
    }

    size_t test = nextPowerOfTwoReference(maximumSize / 2);
    while (test > size) {
        test /= 2;
    }

    return test;
}

static void
assertNextFastEqualsReference(const char *description, size_t testSize, bool constExpr) {
    static char message[1024];
    size_t expected = nextPowerOfTwoReference(testSize);
    size_t actual =
            constExpr ?
            Power2::constant::next(testSize) :
            PowerOfTwo::next(testSize);
    snprintf(message, 1024, "%s (value=%zu); %s", description, testSize,
             constExpr ? "constexpr" : "branchless");
    message[1023] = '\0';
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected, actual);
}
static void
assertPreviousFastEqualsReference(const char *description, size_t testSize, bool constExpr) {
    static char message[1024];
    size_t expected = previousPowerOfTwoReference(testSize);
    size_t actual =
            constExpr ?
            Power2::constant::previous(testSize) :
            PowerOfTwo::previous(testSize);
    snprintf(message, 1024, "%s (value=%zu); %s", description, testSize,
             constExpr ? "constexpr" : "branchless");
    message[1023] = '\0';
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected, actual);
}

void TestPowerOf2::testValueWithinPowerOfTwo(size_t value, size_t powerOfTwo) {
	static char workspace[128];
	size_t expected;
	if (value < powerOfTwo) {
		expected = value;
		snprintf(workspace, 128, "value (%zu) < powerOfTWo (%zu)yields value",
				value, powerOfTwo);
	}
	else if (value > powerOfTwo) {
		expected = powerOfTwo - 1;
		snprintf(workspace, 128,
				"value (%zu) > powerOfTWo (%zu) yields powerOfTwo - 1", value,
				powerOfTwo);
	}
	else {
		expected = powerOfTwo - 1;
		snprintf(workspace, 128,
				"value (%zu) == powerOfTWo (%zu) yields powerOfTwo - 1", value,
				powerOfTwo);
	}

	workspace[127] = 0;
	size_t actual = PowerOfTwo::within(value, powerOfTwo);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(workspace, expected, actual);
}

void TestPowerOf2::testAroundPowersOfTwo() {
	size_t maximumSize = Count<char>::max() / 2;
	size_t testSize = 1;
	while (testSize < maximumSize) {
		testSize *= 2;
            assertNextFastEqualsReference("Next power of two -> power of two", testSize,
                                          true);
            assertNextFastEqualsReference("Next power of two -> power of two", testSize,
                                          false);
            assertNextFastEqualsReference("Next power of two minus one -> power of two",
                                          testSize - 1, true);
            assertNextFastEqualsReference("Next power of two minus one -> power of two",
                                          testSize - 1, false);
            assertNextFastEqualsReference("Next power of two plus one -> next power of two",
                                          testSize + 1, true);
            assertNextFastEqualsReference("Next power of two plus one -> next power of two",
                                          testSize + 1, false);
            assertPreviousFastEqualsReference("Previous power of two -> power of two", testSize,
                                          true);
            assertPreviousFastEqualsReference("Previous power of two -> power of two", testSize,
                                          false);
            assertPreviousFastEqualsReference("Previous power of two minus one -> power of two",
                                          testSize - 1, true);
            assertPreviousFastEqualsReference("Previous power of two minus one -> power of two",
                                          testSize - 1, false);
            assertPreviousFastEqualsReference("Previous power of two plus one -> next power of two",
                                          testSize + 1, true);
            assertPreviousFastEqualsReference("Previous power of two plus one -> next power of two",
                                          testSize + 1, false);
	}
}

template<typename T>
static inline void testMaximumCountFor()
{
	std::string message;
	constexpr size_t elementSize = sizeof(T);
	const char * sizeString = toString(elementSize);
	constexpr size_t expectedMaxCount = Count<char>::max() / elementSize;
	constexpr size_t maxcountPlusOne = expectedMaxCount < Count<char>::max() ? expectedMaxCount + 1 : expectedMaxCount;
	constexpr size_t maxcountMinusOne = expectedMaxCount > 0 ? expectedMaxCount - 1 : expectedMaxCount;
	typedef Count<T> Count;

	message = "Maximum size for elements of size = ";
	message += sizeString;

	std::string specific;

	specific = message;
	specific += "; ForType";
	CPPUNIT_ASSERT_EQUAL_MESSAGE(specific.c_str(), expectedMaxCount, Count::max());

	message = "Elements of size = ";
	message += sizeString;
	message += "; max count must be valid";

	specific = message;
	specific += "; ForType";
	CPPUNIT_ASSERT_EQUAL_MESSAGE(specific.c_str(), true, Count::valid(expectedMaxCount));

	if (maxcountMinusOne != expectedMaxCount) {
		message = "Elements of size = ";
		message += sizeString;
		message += "; max count minus one must be valid";

		specific = message;
		specific += "; ForType";
		CPPUNIT_ASSERT_EQUAL_MESSAGE(specific.c_str(), true, Count::valid(maxcountMinusOne));
	}
	if (maxcountPlusOne != expectedMaxCount) {
		message = "Elements of size = ";
		message += sizeString;
		message += "; max count plus one must be invalid";

		specific = message;
		specific += "; ForType";
		CPPUNIT_ASSERT_EQUAL_MESSAGE(specific.c_str(), false, Count::valid(maxcountPlusOne));
	}
}

void TestPowerOf2::testWithinPowerOfTwo() {
	for (size_t powerOfTwo = 1; powerOfTwo != 0; powerOfTwo *= 2) {
		for (size_t i = powerOfTwo; i > 0; i /= 2) {
			testValueWithinPowerOfTwo(i - 1, powerOfTwo);
			testValueWithinPowerOfTwo(i, powerOfTwo);
			testValueWithinPowerOfTwo(i + 1, powerOfTwo);
		}
		for (double x = 1; x < Count<char>::max(); x *= 1.2) {
			testValueWithinPowerOfTwo((size_t) (x), powerOfTwo);
		}
	}
}


void TestPowerOf2::setUp()
{
	// Nothing to implement
}

void TestPowerOf2::tearDown()
{
	// Nothing to implement
}

TestPowerOf2::~TestPowerOf2()
{
	// Nothing to implement
}

