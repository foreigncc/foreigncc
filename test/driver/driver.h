#ifndef _FOREIGNCC_TEST_DRIVER_H_INCLUDED_
#define _FOREIGNCC_TEST_DRIVER_H_INCLUDED_

#include <common/common.h>
#include "assertion.h"

#include <vector>

namespace test
{
    struct TestCase
    {
        typedef void (*PTestNoInput)();
        typedef void (*PTestWithInput)(const std::string& input);

        explicit TestCase(const PTestNoInput func, const char* name)
            : func_no_input(func),
              name(name)
        {
            ASSERT_NOT_NULL(name);
            ASSERT_NOT_NULL(func);
        }

        explicit TestCase(const PTestWithInput func, const char* name)
            : func_with_input(func),
              name(name)
        {
            ASSERT_NOT_NULL(name);
            ASSERT_NOT_NULL(func);
        }

        TestCase& Description(const char* description)
        {
            this->description = description;
            return *this;
        }

        TestCase& AddInput(const char* input)
        {
            this->inputs.emplace_back(input);
            return *this;
        }

    public:
        PTestNoInput const func_no_input = nullptr;
        PTestWithInput const func_with_input = nullptr;
        const char* const name;
        std::vector<std::string> inputs;
        std::string description;
    };

    struct TestCaseReceiver
    {
        TestCaseReceiver(TestCase&& test)
        {
            tests.emplace_back(std::move(test));
        }

        static std::vector<TestCase> tests;
    };
}

#define DECLARE_UNITTEST(_Test) \
    static ::test::TestCaseReceiver _declare_test_##_Test = ::test::TestCase(_Test, #_Test)


#endif  // !_FOREIGNCC_TEST_DRIVER_H_INCLUDED_
