///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ACTOR_FWD_HPP
#define GCE_ACTOR_ACTOR_FWD_HPP

#include <gce/actor/config.hpp>

namespace gce
{
inline std::size_t default_stacksize()
{
  return boost::coroutines::stack_traits::default_size();
}

inline std::size_t minimum_stacksize()
{
  return boost::coroutines::stack_traits::minimum_size();
}

namespace detail
{
enum actor_type
{
  actor_nil = -1, 
  
  actor_threaded,
  actor_stackful,
  actor_stackless,
  actor_nonblocked,
  actor_socket,
  actor_acceptor,
#ifdef GCE_LUA
  actor_luaed,
#endif

  actor_num
};
}
}

#endif /// GCE_ACTOR_ACTOR_FWD_HPP
