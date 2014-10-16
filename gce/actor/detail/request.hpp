///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_REQUEST_HPP
#define GCE_ACTOR_DETAIL_REQUEST_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>

namespace gce
{
namespace detail
{
class request_t
{
public:
  request_t() : id_(sid_nil) {}
  request_t(sid_t id, aid_t aid) : id_(id), aid_(aid) {}
  ~request_t() {}

public:
  bool valid() const { return id_ != sid_nil; }
  sid_t get_id() const { return id_; }
  aid_t get_aid() const { return aid_; }

private:
  sid_t id_;
  aid_t aid_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_REQUEST_HPP

