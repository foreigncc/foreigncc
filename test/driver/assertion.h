#ifndef _FOREIGNCC_TEST_DRIVER_ASSERTION_H_INCLUDED_
#define _FOREIGNCC_TEST_DRIVER_ASSERTION_H_INCLUDED_

#include <cstdarg>

namespace test
{
    // TestException doesn't inherit std::exception intentionally
    struct TestException {
        const std::string message;
        explicit TestException(std::string&& message) noexcept
            : message(std::move(message))
        { }
    };


    CPP_NO_RETURN
    CPP_NO_INLINE
    static void Fail(
        const char* const assertion,
        const char* const file,
        int const line,
        uint32_t const internal_msgs_count = 0,
        const char* const* const internal_msgs = nullptr,
        const char* const user_msg = nullptr,
        .../*user_msg args*/)
    {
        char buffer[1024];
        uint32_t len = 0;  // in range [0, ::get_array_size(buffer) - 1] (except tailing \n)
        auto const concat = [&](const char* const s, uint32_t const len_s) noexcept {
            if (len_s < ::get_array_size(buffer) - len) {
                memcpy(buffer + len, s, sizeof(char) * len_s);
                len += len_s;
            }
            else {  // len_s >= ::get_array_size(buffer) - len
                memcpy(buffer + len, s, sizeof(char) * (::get_array_size(buffer) - len - 1));
                len = ::get_array_size(buffer) - 1;
            }
            //buffer[len] = '\n';
        };

        std::string const line_str = std::to_string(line);

#define _TMP_CONCAT(_Str)  concat((_Str), ::get_str_length(_Str))
        _TMP_CONCAT("[TEST] (");
        concat(&file[::common::SourceFilePathPrefixLength], (uint32_t)strlen(file) - ::common::SourceFilePathPrefixLength);
        _TMP_CONCAT(":");
        concat(line_str.c_str(), (uint32_t)line_str.size());
        _TMP_CONCAT(") ");
        concat(assertion, (uint32_t)strlen(assertion));
        _TMP_CONCAT(". ");

        if (internal_msgs_count > 0) {
            for (uint32_t i = 0; i < internal_msgs_count; ++i) {
                concat(internal_msgs[i], (uint32_t)strlen(internal_msgs[i]));
            }
            _TMP_CONCAT(". ");
        }

        if (user_msg) {
            va_list args;
            va_start(args, user_msg);
            int const msg_len = vsnprintf(buffer + len, ::get_array_size(buffer) - len, user_msg, args);
            if (msg_len < 0) {
                // Don't know what happened...
                _TMP_CONCAT("(?)");
            }
            else if ((uint32_t)msg_len < ::get_array_size(buffer) - len) {
                len += msg_len;
                _TMP_CONCAT(".");
            }
            else {  // msg_len >= ::get_array_size(buffer) - len
                len = ::get_array_size(buffer) - 1;
            }
            va_end(args);
        }
#undef _TMP_CONCAT

        buffer[len++] = '\n';
        fwrite(buffer, sizeof(char), len, stderr);
        fflush(stderr);

        throw TestException(std::string(buffer, len));
    }
}


#define TEST_ASSERT(_Expr, ...) \
    do { \
        if (!(_Expr)) { \
            ::test::Fail("TEST_ASSERT(" #_Expr ")", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
        } \
    } while(false)

#define TEST_ASSERT_NOT_NULL(_Expr, ...) \
    do { \
        if ((_Expr) == nullptr) { \
            ::test::Fail("TEST_ASSERT_NOT_NULL(" #_Expr ")", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
        } \
    } while(false)

#define TEST_ASSERT_IS_NULL(_Expr, ...) \
    do { \
        if ((_Expr) != nullptr) { \
            ::test::Fail("TEST_ASSERT_IS_NULL(" #_Expr ")", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
        } \
    } while(false)

#define TEST_UNEXPECTED(...) \
    do { \
        ::test::Fail("TEST_UNEXPECTED", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
    } while(false)

#define TEST_UNEXPECTED_REACH_HERE(...) \
    do { \
        ::test::Fail("TEST_UNEXPECTED_REACH_HERE: should not reach here", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
    } while(false)


#define _TEST_ASSERT_COMPARE(_StrAssert, _Left, _Right, _StrLeft, _StrRight, _Comp, ...) \
    do { \
        typename ::unwrap<decltype(_Left)>::cref left_value = (_Left); \
        typename ::unwrap<decltype(_Right)>::cref right_value = (_Right); \
        if (!((_Comp)(left_value, right_value))) { \
            const std::string left_value_str = ::stringify(left_value); \
            const std::string right_value_str = ::stringify(right_value); \
            const char* const internal_msgs[] = { \
                "LHS: " _StrLeft " = ", left_value_str.c_str(), ", RHS: " _StrRight " = ", right_value_str.c_str() \
            }; \
            ::test::Fail(_StrAssert "(" _StrLeft ", " _StrRight ")", __FILE__, __LINE__, ::get_array_size(internal_msgs), internal_msgs, ##__VA_ARGS__); \
        } \
    } while(false)

#define TEST_ASSERT_LT(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_LT", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::less()), ##__VA_ARGS__)
#define TEST_ASSERT_LE(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_LE", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::less_equal()), ##__VA_ARGS__)
#define TEST_ASSERT_GT(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_GT", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::greater()), ##__VA_ARGS__)
#define TEST_ASSERT_GE(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_GE", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::greater_equal()), ##__VA_ARGS__)
#define TEST_ASSERT_EQ(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_EQ", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::equal_to()), ##__VA_ARGS__)
#define TEST_ASSERT_NE(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_NE", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::not_equal_to()), ##__VA_ARGS__)
#define TEST_ASSERT_AND(_Left, _Right, ...) _TEST_ASSERT_COMPARE("TEST_ASSERT_AND", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::logic_and()), ##__VA_ARGS__)
#define TEST_ASSERT_OR(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_OR", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::logic_or()), ##__VA_ARGS__)
#define TEST_ASSERT_XOR(_Left, _Right, ...) _TEST_ASSERT_COMPARE("TEST_ASSERT_XOR", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::logic_xor()), ##__VA_ARGS__)

#define TEST_ASSERT_STR_LT(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_STR_LT", (_Left), (_Right), #_Left, #_Right, (::str_comparer::less()), ##__VA_ARGS__)
#define TEST_ASSERT_STR_LE(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_STR_LE", (_Left), (_Right), #_Left, #_Right, (::str_comparer::less_equal()), ##__VA_ARGS__)
#define TEST_ASSERT_STR_GT(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_STR_GT", (_Left), (_Right), #_Left, #_Right, (::str_comparer::greater()), ##__VA_ARGS__)
#define TEST_ASSERT_STR_GE(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_STR_GE", (_Left), (_Right), #_Left, #_Right, (::str_comparer::greater_equal()), ##__VA_ARGS__)
#define TEST_ASSERT_STR_EQ(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_STR_EQ", (_Left), (_Right), #_Left, #_Right, (::str_comparer::equal_to()), ##__VA_ARGS__)
#define TEST_ASSERT_STR_NE(_Left, _Right, ...)  _TEST_ASSERT_COMPARE("TEST_ASSERT_STR_NE", (_Left), (_Right), #_Left, #_Right, (::str_comparer::not_equal_to()), ##__VA_ARGS__)

#endif // _FOREIGNCC_TEST_DRIVER_ASSERTION_H_INCLUDED_
