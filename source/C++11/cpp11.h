// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _CPP11_
#define _CPP11_
// The reason this header exists is due to the asymmetric availiability of C++11 on our
// various compiler targets.  In particular, none of the typical headers for C++11 support
// exist when building with libstdc++ on Apple, so we have to disable it across the board.
#ifdef __APPLE__
#include <ciso646>  // detect std::lib
#ifdef _LIBCPP_VERSION // clang + libc++
  #define STL11_ALLOWED 1
#else // clang + libstdc++
  #define STL11_ALLOWED 0
#endif
#else // gcc or MSVC
#define STL11_ALLOWED 1
#endif

#define CLANG_CHECK(maj, min) (__clang_major__ == maj && __clang_minor__ >= min || __clang_major__ > maj)
#define GCC_CHECK(maj, min) (__GNUC__ == maj && __GNUC_MINOR__  >= min || __GNUC__ > maj)

// If Boost.Thread is used, we want it to provide the new name for its "future" class
#define BOOST_THREAD_PROVIDES_FUTURE

/*********************
 * __func__ function name support
 *********************/
#ifdef _MSC_VER
  #define __func__ __FUNCTION__
#endif

/*********************
 * Location of the unordered_set header
 *********************/
#if defined(__APPLE__) && !defined(_LIBCPP_VERSION)
  #define STL_UNORDERED_SET <tr1/unordered_set>
  #define STL_UNORDERED_MAP <tr1/unordered_map>
#else
  #define STL_UNORDERED_SET <unordered_set>
  #define STL_UNORDERED_MAP <unordered_map>
#endif

/*********************
 * Check nullptr availability
 *********************/
#if _MSC_VER >= 1500
  #define NULLPTR_AVAILABLE 1
#elif CLANG_CHECK(3, 0) && (__cplusplus >= 201103 || defined(_LIBCPP_VERSION))
  // Nullptr added to clang in version 3.0
  #define NULLPTR_AVAILABLE 1
#else
  #define NULLPTR_AVAILABLE 0
#endif

#if !NULLPTR_AVAILABLE
  #define nullptr 0
#endif

/*********************
 * Check override keyword availability
 *********************/
#if __cplusplus < 201103 && !defined(_MSC_VER)
  #define override
#endif

/*********************
 * static_assert availability
 *********************/
#if CLANG_CHECK(2, 9)
  #define HAS_STATIC_ASSERT 1
#elif _MSC_VER >= 1500
  #define HAS_STATIC_ASSERT 1
#else
  #define HAS_STATIC_ASSERT 0
#endif

#if !HAS_STATIC_ASSERT || !STL11_ALLOWED
  // Static assert completely disabled if it's not available
  // TODO: preferably use LEAP_STATIC_ASSERT
  #define static_assert(...)
#endif

/*********************
 * exception_ptr availability
 *********************/
#if defined(__APPLE__) && !defined(_LIBCPP_VERSION)
  #define EXCEPTION_PTR_HEADER "C++11/boost_exception_ptr.h"
#else
  #define EXCEPTION_PTR_HEADER <stdexcept>
  #define throw_rethrowable throw
#endif

/*********************
 * future availability
 *********************/
#if _MSC_VER >= 1700
  #define FUTURE_HEADER <future>
#else
  #define FUTURE_HEADER "C++11/boost_future.h"
#endif

/**
 * type_index support
 */
// Supported natively on any platform except mac
#if LEAP_OS_MAC && !defined(_LIBCPP_VERSION)
  #define TYPE_INDEX_HEADER "C++11/custom_type_index.h"
#else
  #define TYPE_INDEX_HEADER <typeindex>
#endif

/*********************
 * Decide what version of type_traits we are going to use
 *********************/
#if _MSC_VER >= 1500
  #define TYPE_TRAITS_AVAILABLE 1
#elif __GXX_EXPERIMENTAL_CXX0X__ && STL11_ALLOWED
  #define TYPE_TRAITS_AVAILABLE 1
#else
  #define TYPE_TRAITS_AVAILABLE 0
#endif

#if TYPE_TRAITS_AVAILABLE
  #if defined(_MSC_VER) || defined(_LIBCPP_VERSION)
    #define TYPE_TRAITS_HEADER <type_traits>
  #else
    #define TYPE_TRAITS_HEADER <tr1/type_traits>
  #endif
#else
  #define TYPE_TRAITS_HEADER "C++11/boost_type_traits.h"
#endif

/*********************
 * Decide what version of shared_ptr we are going to use
 *********************/

// Nullptr available in VS10
#if _MSC_VER >= 1500
  #define SHARED_PTR_IN_STL 1
#elif !STL11_ALLOWED
  #define SHARED_PTR_IN_STL 0
#elif __cplusplus > 199711L || __GXX_EXPERIMENTAL_CXX0X__
  #define SHARED_PTR_IN_STL 1
#else
  #define SHARED_PTR_IN_STL 0
#endif

#if SHARED_PTR_IN_STL
  #define SHARED_PTR_HEADER <memory>
#else
  #define SHARED_PTR_HEADER "C++11/boost_shared_ptr.h"
#endif

/*********************
 * noexcept support
 *********************/
#ifdef _MSC_VER
  #define NOEXCEPT(x) __declspec(nothrow) x
#else
  #define NOEXCEPT(x) x noexcept
#endif

/*********************
 * Lambdas?
 *********************/
#if _MSC_VER >= 1500
  #define LAMBDAS_AVAILABLE 1
#elif CLANG_CHECK(3, 2)
  // Only available if we're told we're using at least C++11
  #if __cplusplus >= 201103L
    #define LAMBDAS_AVAILABLE 1
  #else
    #define LAMBDAS_AVAILABLE 0
  #endif
#elif __GXX_EXPERIMENTAL_CXX0X__
  #define LAMBDAS_AVAILABLE 1
#endif

#if LAMBDAS_AVAILABLE && STL11_ALLOWED
  #define FUNCTIONAL_HEADER <functional>
#else
  #define FUNCTIONAL_HEADER "C++11/boost_functional.h"
#endif

#ifndef LAMBDAS_AVAILABLE
  #define LAMBDAS_AVAILABLE 0
#endif

/**
 * R-value reference check
 */
#if STL11_ALLOWED
  #define RVALUE_HEADER <memory>
#else
  // Remove literal references in order to fix another missing header problem on *nix
  // Defined by default on C++11 Clang
  #if !defined(__APPLE__) || __cplusplus >= 201103L
    #define BOOST_NO_UNICODE_LITERALS 1
  #endif
  #define RVALUE_HEADER "C++11/boost_rvalue.h"
#endif

/*********************
 * Make all of TR1 available in namespace std, but only on apple:
 *********************/
#ifdef __APPLE__
namespace std {
  namespace tr1 {}
  using namespace std::tr1;
}
#endif

/*********************
 * Deprecation convenience macro
 *********************/
#ifdef _MSC_VER
#define DEPRECATED(signature, msg) __declspec(deprecated(msg)) signature
#else
#define DEPRECATED(signature, msg) signature __attribute__((deprecated))
#endif

#endif
