///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_SCRIPT_HPP
#define GCE_ACTOR_SCRIPT_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>

namespace gce
{
template <typename Type, typename ActorRef>
inline void register_script(
  ActorRef self, 
  std::string const& name, 
  std::string const& script = std::string()
  )
{
  typedef typename ActorRef::context_t context_t;
  context_t& ctx = self.get_context();
  ctx.register_script<Type>(name, script);
}
} /// namespace gce

#endif /// GCE_ACTOR_SCRIPT_HPP
