///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///


#include <gce/actor/lua_actor.hpp>

#ifdef GCE_LUA
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
lua_actor::lua_actor(aid_t aid, detail::cache_pool* user)
  : base_type(&user->get_context(), user, aid, user->get_index())
  , L_(user->get_lua_state())
  , f_(L_)
  , co_(L_)
  , yielding_(false)
  , recving_(false)
  , responsing_(false)
  , tmr_(ctx_->get_io_service())
  , tmr_sid_(0)
{
}
///----------------------------------------------------------------------------
lua_actor::~lua_actor()
{
}
///----------------------------------------------------------------------------
void lua_actor::recv()
{
  recv_match(pattern());
}
///----------------------------------------------------------------------------
void lua_actor::recv_match(pattern const& patt)
{
  aid_t sender;
  detail::recv_t rcv;
  message msg;

  if (!mb_.pop(rcv, msg, patt.match_list_))
  {
    duration_t tmo = patt.timeout_;
    if (tmo > zero)
    {
      if (tmo < infin)
      {
        start_timer(tmo);
      }
      recving_ = true;
      curr_pattern_ = patt;
      yield();
      return;
    }
  }
  else
  {
    sender = end_recv(rcv, msg);
  }
  
  set_recv_global(sender, msg);
}
///----------------------------------------------------------------------------
void lua_actor::recv_response(resp_t res)
{
  recv_response_timeout(res, seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC));
}
///----------------------------------------------------------------------------
void lua_actor::recv_response_timeout(resp_t res, duration_type tmo)
{
  aid_t sender;
  message msg;
  duration_t dur(tmo.dur_);

  if (!mb_.pop(res, msg))
  {
    if (dur > zero)
    {
      if (dur < infin)
      {
        start_timer(dur);
      }
      responsing_ = true;
      recving_res_ = res;
      yield();
      return;
    }
  }
  else
  {
    sender = end_recv(res);
  }

  set_recv_global(sender, msg);
}
///----------------------------------------------------------------------------
void lua_actor::wait(duration_type dur)
{
  start_timer(dur.dur_);
  yield();
}
///----------------------------------------------------------------------------
void lua_actor::spawn(std::string const& script, bool sync_sire, int type)
{
  detail::cache_pool* user = 0;
  if (sync_sire)
  {
    user = user_;
  }
  else
  {
    user = ctx_->select_cache_pool();
  }

  user->get_strand().post(
    boost::bind(
      &detail::cache_pool::spawn_lua_actor, user, 
      script, get_aid(), (link_type)type
      )
    );
  recv_match(pattern(detail::msg_new_actor));
}
///----------------------------------------------------------------------------
void lua_actor::spawn_remote(
  detail::spawn_type sty, std::string const& func, match_type ctxid, 
  int type, std::size_t stack_size, seconds_t tmo
  )
{
  aid_t aid;
  sid_t sid = base_type::new_request();
  base_type::pri_spawn(sid, sty, func, ctxid, stack_size);

  duration_t curr_tmo = tmo;
  typedef boost::chrono::system_clock clock_t;
  clock_t::time_point begin_tp = clock_t::now();
  spw_hdr_ = 
    boost::bind(
      &lua_actor::handle_remote_spawn, this, _1, _2,
      (link_type)type, begin_tp, sid, tmo, curr_tmo
      );
  recv_match(pattern(detail::msg_spawn_ret, curr_tmo));
}
///----------------------------------------------------------------------------
void lua_actor::set_coro(luabridge::LuaRef co)
{
  co_ = co;
}
///----------------------------------------------------------------------------
void lua_actor::init(std::string const& script)
{
  script_ = script;
}
///----------------------------------------------------------------------------
void lua_actor::start()
{
  try
  {
    luabridge::setGlobal(L_, this, "self");
    luabridge::LuaRef nil(L_);
    luabridge::setGlobal(L_, nil, "gce_curr_co");

    f_ = user_->get_script(script_);
    if (f_.isNil())
    {
      std::string errmsg("script:");
      errmsg += script_;
      errmsg += " compile error or not found";
      throw std::runtime_error(errmsg);
    }
    f_();
    quit();
  }
  catch (std::exception& ex)
  {
    quit(exit_except, ex.what());
  }
}
///----------------------------------------------------------------------------
void lua_actor::on_recv(detail::pack& pk, detail::send_hint)
{
  handle_recv(pk);
}
///----------------------------------------------------------------------------
void lua_actor::quit(exit_code_t exc, std::string const& errmsg)
{
  if (!yielding_)
  {
    aid_t self_aid = get_aid();
    snd_.post(boost::bind(&lua_actor::stop, this, self_aid, exc, errmsg));
  }
}
///----------------------------------------------------------------------------
void lua_actor::yield()
{
  if (!yielding_)
  {
    yielding_ = true;
    lua_yield(co_.state(), 0);
  }
}
///----------------------------------------------------------------------------
void lua_actor::resume()
{
  BOOST_ASSERT(yielding_);
  yielding_ = false;
  luabridge::setGlobal(L_, this, "self");
  luabridge::setGlobal(L_, co_, "gce_curr_co");
}
///----------------------------------------------------------------------------
void lua_actor::stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
{
  base_type::send_exit(self_aid, ec, exit_msg);
  user_->free_actor(this);
}
///----------------------------------------------------------------------------
void lua_actor::start_timer(duration_t dur)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &lua_actor::handle_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_
        )
      )
    );
}
///----------------------------------------------------------------------------
void lua_actor::handle_timeout(errcode_t const& ec, std::size_t tmr_sid)
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    recving_ = false;
    responsing_ = false;
    curr_pattern_.clear();
    try
    {
      if (spw_hdr_)
      {
        spawn_handler_t hdr(spw_hdr_);
        spw_hdr_.clear();
        hdr(aid_t(), message());
      }
      resume();
      set_recv_global(aid_t(), message());
      f_();
      quit();
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
aid_t lua_actor::end_recv(detail::recv_t& rcv, message& msg)
{
  aid_t sender;
  if (aid_t* aid = boost::get<aid_t>(&rcv))
  {
    sender = *aid;
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&rcv))
  {
    sender = req->get_aid();
    msg.req_ = *req;
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv))
  {
    sender = ex->get_aid();
  }
  return sender;
}
///----------------------------------------------------------------------------
aid_t lua_actor::end_recv(resp_t& res)
{
  return res.get_aid();
}
///----------------------------------------------------------------------------
void lua_actor::handle_recv(detail::pack& pk)
{
  bool is_response = false;

  if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
  {
    mb_.push(*aid, pk.msg_);
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&pk.tag_))
  {
    mb_.push(*req, pk.msg_);
  }
  else if (detail::link_t* link = boost::get<detail::link_t>(&pk.tag_))
  {
    add_link(link->get_aid(), pk.skt_);
    return;
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&pk.tag_))
  {
    mb_.push(*ex, pk.msg_);
    base_type::remove_link(ex->get_aid());
  }
  else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
  {
    is_response = true;
    mb_.push(*res, pk.msg_);
  }

  detail::recv_t rcv;
  message msg;
  aid_t sender;
  bool need_resume = false;

  if (
    (recving_ && !is_response) ||
    (responsing_ && is_response)
    )
  {
    if (recving_ && !is_response)
    {
      bool ret = mb_.pop(rcv, msg, curr_pattern_.match_list_);
      if (!ret)
      {
        return;
      }
      sender = end_recv(rcv, msg);
      curr_pattern_.clear();
      need_resume = true;
      recving_ = false;
    }

    if (responsing_ && is_response)
    {
      BOOST_ASSERT(recving_res_.valid());
      bool ret = mb_.pop(recving_res_, msg);
      if (!ret)
      {
        return;
      }
      sender = end_recv(recving_res_);
      need_resume = true;
      responsing_ = false;
      recving_res_ = resp_t();
    }

    ++tmr_sid_;
    errcode_t ec;
    tmr_.cancel(ec);

    if (msg.get_type() == detail::msg_new_actor)
    {
      need_resume = false;
      boost::uint16_t ty = u16_nil;
      msg >> ty;
      link_type type = (link_type)ty;
      handle_spawn(sender, type);
    }
    else if (msg.get_type() == detail::msg_spawn_ret)
    {
      need_resume = false;
      if (spw_hdr_)
      {
        spawn_handler_t hdr(spw_hdr_);
        spw_hdr_.clear();
        hdr(sender, msg);
      }
    }

    if (need_resume)
    {
      try
      {
        resume();
        set_recv_global(sender, msg);
        f_();
        quit();
      }
      catch (std::exception& ex)
      {
        quit(exit_except, ex.what());
      }
    }
  }
}
///----------------------------------------------------------------------------
void lua_actor::handle_spawn(aid_t aid, link_type type)
{
  try
  {
    if (aid)
    {
      if (type == linked)
      {
        link(aid);
      }
      else if (type == monitored)
      {
        monitor(aid);
      }
    }
    resume();
    luabridge::setGlobal(L_, aid, "gce_spawn_aid");
    f_();
    quit();
  }
  catch (std::exception& ex)
  {
    quit(exit_except, ex.what());
  }
}
///----------------------------------------------------------------------------
void lua_actor::handle_remote_spawn(
  aid_t aid,
  message msg, link_type type,
  boost::chrono::system_clock::time_point begin_tp,
  sid_t sid, seconds_t tmo, duration_t curr_tmo
  )
{
  boost::uint16_t err = 0;
  sid_t ret_sid = sid_nil;
  if (msg.get_type() != match_nil)
  {
    msg >> err >> ret_sid;
    do
    {
      if (err != 0 || (aid && sid == ret_sid))
      {
        break;
      }

      if (tmo != infin)
      {
        duration_t pass_time = boost::chrono::system_clock::now() - begin_tp;
        curr_tmo -= pass_time;
      }

      begin_tp = boost::chrono::system_clock::now();
      spw_hdr_ = 
        boost::bind(
          &lua_actor::handle_remote_spawn, this, _1, _2,
          type, begin_tp, sid, tmo, curr_tmo
          );
      recv_match(pattern(detail::msg_spawn_ret, curr_tmo));
      return;
    }
    while (false);

    detail::spawn_error error = (detail::spawn_error)err;
    if (error != detail::spawn_ok)
    {
      aid = aid_t();
    }

    if (aid)
    {
      if (type == linked)
      {
        link(aid);
      }
      else if (type == monitored)
      {
        monitor(aid);
      }
    }
  }

  try
  {
    resume();
    luabridge::setGlobal(L_, aid, "gce_spawn_aid");
    f_();
    quit();
  }
  catch (std::exception& ex)
  {
    quit(exit_except, ex.what());
  }
}
///----------------------------------------------------------------------------
void lua_actor::set_recv_global(aid_t const& sender, message const& msg)
{
  luabridge::setGlobal(L_, sender, "gce_recv_sender");
  luabridge::setGlobal(L_, msg, "gce_recv_msg");
}
///----------------------------------------------------------------------------
}

#endif /// GCE_LUA
