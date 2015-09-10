///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER5_UTIL_HPP
#define CLUSTER5_UTIL_HPP

#include "lua_wrap.hpp"
#include <gce/actor/all.hpp>
#include <sstream>

namespace cluster5
{
static void make_libutil(lua_State* L)
{
  /// register libutil
  gce::lualib::open(L)
    .begin("libutil")
      .add_function("hash", detail::lua::hash)
    .end()
    ;
}
}

#endif /// CLUSTER5_UTIL_HPP
