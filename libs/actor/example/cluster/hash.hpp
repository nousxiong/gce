///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_HASH_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_HASH_HPP

#include <gce/actor/all.hpp>

typedef gce::match_t hash_t;

/// from《The C Programming Language》
hash_t hash(char const* str, std::size_t size)
{
  hash_t ret = 0;
  for (std::size_t i=0; i<size; ++i)
  {
    hash_t ch = (hash_t)*str++;
    ret = ret * 131 + ch;
  }
  return ret;
}

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_HASH_HPP
