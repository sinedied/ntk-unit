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
 * test.hpp
 *
 * author       : Yohan Lasorsa (noda)
 * version      : 1.0
 * last change  : 2012-01-03
 */

/** 
 @file test.hpp  
 NTK/Unit, a minimalistic yet easily extensible unit testing framework.
 This file contains all the needed tools to support basic unit testing, but you may extend the provided functionalities to better fit your needs.
 
 The approach of this testing framework is greatly inspired by CppUnitLite, with some rework to support fixtures, suites, exceptions (including signal 
 handling) and global tests timing.
 All the classes and macros have been organized in only one .hpp file for easy integration and faster adaptation in case of specific needs.
 
 The framework makes heavy use of macros to make writing tests easier and quicker, but you can use the underlying classes directly if you prefer a more
 object-oriented and verbose approach.
 
 Exception handling by the framework can be disabled, to trace the source of an exception in your debugger for example. You just need to declare the 
 compilation constant DO_NOT_USE_EXCEPTIONS to do so. You may also disable catching the system exceptions (also known as signals) by declaring the constant
 DO_NOT_CATCH_SIGNALS.
 
 Even though it is part of the NTK, it does not rely on any NTK classes (in fact was aimed to be a testing framework to test the NTK classes).
 This framework is compiler-agnostic and based on the standard C++ library.  
 
 Example usage:
 @code
 // declare a new test suite
 SUITE(myTestSuite);
 
 // declare a simple test
 TEST(myTest) {
     TM_CHECK_EQUAL(1, 0, "this test is meant to fail");
 }
 
 // declare a fixture
 FIXTURE(myFixture) {
 
     int myVar;
 
     SETUP(myFixture) {
         myVar = 2;
     }
 
 };
 
 // declare a test using our fixture
 TEST(myTest2) {
     USE_FIXTURE(myFixture);     // use our fixture
     T_CHECK_EQUAL(F.myVar, 2);  // make sure our fixture is correctly setup, note that the "F." prefix to access the fixture variable "myVar"
 }
 
 // declare a main function to run all tests using std::cout to output the results
 RUN_TESTS(OStreamTestResult);
 @endcode
 */

#ifndef __ntk_test_hpp__
#define __ntk_test_hpp__

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <limits>
#include <ctime>
#include <cmath>

#ifndef DO_NOT_USE_EXCEPTIONS
#   include <exception>
#   ifndef DO_NOT_CATCH_SIGNALS
#       include <csignal>
#   endif
#endif

namespace ntk {
    
#pragma mark -
#pragma mark Test types
    
    /** Defines constant for the different test types. */
    namespace TestType {
        const std::string TestCase = "TestCase";        ///< Test case type (a simple test).
        const std::string TestSuite = "TestSuite";      ///< Test suite type (a group of tests).
    }
    
#pragma mark -
#pragma mark Test definition
    
    class TestResult;
    class TestFailure;
    
    /**
     Test is a base class for all tests. It provides method for running individual tests (runTest) as well as data members for recording the name and type of
     the test. The type of test may be used in a ntk::TestResult class for processing differently differents test types (for example test case, test 
     suites, etc).
     
     A test class is automatically registred in a global test set so you can use the static method Test::run() to run all tests at once.
     Tests may be constructed and organized using the TEST, FIXTURE, SUITE and SUBSUITE macros, provided for convenience.
     
     @see ntk::TestSuite
     @see ntk::TestFixture
     @see ntk::TestResult
     @see ntk::TestType
     */
    class Test {
    public:
        
        /** 
         Creates a new test with the specified name and type. 
         The test may be specified to automatically register itself in the global test set (disabled by default).
         */
        Test(const std::string& name, const std::string& type = TestType::TestCase, bool autoRegisterTest = false) 
        : mName(name), mType(type)
        { if (autoRegisterTest) registerTest(this); }
        
        /** Destroys the test. */
        virtual ~Test() {}
        
        /**
         Runs the test, using the specified Test::Result object to process the test results.
         Do not override this method to implement the test code, the method runTest() should be used instead.
         This method should be overriden with great care as it all the necessary calls to enable fixture support, exception handling and result commits.
         */
        virtual int run(TestResult& result); // implemented later because of TestFailure and TestResult dependencies
        
        /** Gets the type of the test. */
        const std::string& type() const { return mType; }
        
        /** Gets the name of the test. */
        const std::string& name() const { return mName; }
        
        /** Runs all the tests, using the specified TestResult object to process the test results. */
        static int runAll(TestResult& result); // implemented later because of TestResult dependency
        
    protected:
        
        /** The method containing the actual test code, to be overriden. */
        virtual void runTest(TestResult& result) = 0;
        
    private:
        static std::vector<Test*>& mTests() { static std::vector<Test*> tests; return tests; } // the list of all tests
        static void registerTest(Test* test) { mTests().push_back(test); } // registers a new test in the global list (automatically done)

        std::string mName;                  // the name of the test
        std::string mType;                  // the type of the test
    };
    
#pragma mark -
#pragma mark Test suite definition
    
    /**
     A TestSuite is essentially a group of tests.
     @see ntk::Test
     */
    class TestSuite : public Test 
    {
    public:
        
        /** 
         Creates a new test suite with the specified name and type. 
         The test suite may be specified to automatically register itself in the global test set (enabled by default).
         */
        TestSuite(const std::string& name, const std::string& type = TestType::TestSuite, bool autoRegisterTestGroup = true)
        : Test(name, type, autoRegisterTestGroup)
        {}
        
        /** Destroys the test suite. */
        virtual ~TestSuite() {}
        
        /** Adds a test to the test suite. */
        void addTest(Test* test) { mTests.push_back(test); }
        
        /**
         Sets the current test suite.
         If the test suite provided is NULL, returns the current test suite.
         */
        static TestSuite* currentTestSuite(TestSuite* testSuite) {
            // warning, default test suite won't be registered if a test suite is declared!
            static TestSuite defaultTestSuite("DefaultTestSuite", TestType::TestSuite, (testSuite == NULL));
            static TestSuite* currentSuite = &defaultTestSuite;
            if (testSuite != NULL)
                currentSuite = testSuite;
            return currentSuite;
        }
        
    protected:
        
        /** The method containing the actual test code, to be overriden. */
        virtual void runTest(TestResult& result) {
            for (std::vector<Test*>::iterator it = mTests.begin(); it != mTests.end(); ++it)
                (*it)->run(result);
        }
        
        std::vector<Test*> mTests;  // the tests that are part of the group
    };
    
#pragma mark -
#pragma mark Fixture definition
    
    /**
     A TestFixture is a basic class encapsulating a set of objects that serves as a base for a set of tests.
     The setup/teardown mechanism of a fixture is implemented through the use use of standard c++ constructor and destructor.
     You just have to declare a fixture class on the stack at the beginning of a test to use it.
     
     The convenience macros FIXTURE, SETUP, TEARDOWN and USE_FIXTURE are meant to simplify declaration and usage of test fixtures.
     @see ntk::Test
     */
    class TestFixture {
    public:
        
        /** Creates the fixture, used as the setup method of the fixture. */
        TestFixture() {}
        
        /** Destroys the fixture, used as the teardown method of the fixture. */
        virtual ~TestFixture() = 0;
        
    private:
        // private copy constructor and assign operator as a fixture must not be copied
        TestFixture(const TestFixture&);
        const TestFixture& operator=(const TestFixture&);
    };
    
    inline TestFixture::~TestFixture() {}   // pure virtual destructor implementation
    
#pragma mark -
#pragma mark Test failure recording
    
    /**
     A TestFailure object records the context information about a test failure.
     C++ macros are used to provide the name of the file and the line number where the failure occurred.
     */
    class TestFailure {
    public:
        
        /** Creates a new failure with the given context information. */
        TestFailure(const std::string& theCondition, const std::string& theTestName, const std::string& TheFileName, int theLine)
        : condition(theCondition), testName(theTestName), fileName(TheFileName), line(theLine)
        {}
        
        /** Stream output operator. */
        friend std::ostream& operator<<(std::ostream& os, const TestFailure& failure) {
            return (os << failure.fileName << "(" << failure.line << "): Failure: \"" << failure.condition << "\"");
        }
        
        std::string condition;  ///< The condition that provoked the failure.
        std::string testName;   ///< The name of the test that failed.
        std::string fileName;   ///< The name of the file in which the test failed.
        int line;               ///< The line number at which the failure occured.
    };        
    
#pragma mark -
#pragma mark Test result processing
    
    /**
     The TestResult class is used to process the test results.
     By deriving from this class you can process these results by overriding methods that are called at various stages of the testing process.
     This base class only provides global timing and records of executed tests and failures.
     @see ntk::OStreamTestResult
     */
    class TestResult {
    public:
        
        /** Creates a new test result. */
        TestResult() 
        : mTestCount(0), mFailureCount(0), mElapsedSeconds(0) 
        {}
        
        /** Destroys the test result. */
        virtual ~TestResult() {}
        
        /** This method is called before running all tests. */
        virtual void allTestsBegin() {
            ::time(&mStartTime);
        }
        
        /** This method is called after all tests have been run. */
        virtual void allTestsEnd() {
            std::time_t endTime;
            ::time(&endTime);
            mElapsedSeconds = endTime - mStartTime;
        }
        
        /** This method is called each time a test begins. */
        virtual void testBegins(Test* test) {
        }
        
        /** This method is called each time a test ends. */
        virtual void testEnds(Test* test) {
            if (test->type() == TestType::TestCase)
                ++mTestCount;
        }
        
        /** This method is called when a test has failed. */
        virtual void addFailure(const TestFailure& failure) {
            ++mFailureCount;
        }
        
        /** Gets the number of test failures. */
        int failures() const { return mFailureCount; }
        
        /** Gets the elapsed time in seconds to run all the tests (set after all tests have been run). */
        int elapsedSeconds() const { return mElapsedSeconds; }
        
    protected:
        int mTestCount;             ///< The number of test executed.
        int mFailureCount;          ///< The number of failures.
        int mElapsedSeconds;        ///< The total elapsed time in seconds.
        std::time_t mStartTime;     ///< The start time of the tests.
    };
    
    /** 
     OStreamResult processes test results to an output stream. 
     @see ntk::TestResult
     */
    class OStreamTestResult : public TestResult
    {
    public:
        
        /** 
         Creates a new OStreamResult from the specified output stream.
         If no output stream if specified, std::cout is used.
         */
        OStreamTestResult(std::ostream& os = std::cout)
        : mOutStream(os), mIndent(0)
        {}
        
        /** This method is called before running all tests. */
        virtual void allTestsBegin() {
            TestResult::allTestsBegin();
            std::cout << std::endl << std::endl << "Running unit tests..." << std::endl << std::endl;
        }
        
        /** This method is called after all tests have been run. */
        virtual void allTestsEnd() {
            TestResult::allTestsEnd();
            
            std::cout << std::endl << "Summary:" << std::endl;
            std::cout << "  - Executed tests : " << std::setw(8) << std::right << mTestCount << std::endl;
            std::cout << "  - Passed tests   : " << std::setw(8) << std::right << (mTestCount - mFailureCount) << std::endl;
            
            if (failures() != 0)
                std::cout << "  - Failed tests   : "  << std::setw(8) << std::right << mFailureCount << std::endl;
            
            std::cout << std::endl << "Tests running time: " << elapsedSeconds() << "s." << std::endl << std::endl;
        }
        
        /** This method is called when a test has failed. */
        virtual void addFailure(const TestFailure& failure) {
            TestResult::addFailure(failure);
            std::cout << std::setw(mIndent) << "! " << failure << std::endl;
        }
        
        /** This method is called each time a test begins. */
        virtual void testBegins(Test* test) {
            TestResult::testBegins(test);
            std::cout << std::setw(mIndent + 2) << ((test->type() == TestType::TestSuite) ? "+ " : "- ") << test->name() << std::endl;
            mIndent += 2;
        }
        
        /** This method is called each time a test ends. */
        virtual void testEnds(Test* test) {
            TestResult::testEnds(test);
            mIndent -= 2;
        }
        
    protected:
        std::ostream& mOutStream;   ///< The output stream.
        unsigned int mIndent;       ///< The current indentation.
        
    private:
        // private copy constructor and assign operator as a stream result can't be copied
        OStreamTestResult(const OStreamTestResult&);
        const OStreamTestResult& operator=(const OStreamTestResult&);
    };
    
#pragma mark -
#pragma mark Test inline implementation
    
    // runs the test, using the specified Test::Result object to process the test results.
    inline int Test::run(TestResult& result) {
        int failuresBeforeTest = result.failures();
        
        result.testBegins(this);
#ifndef DO_NOT_USE_EXCEPTIONS
        try {
            runTest(result);
        } catch (std::exception& e) {
            result.addFailure(TestFailure("Unhandled exception: " + std::string(e.what()), mName, "unknown file", -1));
        } catch (...) {
            result.addFailure(TestFailure("Unhandled exception: unknown", mName, "unknown file", -1));
        }
#else
        runTest(result);
#endif
        result.testEnds(this);
        
        return (result.failures() - failuresBeforeTest);    // return the number of failures that occured during the test
    }

    // runs all the tests, using the specified TestResult object to process the test results.
    inline int Test::runAll(TestResult& result) {
        result.allTestsBegin();
        for (std::vector<Test*>::iterator it = mTests().begin(); it != mTests().end(); ++it)
            (*it)->run(result);
        result.allTestsEnd();
        return result.failures();
    }
    
#pragma mark -
#pragma mark Assertion templates
    
    /** Defines a set of assertion methods to do the test checks. */
    namespace TestCheck {
        
        /** Expresses a failure of a test and gives details about it. */
        inline void fail(const std::string& expression, const std::string& message,
                  TestResult& result, const std::string& testName, const std::string& file, int line) 
        {
            std::string condition = expression;
            if (!message.empty()) {
                condition += ", Note: ";
                condition += message;
            }
            result.addFailure(TestFailure(condition, testName, file, line));
        }
        
        /** Returns true if x and y are equal. */
        template <typename X, typename Y>
        bool equal(X x, Y y) {
            return (x == y);
        }
                
        /** Returns true if x and y of floating point type are equal. */
        template <typename X, typename Y>
        bool differ(X x, Y y) {
            return !(x == y);
        }
        
        /** Returns true if x and y are close by a maximum delta of d. */
        template <typename X, typename Y, typename D>
        bool close(X x, Y y, D d) {
            return (((y - x) < d) && ((y - x) > -d));
        }
        
        /** Returns true if x is less than y. */
        template <typename X, typename Y>
        bool less(X x, Y y) {
            return (x < y);
        }
        
        /** Returns true if x is less or equal than y. */
        template <typename X, typename Y>
        bool lessOrEqual(X x, Y y) {
            return (x <= y);
        }
        
        /** Returns true if x is more than y. */
        template <typename X, typename Y>
        bool more(X x, Y y) {
            return (x > y);
        }
        
        /** Returns true if x is more or equal than y. */
        template <typename X, typename Y>
        bool moreOrEqual(X x, Y y) {
            return (x >= y);
        }
        
        /** Returns true if the data of the specified size at position x and y is the same.*/
        inline bool sameData(const void* x, const void* y, unsigned int size) {
            if (size == 0)
                return true;
            
            if (x == y)
                return true;
            
            if ((x == NULL) || (y == NULL))
                return false;
            
            const char* cx = (const char *)x;
            const char* cy = (const char *)y;
            while (size--)
                if (*cx++ != *cy++)
                    return false;
            
            return true;
        }
        
        /** Returns a string corresponding to the specified value. */
        template <typename T>
        std::string stringValue(const T& value) {
            std::ostringstream ss;
            ss << value;
            return ss.str();
        }
        
    }
    
#pragma mark -
#pragma mark Signal -> exception handling
    
#if !defined (DO_NOT_USE_EXCEPTIONS) && !defined (DO_NOT_CATCH_SIGNALS)
    
    /** Creates a new signal exception class. */
#   define SIGNAL_EXCEPTION(signalNumber, name) \
    class name##Exception : public std::exception { \
    public: \
        static int signalType() { return signalNumber; } \
        const char* what() const throw() { return (#name "Exception"); } \
    }
    
    /** Setup a signal to exception translator. */
#   define SIGNAL_EXCEPTION_SETUP(name) \
    static TestSignalTranslator<name##Exception> __##name##ExceptionTranslator; 

    /** Setup the global signal to exception handler (called automatically by the TEST macro to avoid forgetting it). */
#   define SETUP_EXCEPTIONS()  ntk::TestSignalExceptionHandler::setup()
    
    /** Signal to exception translator template helper. */
    template <class SignalExceptionClass> 
    class TestSignalTranslator
    {
    private:
        // singleton translator setup
        struct Translator 
        {
            Translator() {
                ::signal(SignalExceptionClass::signalType(), SignalHandler);
            }
            
            static void SignalHandler(int) {
                throw SignalExceptionClass();
            }
        };
    public:
        /** Setup the signal translator. */
        TestSignalTranslator() {
            static Translator translator;
        }
    };
    
    // declare all signal exception classes
    SIGNAL_EXCEPTION(SIGTERM, Termination);
    SIGNAL_EXCEPTION(SIGABRT, Abort);
    SIGNAL_EXCEPTION(SIGSEGV, SegmentationFault);
    SIGNAL_EXCEPTION(SIGFPE, FloatingPoint);
    SIGNAL_EXCEPTION(SIGILL, IllegalInstruction);
    SIGNAL_EXCEPTION(SIGINT, Interrupt);            
#ifdef SIGBUS
    SIGNAL_EXCEPTION(SIGBUS, BadAccess);            
#endif
#ifdef SIGSYS
    SIGNAL_EXCEPTION(SIGSYS, BadSystemCall);            
#endif
#ifdef SIGKILL
    SIGNAL_EXCEPTION(SIGKILL, Kill);            
#endif
    
    /** Helper class to setup the signal to exception handler. */
    struct TestSignalExceptionHandler {
        inline static void setup() {
            SIGNAL_EXCEPTION_SETUP(Termination);
            SIGNAL_EXCEPTION_SETUP(Abort);
            SIGNAL_EXCEPTION_SETUP(SegmentationFault);
            SIGNAL_EXCEPTION_SETUP(FloatingPoint);
            SIGNAL_EXCEPTION_SETUP(IllegalInstruction);
            SIGNAL_EXCEPTION_SETUP(Interrupt);            
#ifdef SIGBUS
            SIGNAL_EXCEPTION_SETUP(BadAccess);            
#endif
#ifdef SIGSYS
            SIGNAL_EXCEPTION_SETUP(BadSystemCall);            
#endif
#ifdef SIGKILL
            SIGNAL_EXCEPTION_SETUP(Kill);            
#endif
        }
    };
        
#else

#   define SETUP_EXCEPTIONS()
    
#endif // !defined (DO_NOT_USE_EXCEPTIONS) && !defined (DO_NOT_CATCH_SIGNALS)
    
#pragma mark -
#pragma mark Internal helper macros
    
#   define __T_FAIL(condition, message)    ntk::TestCheck::fail(condition, message, result, name(), __FILE__, __LINE__)
#   define __T_VAL(value)                  ntk::TestCheck::stringValue(value)
    
#   define __T_CHECK2(testFunction, operandString, x, y, message) \
    if (!ntk::TestCheck::testFunction(x, y)) { \
        __T_FAIL(#x " (" + __T_VAL(x) + ") " + std::string(operandString) + " " #y " (" + __T_VAL(y) + ")", message); \
        return; \
    }
    
#   define __T_CHECK3(testFunction, operandString1, operandString2, x, y, z, message) \
    if (!ntk::TestCheck::testFunction(x, y, z)) { \
        __T_FAIL(#x " (" + __T_VAL(x) + ") " + std::string(operandString1) + " " #y " (" + __T_VAL(y) + ") " \
                 + std::string(operandString2) + " " #z " (" + __T_VAL(z) + ")", message); \
        return; \
    }
    
#   define __T_CHECK3_NOPRINT(testFunction, operandString1, operandString2, x, y, z, message) \
    if (!ntk::TestCheck::testFunction(x, y, z)) { \
        __T_FAIL(#x " " + std::string(operandString1) + " " #y " " + std::string(operandString2) + " " + #z, message); \
        return; \
    }
    
#   define __T_MULTILINE_BEGIN  do {
#   define __T_MULTILINE_END    } while(0)
    
#ifndef DO_NOT_USE_EXCEPTIONS
    
#   define __E_TRY \
    __T_MULTILINE_BEGIN \
    try {
#   define __E_CATCH \
    } catch (std::exception& e) { \
        __T_FAIL("Unhandled exception: " + std::string(e.what()), ""); \
        return ; \
    } catch (...) { \
        __T_FAIL("Unhandled exception: unknown", ""); \
        return; \
    } \
    __T_MULTILINE_END
    
#else
    
#   define __E_TRY      __T_MULTILINE_BEGIN
#   define __E_CATCH    __T_MULTILINE_END
    
#endif // DO_NOT_USE_EXCEPTIONS
    
#pragma mark -
#pragma mark Test helper macros
    
    /**
     Helper macro for creating a test.
     Usage example:
     @code
     TEST(MyTestName) {
         // test code goes here
     }
     @endcode
     */
#   define TEST(testName) \
    class testName##_Test : public ntk::Test { \
    public: \
        testName##_Test() : ntk::Test(#testName) { ntk::TestSuite::currentTestSuite(NULL)->addTest(this); } \
    protected: \
        void testImplementation(ntk::TestResult& result); \
        virtual void runTest(ntk::TestResult& result) { SETUP_EXCEPTIONS(); __E_TRY testImplementation(result); __E_CATCH; } \
    } testName##_Test_Instance; \
    void testName##_Test::testImplementation(ntk::TestResult& result)
    
    /**
     Helper macro for creating a test fixture.
     Usage example:
     @code
     // define a fixture
     FIXTURE(MyFixture) {
        
         // declare variables here
         int myVar;
     
         // optional setup
         SETUP(MyFixture) {
             // setup code goes here
             myVar = 1;
         }
     
         // optional teardown
         TEARDOWN(MyFixture) {
             // teardown code goes here
         }
     
     }; // don't forget the ";" here, a fixture is a class
     
     // use fixture in a test
     TEST(MyTest) {
          USE_FIXTURE(MyFixture);   // use our fixture
          T_ASSERT(F.myVar == 1);   // do not forget the "F." prefix to access the fixture variables
     }
     @endcode
     */
#   define FIXTURE(fixtureName) \
    struct fixtureName##_TestFixture : public ntk::TestFixture
    
    /**
     Helper macro for declaring fixture setup method (optional).
     @see FIXTURE
     */
#   define SETUP(fixtureName) \
    fixtureName##_TestFixture()
    
    /**
     Helper macro for declaring fixture teardown method (optional).
     @see FIXTURE
     */
#   define TEARDOWN(fixtureName) \
    ~fixtureName##_TestFixture()

    /**
     Helper macro to use a previously defined fixture in a test.
     The fixture members are available using the prefix "F.".
     @see FIXTURE
     */
#   define USE_FIXTURE(fixtureName) \
    fixtureName##_TestFixture F;
    
    /**
     Helper macro to declare a test suite.
     Until a new test suite is declared, all tests declared after are considered as a part of this suite.
     Usage example:
     @code
     // declare a test suite
     SUITE(MySuite);
     
     // all tests declared from here will be a part of MySuite
     @endcode
     */
#   define SUITE(suiteName) \
    class suiteName##_TestSuite : public ntk::TestSuite { \
    public: \
        suiteName##_TestSuite() : ntk::TestSuite(#suiteName) { ntk::TestSuite::currentTestSuite(this); } \
    } suiteName##_TestSuite_Instance
    
    /**
     Helper macro to declare a sub test suite, i.e. a test suite that is part of a test suite.
     Until a new test suite is declared, all tests declared after are considered as a part of this suite.
     A sub suite can also be a part of another sub suite, allowing complex test hierarchies.
     Usage example:
     @code
     // declare a test suite
     SUITE(MySuite);
     
     // declare a sub test suite
     SUBSUITE(MySuite, MySubSuite)
     
     // all tests declared from here will be a part of MySubSuite
     @endcode
     */
#   define SUBSUITE(parentSuiteName, subSuiteName) \
    class subSuiteName##_TestSuite : public ntk::TestSuite { \
    public: \
        subSuiteName##_TestSuite() : ntk::TestSuite(#subSuiteName, ntk::TestType::TestSuite, false) { \
            parentSuiteName##_TestSuite_Instance.addTest(this); \
            ntk::TestSuite::currentTestSuite(this); \
        } \
    } subSuiteName##_TestSuite_Instance

    /**
     Helper macro to create a main() function that will run all the tests, using the result class provided.
     This macro must be used only once, at the end of the tests.
     Usage example:
     @code
     ... // declare some tests
     
     RUN_TESTS(OStreamTestResult);  // run all the test using std::cout as an output for the results.
     @endcode
     */
#   define RUN_TESTS(resultClassName) \
    int main(int argc, char* argv[]) { \
        resultClassName results;\
        return ntk::Test::runAll(results);\
    }\
    
#pragma mark -
#pragma mark Assertion macros

    /** Asserts that the specified predicate is true. An additional explanation message may be provided. */
#   define TM_CHECK(predicate, message) \
    __E_TRY \
    if (!(predicate)) { \
        __T_FAIL(#predicate, message); \
        return; \
    } \
    __E_CATCH
#   define T_CHECK(predicate)       TM_CHECK(predicate, "")     ///< Same as TM_CHECK, without message.
    
    /** Asserts that the values of x and y are equal. An additional explanation message may be provided. */
#   define TM_CHECK_EQUAL(x, y, message) \
    __E_TRY \
    __T_CHECK2(equal, "==", x, y, message) \
    __E_CATCH 
#   define T_CHECK_EQUAL(x, y)      TM_CHECK_EQUAL(x, y, "")    ///< Same as TM_CHECK_EQUAL, without message.
    
    /** Asserts that the values of x and y differ. An additional explanation message may be provided. */
#   define TM_CHECK_DIFFER(x, y, message) \
    __E_TRY \
    __T_CHECK2(differ, "!=", x, y, message) \
    __E_CATCH 
#   define T_CHECK_DIFFER(x, y)     TM_CHECK_DIFFER(x, y, "")   ///< Same as TM_CHECK_DIFFER, without message.

    /** Asserts that the values of x and y are close, with a maximum delta of d. An additional explanation message may be provided. */
#   define TM_CHECK_CLOSE(x, y, d, message) \
    __E_TRY \
    __T_CHECK3(close, "close to", "with delta", x, y, d, message) \
    __E_CATCH 
#   define T_CHECK_CLOSE(x, y, d)   TM_CHECK_CLOSE(x, y, d, "") ///< Same as TM_CHECK_CLOSE, without message.

    /** Asserts that the value of x is less than y. An additional explanation message may be provided. */
#   define TM_CHECK_LESS_THAN(x, y, message) \
    __E_TRY \
    __T_CHECK2(less, "<", x, y, message) \
    __E_CATCH 
#   define T_CHECK_LESS_THAN(x, y)  TM_CHECK_LESS_THAN(x, y, "")    ///< Same as TM_CHECK_LESS_THAN, without message.
  
    /** Asserts that the value of x is less or equal than y. An additional explanation message may be provided. */
#   define TM_CHECK_LESS_OR_EQUAL(x, y, message) \
    __E_TRY \
    __T_CHECK2(lessOrEqual, "<=", x, y, message) \
    __E_CATCH 
#   define T_CHECK_LESS_OR_EQUAL(x, y)  TM_CHECK_LESS_OR_EQUAL(x, y, "")    ///< Same as TM_CHECK_LESS_OR_EQUAL, without message.

    /** Asserts that the value of x is more than y. An additional explanation message may be provided. */
#   define TM_CHECK_MORE_THAN(x, y, message) \
    __E_TRY \
    __T_CHECK2(more, ">", x, y, message) \
    __E_CATCH 
#   define T_CHECK_MORE_THAN(x, y)  TM_CHECK_MORE_THAN(x, y, "")    ///< Same as TM_CHECK_MORE_THAN, without message.

    /** Asserts that the value of x is more or equal than y. An additional explanation message may be provided. */
#   define TM_CHECK_MORE_OR_EQUAL(x, y, message) \
    __E_TRY \
    __T_CHECK2(moreOrEqual, ">=", x, y, message) \
    __E_CATCH 
#   define T_CHECK_MORE_OR_EQUAL(x, y)  TM_CHECK_MORE_OR_EQUAL(x, y, "")    ///< Same as TM_CHECK_MORE_OR_EQUAL, without message.

    /** Asserts that the objects x and y of size s have the same data. An additional explanation message may be provided. */
#   define TM_CHECK_SAME_DATA(x, y, s, message) \
    __E_TRY \
    __T_CHECK3_NOPRINT(sameData, "has same data as", "with size", x, y, s, message) \
    __E_CATCH 
#   define T_CHECK_SAME_DATA(x, y, s)   TM_CHECK_SAME_DATA(x, y, s, "") ///< Same as TM_CHECK_SAME_DATA, without message.

    /** Asserts that the specified method throws an exception of the specified type. An additional explanation message may be provided. */
#   define TM_CHECK_THROWS(method, exception, message) \
    __E_TRY \
    try { \
        method; \
        __T_FAIL(#method " throws exception " #exception, message); \
        return; \
    } catch(exception) {} \
    __E_CATCH 
#   define T_CHECK_THROWS(method, exception)   TM_CHECK_THROWS(method, exception, "")  ///< Same as TM_CHECK_THROWS, without message.
    
    /** Asserts that the specified method throws any exception. An additional explanation message may be provided. */
#   define TM_CHECK_THROWS_ANY(method, message) \
    __T_MULTILINE_BEGIN \
    try { \
        method; \
        __T_FAIL(#method " throws any exception", message); \
        return; \
    } catch(...) {} \
    __T_MULTILINE_END
#   define T_CHECK_THROWS_ANY(method)   TM_CHECK_THROWS_ANY(method, "") ///< Same as TM_CHECK_THROWS_ANY, without message.
    
    /** Asserts that the specified method does not throw any exception. An additional explanation message may be provided. */
#   define TM_CHECK_NOTHROW(method, message) \
    __T_MULTILINE_BEGIN \
    try { \
        method; \
    } catch(...) { \
        __T_FAIL(#method " does not throw exception", message); \
        return; \
    } \
    __T_MULTILINE_END
#   define T_CHECK_NOTHROW(method)  TM_CHECK_NOTHROW(method, "")    ///< Same as TM_CHECK_NOTHROW, without message.
    
    /** Explicitely fails the current test. An additional explanation message may be provided. */
#   define TM_CHECK_FAIL(message) \
    __T_MULTILINE_BEGIN \
    __T_FAIL("Explicit failure", message); \
    return; \
    __T_MULTILINE_END
#   define T_CHECK_FAIL()           TM_CHECK_FAIL("")   ///< Same as TM_CHECK_FAIL, without message.
        
} // namespace ntk

#endif // __ntk_test_hpp__
