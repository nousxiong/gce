///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_LUA_OBJECT_HPP
#define GCE_MYSQL_LUA_OBJECT_HPP

#include <gce/mysql/config.hpp>
#include <gce/lualib/all.hpp>
#include <cstring>

namespace gce
{
namespace mysql
{
namespace lua
{
enum
{
  /// both in adata and amsg means userdata
  ty_result = gce::lua::ty_num + 100,

  ty_num
};

} /// namespace lua
} /// namespace mysql
} /// namespace gce

#endif /// GCE_MYSQL_LUA_OBJECT_HPP
