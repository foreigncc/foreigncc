#ifndef _FOREIGNCC_COMMON_ASSERTION_H_INCLUDED_
#define _FOREIGNCC_COMMON_ASSERTION_H_INCLUDED_

#ifndef _FOREIGNCC_COMMON_COMMON_H_INCLUDED_
#error "Do not include <common/assertion.h> directly. Include <common/common.h> instead."
#endif // _FOREIGNCC_COMMON_COMMON_H_INCLUDED_

#include <cstdarg>
#include <cstring>
#include <cerrno>
#include <array>


namespace common
{
#if !BUILD_UNITTEST_ASSERTIONS
    CPP_NO_RETURN
#endif
    CPP_NO_INLINE
    static void Panic(
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
        _TMP_CONCAT("Panic! (");
        concat(&file[SourceFilePathPrefixLength], (uint32_t)strlen(file) - SourceFilePathPrefixLength);
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

#if !BUILD_UNITTEST_ASSERTIONS
        exit(1);
#else
        extern int assertion_failure_count;
        ++assertion_failure_count;
#endif
    }

}  // namespace common


#define ASSERT(_Expr, ...) \
    (([&]() { \
        if (!(_Expr)) { \
            ::common::Panic("ASSERT(" #_Expr ")", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
        } \
    })())

#define ASSERT_NOT_NULL(_Expr, ...) \
    (([&]() { \
        if ((_Expr) == nullptr) { \
            ::common::Panic("ASSERT_NOT_NULL(" #_Expr ")", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
        } \
    })())

#define ASSERT_IS_NULL(_Expr, ...) \
    (([&]() { \
        if ((_Expr) != nullptr) { \
            ::common::Panic("ASSERT_IS_NULL(" #_Expr ")", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
        } \
    })())

#define PANIC(...) \
    (([&]() { \
        ::common::Panic("PANIC", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
    })())

#define UNEXPECTED_REACH_HERE(...) \
    (([&]() { \
        ::common::Panic("UNEXPECTED_REACH_HERE: should not reach here", __FILE__, __LINE__, 0, nullptr, ##__VA_ARGS__); \
    })())

#define CALL_C(_Expr, ...) \
    (([&]() -> decltype(_Expr) { \
        auto const ret = (_Expr); \
        if (ret < 0) { \
            int const err = errno; \
            std::string ret_str = ::stringify(ret); \
            std::string errno_str = ::stringify(err); \
            const char* const internal_msgs[] = { \
                #_Expr " returns ", ret_str.c_str(), ", errno = ", errno_str.c_str(), " (", strerror(err), ")", \
            }; \
            ::common::Panic("CALL_C(" #_Expr ")", __FILE__, __LINE__, ::get_array_size(internal_msgs), internal_msgs, ##__VA_ARGS__); \
        } \
        return ret; \
    })())

#define CALL_C_EXCEPT(_Expr, _AllowErrs, ...) \
    (([&]() -> decltype(_Expr) { \
        auto const ret = (_Expr); \
        if (ret < 0) { \
            int const err = errno; \
            /* _AllowErrs is expected to be surrounded with { } */ \
            const int allow_errs[] = _AllowErrs; \
            bool expected = false; \
            for (size_t ii = 0; ii < ::get_array_size(allow_errs); ++ii) { \
                if (allow_errs[ii] == err) { \
                    expected = true; \
                    break; \
                } \
            } \
            if (!expected) { \
                std::string ret_str = ::stringify(ret); \
                std::string errno_str = ::stringify(err); \
                const char* const internal_msgs[] = { \
                    #_Expr " returns ", ret_str.c_str(), ", errno = ", errno_str.c_str(), " (", strerror(err), ")", \
                }; \
                ::common::Panic("CALL_C_EXCEPT(" #_Expr ", " #_AllowErrs ")", __FILE__, __LINE__, ::get_array_size(internal_msgs), internal_msgs, ##__VA_ARGS__); \
            } \
        } \
        return ret; \
    })())

#if BOOST_OS_WINDOWS
#define CALL_WIN(_Expr, ...) \
    (([&]() -> decltype(_Expr) { \
        auto const ret = (_Expr); \
        if (!ret) { \
            int const err = GetLastError(); \
            std::string ret_str = ::stringify(ret); \
            std::string err_str = ::stringify(err); \
            LPSTR messageBuffer = nullptr; \
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
                nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL); \
            std::string message; \
            if (size == 0) { \
                message = "(no message)"; \
            } \
            else { \
                /* Trim \r\n at the end of Win32 error message */ \
                if (size > 0 && messageBuffer[size - 1] == '\n') { \
                    --size; \
                    if (size > 0 && messageBuffer[size - 1] == '\r') { \
                        --size; \
                    } \
                } \
                message = std::string(messageBuffer, size); \
            } \
            const char* const internal_msgs[] = { \
                #_Expr " returns ", ret_str.c_str(), ", GetLastError() = ", err_str.c_str(), " (", message.c_str(), ")", \
            }; \
            ::common::Panic("CALL_WIN(" #_Expr ")", __FILE__, __LINE__, ::get_array_size(internal_msgs), internal_msgs, ##__VA_ARGS__); \
            if (size != 0) { \
                LocalFree(messageBuffer); \
            } \
        } \
        return ret; \
    })())
#endif  // BOOST_OS_WINDOWS

#define CALL_TRUE(_Expr, ...) \
    (([&]() -> decltype(_Expr) { \
        auto const ret = (_Expr); \
        if (!(ret)) { \
            int const err = errno; \
            std::string ret_str = ::stringify(ret); \
            std::string errno_str = ::stringify(err); \
            const char* const internal_msgs[] = { \
                #_Expr " returns ", ret_str.c_str(), ", errno = ", errno_str.c_str(), " (" strerror(errno), ")", \
            }; \
            ::common::Panic("CALL_TRUE(" #_Expr ")", __FILE__, __LINE__, internal_msgs, ::get_array_size(internal_msgs), ##__VA_ARGS__); \
        } \
        return ret; \
    })())


#define _ASSERT_COMPARE(_StrAssert, _Left, _Right, _StrLeft, _StrRight, _Comp, ...) \
    (([&](typename ::unwrap<decltype(_Left)>::cref left_value, \
         typename ::unwrap<decltype(_Right)>::cref right_value) { \
        if (!((_Comp)(left_value, right_value))) { \
            const std::string left_value_str = ::stringify(left_value); \
            const std::string right_value_str = ::stringify(right_value); \
            const char* const internal_msgs[] = { \
                "LHS: " _StrLeft " = ", left_value_str.c_str(), ", RHS: " _StrRight " = ", right_value_str.c_str() \
            }; \
            ::common::Panic(_StrAssert "(" _StrLeft ", " _StrRight ")", __FILE__, __LINE__, ::get_array_size(internal_msgs), internal_msgs, ##__VA_ARGS__); \
        } \
    })((_Left), (_Right)))

#define ASSERT_LT(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_LT", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::less()), ##__VA_ARGS__)
#define ASSERT_LE(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_LE", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::less_equal()), ##__VA_ARGS__)
#define ASSERT_GT(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_GT", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::greater()), ##__VA_ARGS__)
#define ASSERT_GE(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_GE", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::greater_equal()), ##__VA_ARGS__)
#define ASSERT_EQ(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_EQ", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::equal_to()), ##__VA_ARGS__)
#define ASSERT_NE(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_NE", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::not_equal_to()), ##__VA_ARGS__)
#define ASSERT_AND(_Left, _Right, ...) _ASSERT_COMPARE("ASSERT_AND", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::logic_and()), ##__VA_ARGS__)
#define ASSERT_OR(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_OR", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::logic_or()), ##__VA_ARGS__)
#define ASSERT_XOR(_Left, _Right, ...) _ASSERT_COMPARE("ASSERT_XOR", (_Left), (_Right), #_Left, #_Right, (::general_comparer<decltype(_Left), decltype(_Right)>::logic_xor()), ##__VA_ARGS__)

#define ASSERT_STR_LT(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_STR_LT", (_Left), (_Right), #_Left, #_Right, (::str_comparer::less()), ##__VA_ARGS__)
#define ASSERT_STR_LE(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_STR_LE", (_Left), (_Right), #_Left, #_Right, (::str_comparer::less_equal()), ##__VA_ARGS__)
#define ASSERT_STR_GT(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_STR_GT", (_Left), (_Right), #_Left, #_Right, (::str_comparer::greater()), ##__VA_ARGS__)
#define ASSERT_STR_GE(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_STR_GE", (_Left), (_Right), #_Left, #_Right, (::str_comparer::greater_equal()), ##__VA_ARGS__)
#define ASSERT_STR_EQ(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_STR_EQ", (_Left), (_Right), #_Left, #_Right, (::str_comparer::equal_to()), ##__VA_ARGS__)
#define ASSERT_STR_NE(_Left, _Right, ...)  _ASSERT_COMPARE("ASSERT_STR_NE", (_Left), (_Right), #_Left, #_Right, (::str_comparer::not_equal_to()), ##__VA_ARGS__)

#endif // _FOREIGNCC_COMMON_ASSERTION_H_INCLUDED_
