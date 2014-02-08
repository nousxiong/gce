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
#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/mpsc_queue.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/detail/exit.hpp>
#include <gce/actor/actor_id.hpp>
#include <boost/variant/variant.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
struct pack
  : public object_pool<pack>::object
  , public mpsc_queue<pack>::node
{
  typedef boost::variant<aid_t, request_t, response_t, link_t, exit_t> tag_t;

  inline void on_free()
  {
    tag_ = tag_t();
    recver_ = aid_t();
    msg_ = message();
  }

  tag_t tag_;
  aid_t recver_;
  message msg_;

  cache_pool* owner_;
};

typedef object_pool<pack> pack_pool_t;
typedef mpsc_queue<pack> pack_queue_t;
}
}

#endif /// GCE_ACTOR_DETAIL_PACK_HPP
