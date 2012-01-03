/* Copyright (c) 2012 Yohan Lasorsa (noda@free.fr)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * NTK/Unit
 * test_example.cpp
 *
 * author       : Yohan Lasorsa (noda)
 * version      : 1.0
 * last change  : 2012-01-03
 */

#include "test.hpp"

SUITE(NTK_Unit);

// -- Test all assertions macros ---------------------------------

SUBSUITE(NTK_Unit, Assertions);

FIXTURE(AssertionsFixture) {
    
    int i;
    float f;
    char d[10];
    
    SETUP(AssertionsFixture) {
        i = 2;
        f = 3.0;
        for (int j = 0; j < 10; ++j)
            d[j] = j;
    }
    
    TEARDOWN(AssertionsFixture) {
        i = 0;
        f = 0.0;
        for (int j = 0; j < 10; ++j)
            d[j] = 0;
    }
    
};

TEST(Check) {
    T_CHECK(true);
}

TEST(CheckEqual) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_EQUAL(F.i, 2);
}

TEST(CheckDiffer) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_DIFFER(F.i, 0);
}

TEST(CheckClose) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_CLOSE(F.f, 3.0001, 0.001);
}

TEST(CheckLessThan) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_LESS_THAN(F.f, 3.1);
}

TEST(CheckLessOrEqual) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_LESS_OR_EQUAL(F.f, 3.1);
    T_CHECK_LESS_OR_EQUAL(F.f, 3.0);
}

TEST(CheckMoreThan) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_MORE_THAN(F.f, 2.9);
}

TEST(CheckMoreOrEqual) {
    USE_FIXTURE(AssertionsFixture);
    T_CHECK_MORE_OR_EQUAL(F.f, 2.9);
    T_CHECK_MORE_OR_EQUAL(F.f, 3.0);
}

TEST(CheckSameData) {
    USE_FIXTURE(AssertionsFixture);
    char data[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    T_CHECK_SAME_DATA(F.d, data, 10);
}

TEST(CheckThrows) {
    T_CHECK_THROWS(throw 1, int);
}

TEST(CheckThrowsAny) {
    T_CHECK_THROWS_ANY(throw 1);
}

TEST(CheckNoThrow) {
    int i = 0;
    T_CHECK_NOTHROW(i++);
}

// -- Test all assertions macros failures ------------------------

SUBSUITE(NTK_Unit, Failures);

FIXTURE(FailuresFixture) {
    
    int i;
    float f;
    char d[10];
    
    SETUP(FailuresFixture) {
        i = 1;
        f = 3.0;
        for (int j = 0; j < 10; ++j)
            d[j] = 10 - j;
    }
    
    TEARDOWN(FailuresFixture) {
        i = 0;
        f = 0.0;
        for (int j = 0; j < 10; ++j)
            d[j] = 0;
    }
    
};

TEST(CheckFailure) {
    TM_CHECK(false, "This test should fail");
}

TEST(CheckEqualFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_EQUAL(F.i, 2, "This test should fail");
}

TEST(CheckDifferFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_DIFFER(F.i, 1, "This test should fail");
}

TEST(CheckCloseFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_CLOSE(F.f, 3.01, 0.001, "This test should fail");
}

TEST(CheckLessThanFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_LESS_THAN(F.f, 2.9, "This test should fail");
}

TEST(CheckLessOrEqualFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_LESS_OR_EQUAL(F.f, 2.9, "This test should fail");
}

TEST(CheckMoreThanFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_MORE_THAN(F.f, 3.1, "This test should fail");
}

TEST(CheckMoreOrEqualFailure) {
    USE_FIXTURE(FailuresFixture);
    TM_CHECK_MORE_OR_EQUAL(F.f, 3.1, "This test should fail");
}

TEST(CheckSameDataFailure) {
    USE_FIXTURE(FailuresFixture);
    char data[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    TM_CHECK_SAME_DATA(F.d, data, 10, "This test should fail");
}

TEST(CheckThrowsFailure) {
    int i = 0;
    TM_CHECK_THROWS(i++, int, "This test should fail");
}

TEST(CheckThrowsAnyFailure) {
    int i = 0;
    TM_CHECK_THROWS_ANY(i++, "This test should fail");
}

TEST(CheckNoThrowFailure) {
    TM_CHECK_NOTHROW(throw 1, "This test should fail");
}

TEST(CheckFail) {
    TM_CHECK_FAIL("This test should fail");
}

// -- Test unhandled exception failures ------------------------

#ifndef DO_NOT_USE_EXCEPTIONS

#include <exception>

SUBSUITE(NTK_Unit, UnhandledExceptions);

TEST(UnhandledStdException) {
    throw std::exception();
}

TEST(UnhandledOtherException) {
    throw 1;
}

#ifndef DO_NOT_CATCH_SIGNALS

TEST(UnhandledSystemException) {
    int* p = 0;
    p[10] = 1;  // bad pointer access
    
    // I don't know why, but on some compilers the exception jumps out of the first try/catch block
}

#endif // DO_NOT_CATCH_SIGNALS

#endif // DO_NOT_USE_EXCEPTIONS

// -- Run all tests ----------------------------------------------

RUN_TESTS(ntk::OStreamTestResult);

