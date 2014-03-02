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
#include <gce/actor/context.hpp>
#include <gce/detail/scope.hpp>

namespace gce
{
namespace detail
{
///----------------------------------------------------------------------------
conn::conn(context* ctx)
  : base_type(ctx->get_attributes().max_cache_match_size_)
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
void conn::start(aid_t acpr, aid_t master, aid_t skt)
{
  acpr_ = acpr;
  master_ = master;
  skt_ = skt;

  base_type::link(link_t(monitored, master_), owner_));
  stat_ = on;
}
///----------------------------------------------------------------------------
void conn::on_free()
{
  base_type::on_free();

  stat_ = ready;
  acpr_ = aid_t();
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
void conn::send(aid_t recver, message const& m)
{
  detail::pack* pk = base_type::alloc_pack(user_);
  pk->tag_ = get_aid();
  pk->recver_ = recver;
  pk->msg_ = m;

  recver.get_actor_ptr()->on_recv(pk);
}
///----------------------------------------------------------------------------
void conn::handle_recv(pack* pk)
{
  scope scp(boost::bind(&base_type::dealloc_pack, user_, pk));
  if (check(pk->recver_))
  {
    if (aid_t* aid = boost::get<aid_t>(&pk->tag_))
    {
      //send(pk->msg_);
      aid_t sender = *aid;
      match_t type = pk->msg_.get_type();
      if (sender == acpr_)
      {
        if (type == atom("gce_set_skt"))
        {
          pk->msg_ >> skt_;
        }
      }
      else if (sender == master_)
      {
        send(skt_, pk->msg_);
      }
      else if (sender == skt_)
      {
        send(master_, pk->msg_);
      }
    }
    else if (link_t* link = boost::get<link_t>(&pk->tag_))
    {
      add_link(link->get_aid());
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk->tag_))
    {
      base_type::remove_link(ex->get_aid());
      if (ex->get_aid() == master_)
      {
        message m(exit);
        std::string exit_msg("remote exited");
        m << exit_remote << exit_msg;
        send(m);
        free_self(exit_normal, "exit normal");
      }
    }
  }
  else if (!pk->is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
    {
      /// send actor exit msg
      base_type::send_already_exited(link->get_aid(), pk->recver_, user_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
    {
      /// reply actor exit msg
      response_t res(req->get_id(), pk->recver_);
      base_type::send_already_exited(req->get_aid(), res, user_);
    }
  }
}
///----------------------------------------------------------------------------
void conn::free_self(exit_code_t exc, std::string const& exit_msg)
{
  base_type::send_exit(exc, exit_msg, user_);
  base_type::update_aid();
  user_->free_conn(owner_, this);
}
///----------------------------------------------------------------------------
}
}
