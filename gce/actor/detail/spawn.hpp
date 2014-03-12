///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SPAWN_HPP
#define GCE_ACTOR_DETAIL_SPAWN_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>

namespace gce
{
namespace detail
{
class spawn_t
{
public:
  spawn_t()
    : func_(match_nil)
    , ctxid_(ctxid_nil)
    , stack_size_(0)
  {
  }

  spawn_t(
    match_t func, match_t ctxid,
    std::size_t stack_size, sid_t sid, aid_t aid
    )
    : func_(func)
    , ctxid_(ctxid)
    , stack_size_(stack_size)
    , sid_(sid)
    , aid_(aid)
  {
  }

  ~spawn_t()
  {
  }

public:
  inline match_t get_func() const { return func_; }
  inline match_t get_ctxid() const { return ctxid_; }
  inline std::size_t get_stack_size() const { return stack_size_; }
  inline sid_t get_id() const { return sid_; }
  inline aid_t get_aid() const { return aid_; }

private:
  match_t func_;
  match_t ctxid_;
  std::size_t stack_size_;
  sid_t sid_;
  aid_t aid_;
};

enum spawn_error
{
  spawn_ok = 0,
  spawn_no_socket,
  spawn_func_not_found,
};

class spawn_ret_t
{
public:
  spawn_ret_t()
    : err_(spawn_ok)
    , sid_(sid_nil)
  {
  }

  spawn_ret_t(spawn_error err, sid_t sid, aid_t aid)
    : err_(err)
    , sid_(sid)
    , aid_(aid)
  {
  }

  ~spawn_ret_t()
  {
  }

public:
  inline spawn_error get_error() const { return err_; }
  inline sid_t get_id() const { return sid_; }
  inline aid_t get_aid() const { return aid_; }

private:
  spawn_error err_;
  sid_t sid_;
  aid_t aid_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SPAWN_HPP

