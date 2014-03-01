///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/conn.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
///----------------------------------------------------------------------------
conn::conn(context* ctx)
  : basic_actor(ctx->get_attributes().max_cache_match_size_)
  , stat_(ready)
{
}
///----------------------------------------------------------------------------
conn::~conn()
{
}
///----------------------------------------------------------------------------
void conn::init(cache_pool* user, cache_pool* owner)
{
  BOOST_ASSERT_MSG(stat_ == ready, "conn status error");
  user_ = user;
  owner_ = owner;

  base_type::update_aid();
}
///----------------------------------------------------------------------------
void conn::start(aid_t skt, aid_t master)
{
  master_ = master;
  skt_ = skt;
}
///----------------------------------------------------------------------------
void conn::on_free()
{
  base_type::on_free();

  stat_ = ready;
  master_ = aid_t();
  skt_ = aid_t();
}
///----------------------------------------------------------------------------
void conn::on_recv(pack* pk)
{
  user_->get_strand().dispatch(
    boost::bind(
      &conn::handle_recv, this, pk
      )
    );
}
///----------------------------------------------------------------------------
}
}
