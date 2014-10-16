///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ATOM_HPP
#define GCE_ACTOR_ATOM_HPP

#include <gce/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/integer.hpp>
#include <string>
#include <cstring>

namespace gce
{
/// Since lordoffox's str2val.h (http://bbs.cppfans.org/forum.php?mod=viewthread&tid=56&extra=page%3D1)
boost::uint64_t atom(char const* str)
{
  std::size_t len = std::strlen(str);
  BOOST_ASSERT(len <= 13);

  static char const* const encoding_table =
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9\xa\xb\xc\xd\xe\xf"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x0\x0\x0\x0\x1b"
    "\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9\xa\xb\xc\xd\xe\xf"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
    "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0";

  boost::uint64_t value = 0;
  boost::uint32_t encode_value = 0;
  for (std::size_t i=0 ; i<len; ++i)
  {
    value *= 28;
    encode_value = encoding_table[(int)str[i]];
    if (encode_value)
    {
      value += encode_value;
    }
    else
    {
      return 0;
    }
  }
  return value;
}

/// Since lordoffox's str2val.h (http://bbs.cppfans.org/forum.php?mod=viewthread&tid=56&extra=page%3D1)
std::string atom(boost::uint64_t what)
{
  std::string ret;
  static std::string::const_pointer const decoding_table = "\0abcdefghijklmnopqrstuvwxyz_";
  boost::uint64_t x = what;
  std::string::value_type buf[21] = {0};
  std::size_t pos = 19;
  while (x)
  {
    buf[pos--] = decoding_table[x % 28];
    x = x / 28;
  }
  ++pos;
  ret.assign(buf + pos, 20 - pos);
  return ret;
}

namespace detail
{
#ifdef GCE_LUA
enum overloading_type
{
  overloading_0 = 0,
  overloading_1,
  overloading_2,

  overloading_aid = overloading_0,
  overloading_svcid = overloading_1,

  overloading_pattern = overloading_0,
  overloading_match_t = overloading_1,
  overloading_duration = overloading_2,

  overloading_msg = overloading_0,
};

int lua_overloading_0()
{
  return (int)overloading_0;
}
int lua_overloading_1()
{
  return (int)overloading_1;
}
int lua_overloading_2()
{
  return (int)overloading_2;
}

#endif
} /// namespace detail
} /// namespace gce

#ifdef GCE_LUA
# define GCE_LUA_SERIALIZE_FUNC \
  template <typename Strm> \
  Strm serialize(Strm& s) \
  { \
    s << *this; \
    return s; \
  } \
  template <typename Strm> \
  Strm deserialize(Strm& s) \
  { \
    s >> *this; \
    return s; \
  } \
  template <typename Self> \
  Self make() \
  { \
    return Self(); \
  }

# define GCE_LUA_REG_SERIALIZE_FUNC(class_name) \
  .addFunction("serialize", &class_name::serialize<message>) \
  .addFunction("deserialize", &class_name::deserialize<message>) \
  .addFunction("make", &class_name::make<class_name>)

#endif /// GCE_LUA

namespace gce
{
struct match_type
{
  match_type()
    : val_(0)
  {
  }

  explicit match_type(int val)
    : val_((boost::uint64_t)val)
  {
  }

  match_type(boost::uint64_t val)
    : val_(val)
  {
  }

  operator boost::uint64_t() const
  {
    return val_;
  }

  void operator=(boost::uint64_t val)
  {
    val_ = val;
  }

  void operator=(boost::uint32_t val)
  {
    val_ = val;
  }

  bool equals(match_type const& rhs) const
  {
    return val_ == rhs.val_;
  }

#ifdef GCE_LUA
  int get_overloading_type() const
  {
    return (int)detail::overloading_match_t;
  }

  std::string to_string()
  {
    std::string rt;
    rt += "<";
    rt += boost::lexical_cast<std::string>(val_);
    rt += ">";
    return rt;
  }

  GCE_LUA_SERIALIZE_FUNC
#endif

  boost::uint64_t val_;
};

#ifdef GCE_LUA
match_type make_match(int i)
{
  return match_type(i);
}

match_type s2i(char const* str)
{
  return atom(str);
}

std::string i2s(match_type what)
{
  return atom(what);
}
#endif
}

#endif /// GCE_ACTOR_ATOM_HPP
