///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_NETWORK_SERVICE_HPP
#define GCE_ACTOR_DETAIL_NETWORK_SERVICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/inpool_service.hpp>
#include <gce/actor/detail/actor_pool.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <set>

namespace gce
{
namespace detail
{
template <typename Actor>
class network_service
  : public inpool_service<Actor>
{
  typedef Actor actor_t;
  typedef inpool_service<actor_t> base_t;
  typedef network_service<actor_t> self_t;
  typedef actor_pool<actor_t> actor_pool_t;

public:
  typedef typename actor_t::type type;
  typedef typename actor_t::context_t context_t;

public:
  network_service(context_t& ctx, strand_t& snd, size_t index)
    : base_t(ctx, snd, index)
    , actor_pool_(
        base_t::ctxid_, base_t::timestamp_, (uint16_t)base_t::index_,
        actor_t::get_pool_reserve_size(base_t::ctx_.get_attributes())
        )
  {
  }

  ~network_service()
  {
  }

public:
  actor_t* make_actor()
  {
    return actor_pool_.make(boost::ref(*this));
  }

  void free_actor(actor_t* a)
  {
    actor_pool_.free(a);
  }

  void add_actor(actor_t* a)
  {
    actor_list_.insert(a);
  }

  void remove_actor(actor_t* a)
  {
    actor_list_.erase(a);
  }

  void stop()
  {
    base_t::stop();
    BOOST_FOREACH(actor_t* a, actor_list_)
    {
      a->stop();
    }
    actor_list_.clear();
  }

protected:
  actor_t* find_actor(actor_index ai, sid_t sid)
  {
    return actor_pool_.find(ai, sid);
  }

private:
  actor_pool_t actor_pool_;
  std::set<actor_t*> actor_list_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_NETWORK_SERVICE_HPP
