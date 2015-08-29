///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER4_STRING_HASH_HPP
#define CLUSTER4_STRING_HASH_HPP

#include <gce/actor/all.hpp>
#include <cstring>

/// 简单的字符串哈希函数，来自《The C Programming Language》
inline uint64_t string_hash(char const* str)
{
  size_t size = std::strlen(str);
  uint64_t ret = 0;
  for (size_t i=0; i<size; ++i)
  {
    uint64_t ch = (uint64_t)*str++;
    ret = ret * 131 + ch;
  }
  return ret;
}

#endif /// CLUSTER4_STRING_HASH_HPP
