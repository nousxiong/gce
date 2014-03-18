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
#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/mpsc_queue.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
struct pack
  : public object_pool<pack>::object
  , public mpsc_queue<pack>::node
{
  pack()
    : is_err_ret_(false)
    , owner_(0)
  {
  }

  inline void on_free()
  {
    tag_ = detail::tag_t();
    recver_ = aid_t();
    skt_ = aid_t();
    svc_ = svcid_t();
    is_err_ret_ = false;
    msg_ = message();
  }

  detail::tag_t tag_;
  aid_t recver_;
  aid_t skt_;
  svcid_t svc_;
  bool is_err_ret_;
  message msg_;

  cache_pool* owner_;
};

typedef object_pool<pack> pack_pool_t;
typedef mpsc_queue<pack> pack_queue_t;
}
}

#endif /// GCE_ACTOR_DETAIL_PACK_HPP
