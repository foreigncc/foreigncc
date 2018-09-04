#include <common/common.h>
#include "driver.h"

using test::TestCaseReceiver;
using test::TestCase;
using test::TestException;

std::vector<TestCase> TestCaseReceiver::tests;

static bool RunTestCaseOneInput(const TestCase& test, uint32_t input_idx)
{
    try {
        ASSERT_XOR(test.func_no_input != nullptr, test.func_with_input != nullptr);
        if (test.func_no_input) {
            test.func_no_input();
        }
        else {
            ASSERT_GE(input_idx, 1u);
            ASSERT_LE(input_idx, test.inputs.size());
            test.func_with_input(test.inputs[input_idx]);
        }
        return true;
    }
    catch (TestException& ex) {
        fprintf(stderr, "Message: %s\n", ex.message.c_str());
        return false;
    }
}

static bool RunTestCase(uint32_t const index, uint32_t const all_count)
{
    bool success = true;

    ASSERT_GE(index, 1u);
    ASSERT_LE(index, TestCaseReceiver::tests.size());
    const TestCase& test = TestCaseReceiver::tests[index - 1];

    ASSERT_XOR(test.func_no_input != nullptr, test.func_with_input != nullptr);
    if (test.func_no_input) {
        fprintf(stderr, "\n================================================\n");
        fprintf(stderr, "(%u/%u) %s\n", index, all_count, test.name);
        fprintf(stderr, "================================================\n");
        success = RunTestCaseOneInput(test, 0);
        fprintf(stderr, "\n%s: %s\n", test.name, success ? "SUCCEEDED" : "FAILED");
    }
    else {
        uint32_t const input_count = (uint32_t)test.inputs.size();
        ASSERT_GT(input_count, 0u);
        for (uint32_t input_idx = 1; input_idx <= test.inputs.size(); ++input_idx) {
            fprintf(stderr, "\n================================================\n");
            fprintf(stderr, "(%u/%u, input %u/%u) %s\n", index, all_count, input_idx, input_count, test.name);
            fprintf(stderr, "================================================\n");
            bool const one_success = RunTestCaseOneInput(test, input_idx);
            fprintf(stderr, "\n%s (input %u/%u): %s\n", test.name, input_idx, input_count, one_success ? "SUCCEEDED" : "FAILED");
            success &= one_success;
        }
    }

    return success;
}

int main(int argc, char** argv)
{
    bool all_success = true;

    uint32_t const all_count = (uint32_t)TestCaseReceiver::tests.size();
    for (uint32_t index = 1; index <= all_count; ++index) {
        bool const success = RunTestCase(index, all_count);
        all_success &= success;
    }

    return (all_success ? 0 : 1);
}
