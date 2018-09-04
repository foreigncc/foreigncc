#ifndef _FOREIGNCC_COMMON_BASIC_H_INCLUDED_
#define _FOREIGNCC_COMMON_BASIC_H_INCLUDED_

#ifndef _FOREIGNCC_COMMON_COMMON_H_INCLUDED_
#error "Do not include <common/basic.h> directly. Include <common/common.h> instead."
#endif // _FOREIGNCC_COMMON_COMMON_H_INCLUDED_

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>


//
// tribool from boost
// Use tribool as a basic type in foreigncc
//
#include <boost/logic/tribool.hpp>
using ::boost::tribool;
using ::boost::indeterminate;

static inline bool true_or_indeterminate(const tribool& tb) noexcept
{
    return tb.value == tribool::true_value || tb.value == tribool::indeterminate_value;
}

static inline bool false_or_indeterminate(const tribool& tb) noexcept
{
    return tb.value == tribool::false_value || tb.value == tribool::indeterminate_value;
}


//
// unwrap
//   Type unwrapper, expose "origin" type (const, ref removed)
//
template <typename T>
struct unwrap {
    typedef T original_type;
    typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
    typedef const type ctype;
    typedef volatile type vtype;
    typedef type& ref;
    typedef ctype& cref;
    typedef vtype& vref;
};


//
// get_array_size
//   Get length of fixed-size array
//
template <typename T, uint32_t _Size>
constexpr static inline uint32_t get_array_size(const T(&)[_Size]) noexcept
{
    return _Size;
}

template <typename T, uint32_t _Size>
constexpr static inline uint32_t get_array_size(const std::array<T, _Size>&) noexcept
{
    return _Size;
}

template <uint32_t _Size>
constexpr static inline uint32_t get_str_length(const char(&)[_Size]) noexcept
{
    return _Size - 1;
}


//
// general comparer, borrowed from C++14 less<void>
//
template <typename T, typename U>
struct general_comparer
{
#define _DECLARE_COMPARER(_Name, _Op) \
    struct _Name { \
        constexpr bool operator ()(typename unwrap<T>::cref left, typename unwrap<U>::cref right) const noexcept { \
            return (left _Op right); \
        } \
    };
    _DECLARE_COMPARER(less, < )
        _DECLARE_COMPARER(less_equal, <= )
        _DECLARE_COMPARER(greater, > )
        _DECLARE_COMPARER(greater_equal, >= )
        _DECLARE_COMPARER(equal_to, == )
        _DECLARE_COMPARER(not_equal_to, != )
        _DECLARE_COMPARER(logic_and, &&)
        _DECLARE_COMPARER(logic_or, || )
        _DECLARE_COMPARER(logic_xor, ^)
#undef _DECLARE_COMPARER
};

struct str_comparer
{
#define _DECLARE_STR_COMPARER(_Name, _Op) \
    struct _Name { \
        bool operator ()(const char* left, const char* right) const noexcept { \
            return (strcmp(left, right) _Op 0); \
        } \
        bool operator ()(const std::string& left, const char* right) const noexcept { \
            return (strcmp(left.c_str(), right) _Op 0); \
        } \
        bool operator ()(const char* left, const std::string& right) const noexcept { \
            return (strcmp(left, right.c_str()) _Op 0); \
        } \
        bool operator ()(const std::string& left, const std::string& right) const noexcept { \
            return (strcmp(left.c_str(), right.c_str()) _Op 0); \
        } \
    };
    _DECLARE_STR_COMPARER(less, < )
        _DECLARE_STR_COMPARER(less_equal, <= )
        _DECLARE_STR_COMPARER(greater, > )
        _DECLARE_STR_COMPARER(greater_equal, >= )
        _DECLARE_STR_COMPARER(equal_to, == )
        _DECLARE_STR_COMPARER(not_equal_to, != )
#undef _DECLARE_STR_COMPARER
};



//
// stringify
//
template<typename T>
static inline typename std::enable_if<
    !std::is_same<typename unwrap<T>::type, bool>::value &&
    !std::is_same<typename unwrap<T>::type, char>::value &&
    (std::is_integral<typename unwrap<T>::type>::value ||
        std::is_floating_point<typename unwrap<T>::type>::value),
    std::string>::type stringify(T&& value)
{
    return std::to_string(std::forward<T>(value));
}

template<typename T>
static inline typename std::enable_if<
    std::is_same<typename unwrap<T>::type, std::string>::value,
    std::string>::type stringify(T&& value)
{
    return std::string(std::forward<T>(value));
}

template<typename T>
static inline typename std::enable_if<
    std::is_same<typename unwrap<T>::type, char*>::value,
    std::string>::type stringify(T&& value)
{
    return std::string(std::forward<T>(value));
}

template<size_t _Size>
static inline std::string stringify(const char(&value)[_Size])
{
    return std::string(value);
}

template<typename T>
static inline typename std::enable_if<
    std::is_same<typename unwrap<T>::type, char>::value,
    std::string>::type stringify(T&& value)
{
    return std::string(1, std::forward<T>(value));
}

template<typename T>
static inline typename std::enable_if<
    std::is_same<typename unwrap<T>::type, bool>::value,
    std::string>::type stringify(T&& value)
{
    return value ? "true" : "false";
}


//
// Detect prefix length of source file path
//   For example, if this file's path is "/whatever/whatever/src/common/basic.h",
//   we want to trim the prefix "/whatever/whatever/", and leave "src/common/basic.h" in logging
//
namespace common
{
    namespace detail
    {
        constexpr char PathNormalize(char const ch)
        {
            return (ch == '\\') ? '/' : ch;
        }

        template <uint32_t _FullSize, uint32_t _SuffixSize>
        constexpr bool IsSuffixEqual(const char(&full)[_FullSize], const char(&suffix)[_SuffixSize], uint32_t const rstart = 1)
        {
            static_assert(_SuffixSize <= _FullSize, "Expect: _SuffixSize <= _FullSize");
            //static_assert(rstart <= _SuffixSize + 1, "Expect: rstart <= _SuffixSize + 1");
            //static_assert(rstart >= 1, "Expect: rstart >= 1");
            return (rstart == _SuffixSize + 1) ||
                   ((PathNormalize(full[_FullSize - rstart]) == PathNormalize(suffix[_SuffixSize - rstart])) &&
                    IsSuffixEqual(full, suffix, rstart + 1));
        }
    }

#define _THIS_FILE_RELATIVE_PATH    ("src/common/basic.h")
    static_assert(detail::IsSuffixEqual(__FILE__, _THIS_FILE_RELATIVE_PATH), "Expect: IsSuffixEqual");
    constexpr static uint32_t SourceFilePathPrefixLength = get_array_size(__FILE__) - get_array_size(_THIS_FILE_RELATIVE_PATH);
#undef _THIS_FILE_RELATIVE_PATH

}  // namespace common


#endif // _FOREIGNCC_COMMON_BASIC_H_INCLUDED_
