///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ALL_HPP
#define GCE_ACTOR_ALL_HPP

#include <gce/actor/context.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/addon.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/remote.hpp>
#include <gce/actor/spawn.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/guard.hpp>
#include <gce/actor/service.hpp>
#ifdef GCE_SCRIPT
# include <gce/actor/script.hpp>
#endif
#ifdef GCE_LUA
# include <gce/actor/cpp2lua.hpp>
# include <gce/actor/lua_actor_proxy.hpp>
#endif
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/adaptor.hpp>
#include <gce/actor/asio.hpp>
#include <gce/actor/exception.hpp>

#endif /// GCE_ACTOR_ALL_HPP
