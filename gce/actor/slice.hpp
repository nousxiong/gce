///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_SLICE_HPP
#define GCE_ACTOR_SLICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/ref_count.hpp>

namespace gce
{
class mixin;
class slice
  : public detail::object_pool<slice, mixin*>::object
  , public basic_actor
  , public detail::ref_count_st
{
  typedef basic_actor base_type;

public:
  explicit slice(mixin*);
  ~slice();

public:
  aid_t recv(message&, match_list_t const& match_list = match_list_t());
  aid_t recv(response_t, message&);

public:
  /// internal use
  void init(aid_t);
  void on_free();
  void on_recv(detail::pack*);
  void free();

private:
  mixin* sire_;
};

typedef boost::intrusive_ptr<slice> slice_t;
}

#endif /// GCE_ACTOR_SLICE_HPP
