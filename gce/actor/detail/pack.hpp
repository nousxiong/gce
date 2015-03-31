///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_PACK_HPP
#define GCE_ACTOR_DETAIL_PACK_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/service_id.hpp>
#include <gce/actor/message.hpp>

namespace gce
{
namespace detail
{
struct pack
{
  pack()
    : is_err_ret_(false)
    , concurrency_index_(size_nil)
    , cache_index_(u64_nil)
    , type_(actor_nil)
    , sid_(sid_nil)
    , expiry_(false)
  {
  }

  ~pack()
  {
  }

  tag_t tag_;
  aid_t recver_;
  aid_t skt_;
  svcid_t svc_;
  bool is_err_ret_;
  message msg_;

  /// for nonblocked actor recv
  size_t concurrency_index_;
  uint64_t cache_index_;
  actor_type type_;

  /// for in pool actor recv
  actor_index ai_;
  sid_t sid_;

  /// expiry flag
  bool expiry_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_PACK_HPP
