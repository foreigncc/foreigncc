#ifndef _FOREIGNCC_COMMON_MACROS_H_INCLUDED_
#define _FOREIGNCC_COMMON_MACROS_H_INCLUDED_

#ifndef _FOREIGNCC_COMMON_COMMON_H_INCLUDED_
#error "Do not include <common/macros.h> directly. Include <common/common.h> instead."
#endif // _FOREIGNCC_COMMON_COMMON_H_INCLUDED_


// This is to get rid of comma problem in macro substitution
#define JUST(...)  __VA_ARGS__


#if BOOST_COMP_MSVC
#define CPP_NO_INLINE   __declspec(noinline)
#elif BOOST_COMP_GNUC
#define CPP_NO_INLINE   __attribute__((noinline))
#else
#define CPP_NO_INLINE
#endif



//#if BOOST_COMP_MSVC
//#define CPP_NO_INLINE   __declspec(noreturn)
//#elif BOOST_COMP_GNUC
//#define CPP_NO_INLINE   __attribute__((noreturn))
//#else
//#define CPP_NO_INLINE
//#endif
//
// [[noreturn]] is C++11 standard
#define CPP_NO_RETURN   [[noreturn]]


#endif  // !_FOREIGNCC_COMMON_MACROS_H_INCLUDED_
