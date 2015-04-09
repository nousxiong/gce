///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_LUA_ACTOR_PROXY_HPP
#define GCE_ACTOR_LUA_ACTOR_PROXY_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/lua_actor.hpp>

namespace gce
{
namespace lua
{
typedef detail::lua_actor<context>::proxy actor_proxy;
}
}

#endif /// GCE_ACTOR_LUA_ACTOR_PROXY_HPP
