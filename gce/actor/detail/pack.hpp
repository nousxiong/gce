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
#include <boost/function.hpp>

namespace gce
{
class thread;
namespace detail
{
struct pack
  : public object_pool<pack>::object
  , public mpsc_queue<pack>::node
{
  pack()
    : is_err_ret_(false)
    , thr_(0)
    , que_(0)
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

    target_ = aid_t();
    thr_ = 0;
    que_ = 0;
    f_.clear();
  }

  detail::tag_t tag_;
  aid_t recver_;
  aid_t skt_;
  svcid_t svc_;
  bool is_err_ret_;
  message msg_;

  aid_t target_;
  thread* thr_;
  mpsc_queue<pack>* que_;
  boost::function<void ()> f_;
};

typedef object_pool<pack> pack_pool_t;
typedef mpsc_queue<pack> pack_queue_t;
}
}

#endif /// GCE_ACTOR_DETAIL_PACK_HPP
