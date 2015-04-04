///
/// integer.hpp
///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_INTEGER_HPP
#define GCE_INTEGER_HPP

#include <gce/config.hpp>
#ifdef _MSC_VER
# if _MSC_VER <= 1500
#   include <gce/adata/cpp/stdint.hpp>
# else
#   include <cstdint>
# endif
#elif __cplusplus < 201103L
# include <stdint.h>
#else
# include <cstdint>
#endif
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/array.hpp>

namespace gce
{
typedef boost::mpl::if_<
  boost::is_same<boost::mpl::int_<sizeof(void*)>, boost::mpl::int_<4> >,
    int32_t, int64_t
  >::type intptr_t;

typedef boost::mpl::if_<
  boost::is_same<boost::mpl::int_<sizeof(void*)>, boost::mpl::int_<4> >,
    uint32_t, uint64_t
  >::type uintptr_t;

typedef boost::array<char, 32> intbuf_t;

typedef unsigned char byte_t;
static byte_t const byte_nil = static_cast<byte_t>(-1);
static int8_t const i8_nil = static_cast<int8_t>(-1);
static int16_t const i16_nil = static_cast<int16_t>(-1);
static int32_t const i32_nil = static_cast<int32_t>(-1);
static int64_t const i64_nil = static_cast<int64_t>(-1);
static uint8_t const u8_nil = static_cast<uint8_t>(-1);
static uint16_t const u16_nil = static_cast<uint16_t>(-1);
static uint32_t const u32_nil = static_cast<uint32_t>(-1);
static uint64_t const u64_nil = static_cast<uint64_t>(-1);
static size_t const size_nil = static_cast<size_t>(-1);
}

#endif /// GCE_INTEGER_HPP
