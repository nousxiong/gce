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
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/integer.hpp>

namespace gce
{
typedef boost::mpl::if_<
  boost::is_same<boost::mpl::int_<sizeof(void*)>, boost::mpl::int_<4> >,
    boost::uint32_t, boost::uint64_t
  >::type uintptr_t;

typedef unsigned char byte_t;

static byte_t const byte_nil = static_cast<byte_t>(-1);
static boost::int8_t const i8_nil = static_cast<boost::int8_t>(-1);
static boost::int16_t const i16_nil = static_cast<boost::int16_t>(-1);
static boost::int32_t const i32_nil = static_cast<boost::int32_t>(-1);
static boost::int64_t const i64_nil = static_cast<boost::int64_t>(-1);
static boost::uint8_t const u8_nil = static_cast<boost::uint8_t>(-1);
static boost::uint16_t const u16_nil = static_cast<boost::uint16_t>(-1);
static boost::uint32_t const u32_nil = static_cast<boost::uint32_t>(-1);
static boost::uint64_t const u64_nil = static_cast<boost::uint64_t>(-1);
static std::size_t const size_nil = static_cast<std::size_t>(-1);
}

#endif /// GCE_INTEGER_HPP
